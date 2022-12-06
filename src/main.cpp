#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef size_t Usize;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

#define DEBUG_ASSERTS

#ifdef DEBUG_ASSERTS
#define assert(expression)                                                                                  \
    do                                                                                                      \
    {                                                                                                       \
        if (!(expression))                                                                                  \
        {                                                                                                   \
            fprintf(stderr, "ERROR: assertion failed %s, at %s:%d in \n", #expression, __FILE__, __LINE__); \
            exit(1);                                                                                        \
        }                                                                                                   \
    } while (0)
#else
#define assert(expression)
#endif

#define TODO(string)                                                                   \
    do                                                                                 \
    {                                                                                  \
        fprintf(stderr, "ERROR: TODO %s, at %s:%d in \n", string, __FILE__, __LINE__); \
        exit(1);                                                                       \
    } while (0)


#define ARRAY_COUNT(array) sizeof(array) / sizeof(array[0])


template <typename T>
static T *alloc(Usize amount)
{
    return (T *)malloc(sizeof(T) * amount);
}


enum class FLAGS
{
    HOST,
    PORT,
    IP,
};

struct FlagData
{
    FLAGS flag;

    union
    {
        U64 data;
        U8 bytes[8];
    };
    
};

struct Config
{
    bool host;
    U16 port;
    U8 ip_bytes[4];
};

static Config g_config = {
    .host = false,
    .port = 25565,
    .ip_bytes = {127, 0, 0, 1},
}; 

static bool is_str(const char *str1, const char *str2)
{
    if (str1 == nullptr || str2 == nullptr)
    {
        return false;
    }

    while (*str1 != '\0' && *str2 != '\0')
    {
        if (*str1 != *str2)
        {
            return false;
        }
        str1 += 1;
        str2 += 1;
    }
    
    return true;
}

static bool is_number(char chr)
{
    switch (chr)
    {
        case '0': return true;
        case '1': return true;
        case '2': return true;
        case '3': return true;
        case '4': return true;
        case '5': return true;
        case '6': return true;
        case '7': return true;
        case '8': return true;
        case '9': return true;
        
        default: return false;
    }
}

static Usize str_len(const char *str)
{
    Usize count = 0;
    while (str[count] != '\0')
    {
        count += 1;
    }
    return count;
}

#pragma region Threading

struct Thread
{
    bool initalized = false;
    bool suspended = false;
    U32 id;
    HANDLE win_handle;
};

static Thread spawn_thread(unsigned long (thread_func)(void *), bool suspended, void *parameters)
{
    Thread thread = {};
    {

        U32 creation_flags = 0;
        if (suspended)
        {
            creation_flags = CREATE_SUSPENDED;
        }

        U32 id;
        HANDLE handle = CreateThread(nullptr, 0, thread_func, parameters, creation_flags, (unsigned long *)&id);
        if (handle == nullptr)
        {
            TODO("handle error failed to create thread");
        }
        thread.initalized = true;
        thread.id = id;
        thread.win_handle = handle;
    }
    return thread;
}

static bool is_thread_alive(Thread *thread)
{
    if (!thread->initalized)
        return false;

    U32 exit_code;
    if (GetExitCodeThread(thread->win_handle, (unsigned long *)&exit_code))
    {
        TODO("handle failed to check if thread is alive");
    }

    if (exit_code == STILL_ACTIVE)
        return true;

    return false;
}

static void join_thread(Thread *thread)
{
    while (is_thread_alive(thread))
    {}
}

static void kill_thread(Thread *thread)
{
    if (is_thread_alive(thread))
    {
        TerminateThread(thread->win_handle, 0);
        thread->initalized = false;
    }
}

static void resume_thread(Thread *thread)
{
    if (thread->suspended)
    {
        ResumeThread(thread->win_handle);
        thread->suspended = false;
    }
}

static void suspend_thread(Thread *thread)
{
    if (!thread->suspended)
    {
        thread->suspended = true;
        SuspendThread(thread->win_handle);
    }
}

static Thread get_current_thread()
{   
    HANDLE win_handle = GetCurrentThread();

    Thread thread = {
        .initalized = true,
        .suspended = false,
        .id = GetThreadId(win_handle),
        .win_handle = win_handle,
    };
    return thread;
}

#pragma endregion

#pragma region Winsocket

static void init_WSA()
{
    WORD wsa_version = MAKEWORD(2, 2);
    WSADATA data;
    if (WSAStartup(wsa_version, &data) != 0)
    {
        TODO("handle WSA startup error");
    }
}
#pragma endregion

enum DataFormat
{
    INVALID,
    PUBLIC_MESSAGE,
    PRIVATE_MESSAGE,
    NAME_CHANGE,
    COMMAND,
    SERVER_BROADCAST,
};


enum Settings
{
    RECEIVE_BUF_LEN = 256,
    SEND_BUF_LEN = RECEIVE_BUF_LEN,
    DATA_HEADER_LEN = sizeof(DataFormat),
    MAX_DATA_LEN = RECEIVE_BUF_LEN - DATA_HEADER_LEN,
    MAX_NAME_LEN = 32,
    MAX_MESSAGE_LEN = MAX_DATA_LEN - MAX_NAME_LEN - 1, // -1 for separator
};

//TODO(Johan): was last here

struct PublicMessage
{
    DataFormat format = PUBLIC_MESSAGE;
    char message[MAX_MESSAGE_LEN];
};


struct ServerBroadcast
{
    DataFormat format = SERVER_BROADCAST;
    char name[MAX_NAME_LEN]
    char data[MAX_MESSAGE_LEN];
}


#pragma region Client

enum ClientSettings
{
    MAX_COMMAND_LEN = MAX_DATA_LEN,
    MAX_ACTIVE_CLIENT_THREADS = 5,
};

struct MessageThreadData
{
    volatile U64 queue_count = 0;
    volatile bool send_lock = true;
    Thread *thread_queue[MAX_ACTIVE_CLIENT_THREADS] = {};
    char message_buffer[SEND_BUF_LEN] = {};

};

static MessageThreadData g_message_thread_data = {};

static unsigned long client_sender(void *)
{
    while (true)
    {
        // if not nullptr something is at the start of the stack
        if (g_message_thread_data.queue_count != 0 &&
            g_message_thread_data.thread_queue[g_message_thread_data.queue_count - 1] != nullptr)
        {
            //TODO(Johan): maybe change message queue from stack to FIFO
            resume_thread(g_message_thread_data.thread_queue[g_message_thread_data.queue_count - 1]);
            g_message_thread_data.thread_queue[g_message_thread_data.queue_count] = nullptr;
            InterlockedDecrement64((volatile LONG64 *)&g_message_thread_data.queue_count);

            // wait on sender thread to copy data to send buffer
            while (g_message_thread_data.send_lock)
            {}

            memset(g_message_thread_data.message_buffer, 0, sizeof(g_message_thread_data.message_buffer));
            g_message_thread_data.send_lock = true;
        }
    }
    return 0;
}

static void push_on_client_thread_queue_and_suspend(Thread *thread, U64 queue_pos)
{
    if (queue_pos >= ARRAY_COUNT(g_message_thread_data.thread_queue))
    {
        TODO("Handle error max threads in server message queue");
    }

    g_message_thread_data.thread_queue[queue_pos] = thread;
    suspend_thread(thread);
}

static void send_data(Thread *self, const char *byte_buffer, Usize buf_size)
{
    if (buf_size > sizeof(g_message_thread_data.message_buffer))
    {
        TODO("handle data is too big");
    }

    {
        U64 queue_pos = (U64)InterlockedIncrement64((volatile LONG64 *)&g_message_thread_data.send_lock) - 1;
        push_on_client_thread_queue_and_suspend(self, queue_pos);
    }


    memcpy(g_message_thread_data.message_buffer, byte_buffer, buf_size);
    g_message_thread_data.send_lock = false;
}

static void send_message(Thread *self, const char *name, const char *message)
{
    Usize name_len = str_len(name);
    Usize message_len = str_len(message);
    if (name_len + 1 + message_len > sizeof(g_message_thread_data.message_buffer))
    {
        TODO("handle message is too long");
    }


    char buffer[SEND_BUF_LEN] = {};

    memcpy(buffer, name, sizeof(*name) * name_len);
    buffer[name_len] = ':';
    memcpy(buffer + name_len + 1, message, sizeof(*name) * message_len);

    send_data(self, buffer, sizeof(char) * (name_len + 1 + message_len));

}


struct ReceiverParams
{
    Thread *self;
    SOCKET socket;
    char *receive_buffer;
    I32 receive_len;
};

static unsigned long receiver(void *param)
{
    ReceiverParams *p = (ReceiverParams *)param; 
    while (true)
    {
        I32 flags = 0;
        I32 bytes_received = recv(p->socket, p->receive_buffer, p->receive_len, flags);
        if (bytes_received == SOCKET_ERROR)
        {
            TODO("handle error receiving message from server");
        }
        printf("received %d bytes\n", bytes_received);

        

        printf("%s\n", p->receive_buffer);


    }
    return 0;
}

static void client(const char *name_buffer)
{
    SOCKET connect_socket;
    {
        connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connect_socket == INVALID_SOCKET)
        {
            TODO("handle invalid connect_socket");
        }

        sockaddr_in SOCK_addr;

        SOCK_addr.sin_family = AF_INET;
        SOCK_addr.sin_port = g_config.port;
        SOCK_addr.sin_addr.S_un.S_un_b.s_b1 = g_config.ip_bytes[0];
        SOCK_addr.sin_addr.S_un.S_un_b.s_b2 = g_config.ip_bytes[1];
        SOCK_addr.sin_addr.S_un.S_un_b.s_b3 = g_config.ip_bytes[2];
        SOCK_addr.sin_addr.S_un.S_un_b.s_b4 = g_config.ip_bytes[3];

        if (connect(connect_socket, (const struct sockaddr *)&SOCK_addr, sizeof(SOCK_addr)) == SOCKET_ERROR)
        {
            TODO("handle failed connection");
        }
    }
    
    {
        char receive_buf[RECEIVE_BUF_LEN];

        Thread send_thread = {};
        {
            send_thread = spawn_thread(client_sender, false, nullptr);
        }

        Thread receive_thread = {};
        {
            ReceiverParams rparams = {
                .self = &receive_thread,
                .socket = connect_socket,
                .receive_buffer = receive_buf,
                .receive_len = ARRAY_COUNT(receive_buf),
            };

            receive_thread = spawn_thread(receiver, false, &rparams);
        }


        Thread self = get_current_thread();
        {
            char command_buffer[MAX_COMMAND_LEN + 1]; // + 1 for null termination
            
            while (true)
            {
                memset(command_buffer, 0, sizeof(command_buffer));
                {
                    char *result = fgets(command_buffer, ARRAY_COUNT(command_buffer), stdin);
                    if (result == nullptr && ferror(stdin) != 0)
                    {
                        TODO("handle error with command_buffer input");
                    }
                }
                if (command_buffer[0] == '/')
                {
                    if (is_str(&command_buffer[1], "q"))
                    {
                        break;
                    }
                    else if (false)
                    {

                    }
                    else
                    {
                        TODO("handle incorrect command");
                    }
                }
                else
                {
                    send_message(&self, name_buffer, command_buffer);
                }
            }
        }

        kill_thread(&send_thread);
        kill_thread(&receive_thread);

        join_thread(&send_thread);
        join_thread(&receive_thread);
    }
}

#pragma endregion

#pragma region Server

enum ServerSettings
{
    MAX_ACTIVE_THREADS = 128,
    SERVER_RECEIVE_BUF_LEN = RECEIVE_BUF_LEN,
    SERVER_SEND_BUF_LEN = SERVER_RECEIVE_BUF_LEN,
};

struct Client
{
    bool active = false;
    volatile bool receive_lock = true;
    char receive_buf[SERVER_RECEIVE_BUF_LEN];
    Thread thread;
    SOCKET socket;
};

struct ClientToServerReceiverParams
{
    Thread *self;
    SOCKET client_socket;
    char *receive_buffer;
    I32 receive_len;
};

static volatile Usize g_connection_count = 0;
static Client g_client_pool[MAX_ACTIVE_THREADS] = {};

static volatile U64 g_server_data_queue_count = 0;
static volatile bool g_server_data_send_lock = true;
static char g_server_data_buffer[SERVER_SEND_BUF_LEN] = {};

// + 10 to account for main thread and other non-client threads
static Thread *g_server_message_thread_queue[MAX_ACTIVE_THREADS + 10] = {};

static unsigned long server_sender(void *)
{
    while (true)
    {
        // if not nullptr something is at the start of the stack
        if (g_server_data_queue_count != 0 && 
            g_server_message_thread_queue[g_server_data_queue_count - 1] != nullptr)
        {
            //TODO(Johan): maybe change message queue from stack to FIFO
            resume_thread(g_server_message_thread_queue[g_server_data_queue_count - 1]);
            g_server_message_thread_queue[g_server_data_queue_count] = nullptr;
            InterlockedDecrement64((volatile LONG64 *)&g_server_data_queue_count);

            // wait on broadcasting thread to copy data to send buffer
            while (g_server_data_send_lock)
            {}

            for (Usize i = 0; i < ARRAY_COUNT(g_client_pool); ++i)
            {
                if (g_client_pool[i].active)
                {
                    I32 send_len = (I32)str_len(g_server_data_buffer) + 1;
                    I32 flags = 0;
                    I32 bytes_sent = send(g_client_pool[i].socket, g_server_data_buffer, send_len, flags);
                    if (bytes_sent == SOCKET_ERROR)
                    {
                        TODO("handle failed to send bytes");
                    }
                    printf("sent %d bytes\n", bytes_sent);
                }
            }
            memset(g_server_data_buffer, 0, sizeof(g_server_data_buffer));
            g_server_data_send_lock = true;
        }
    }
    return 0;
}


static void push_on_server_thread_queue_and_suspend(Thread *thread, U64 queue_pos)
{
    if (queue_pos >= ARRAY_COUNT(g_server_message_thread_queue))
    {
        TODO("Handle error max threads in server message queue");
    }

    g_server_message_thread_queue[queue_pos] = thread;
    suspend_thread(thread);
}


static void broadcast_message_to_all_clients(const char *name, const char *message, Thread *self)
{
    Usize name_len = str_len(name);
    Usize message_len = str_len(message);
    if (name_len + message_len >= ARRAY_COUNT(g_server_data_buffer))
    {
        TODO("handle message is too long");
    }

    {
        U64 queue_pos = (U64)InterlockedIncrement64((volatile LONG64 *)&g_server_data_queue_count) - 1;
        push_on_server_thread_queue_and_suspend(self, queue_pos);
    }

    memcpy(g_server_data_buffer, name, sizeof(char) * name_len); // - 1 to remove null termination char
    g_server_data_buffer[name_len] = ':';
    memcpy(g_server_data_buffer + name_len + 1, message, sizeof(char) * message_len);
    g_server_data_buffer[name_len + message_len + 1] = '\0';
    printf("%s: %s\n", name, message);
    g_server_data_send_lock = false;
}

static void receive_data_server(char *raw_data, Usize data_len)
{
    DataFormat *format = (DataFormat *)raw_data;
    switch (*format)
    {
        case DataFormat::INVALID:
        {
            TODO("handle invalid data");
        } break;

        case DataFormat::PUBLIC_MESSAGE:
        {
            PublicMessage *message = (PublicMessage *)raw_data;
            broadcast_message_to_all_clients()

        } break;

        case DataFormat::PRIVATE_MESSAGE:
        {

        } break;

        case DataFormat::NAME_CHANGE:
        {

        } break;

        case DataFormat::COMMAND:
        {

        } break;

    }
}

static unsigned long client_to_server_receiver(void *param)
{
   ClientToServerReceiverParams *p = (ClientToServerReceiverParams *)param; 
    while (true)
    {
        I32 flags = 0;
        I32 bytes_received = recv(p->client_socket, p->receive_buffer, p->receive_len, flags);
        if (bytes_received == SOCKET_ERROR)
        {
            TODO("handle error receiving message from client");
        }

        TODO("fix");

        //TODO(Johan): fix 
        // const char *name = p->receive_buffer;
        // const char *message = &p->receive_buffer[NAME_LEN]; 
        // broadcast_message_to_all_clients(name, message, p->self);
    }
    return 0;
}



struct AcceptConnectionsParams
{
    Thread *self;
    SOCKET server_socket;
};

static unsigned long accept_connections(void *param)
{
    AcceptConnectionsParams *p = (AcceptConnectionsParams *)param;
    while (true)
    {
        struct sockaddr_in socket_addr = {};
        socket_addr.sin_family = AF_INET;
        I32 addr_len = sizeof(socket_addr);
        if (g_connection_count < MAX_ACTIVE_THREADS)
        {
            SOCKET tmp_socket = accept(p->server_socket, (struct sockaddr *)&socket_addr, &addr_len);
            if (tmp_socket == INVALID_SOCKET)
            {
                fprintf(stderr, "failed to accept connection with, %d \n", WSAGetLastError());
                exit(1);
                TODO("handle error failed to accept connection");
            }


            // spawn receiver thread
            {
                U32 i = 0;
                for (i = 0; i < ARRAY_COUNT(g_client_pool); ++i)
                {
                    if (!g_client_pool[i].active)
                    {
                        g_connection_count += 1;
                        g_client_pool[i].receive_lock = true; 
                        g_client_pool[i].socket = tmp_socket;
                        g_client_pool[i].active = true;

                        {
                            ClientToServerReceiverParams ctsrp = {
                                .self = &g_client_pool[i].thread,
                                .client_socket = g_client_pool->socket,
                                .receive_buffer = g_client_pool->receive_buf,
                                .receive_len = ARRAY_COUNT(g_client_pool->receive_buf), 
                            };

                            g_client_pool[i].thread = spawn_thread(client_to_server_receiver, false, (void *)&ctsrp);
                        }
                        break;
                    }
                }
                assert(i != ARRAY_COUNT(g_client_pool) - 1); // max client amount reached
            }
            broadcast_message_to_all_clients("Server", "Someone connected", p->self);
        }
    }
    return 0;
}


static void server()
{
    SOCKET server_socket;
    {
        server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_socket == INVALID_SOCKET)
        {
            TODO("handle failed to create server socket");
        }

        sockaddr_in SOCK_addr;

        SOCK_addr.sin_family = AF_INET;
        SOCK_addr.sin_port = g_config.port;
        SOCK_addr.sin_addr.S_un.S_un_b.s_b1 = g_config.ip_bytes[0];
        SOCK_addr.sin_addr.S_un.S_un_b.s_b2 = g_config.ip_bytes[1];
        SOCK_addr.sin_addr.S_un.S_un_b.s_b3 = g_config.ip_bytes[2];
        SOCK_addr.sin_addr.S_un.S_un_b.s_b4 = g_config.ip_bytes[3];

        if (bind(server_socket, (struct sockaddr *)&SOCK_addr, sizeof(SOCK_addr)) == SOCKET_ERROR)
        {
            TODO("handle failed to bind server socket to ip address");
        }

        if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
        {
            TODO("handle failed to set socket to listener");
        }
    }

    {
        Thread sender_thread = {};
        {
            sender_thread = spawn_thread(server_sender, false, nullptr);
        }


        Thread connection_thread = {};
        {
            AcceptConnectionsParams acp = {
                .self = &connection_thread,
                .server_socket = server_socket,
            };

            connection_thread =  spawn_thread(accept_connections, false, (void *)&acp);
        }
    }

    while (true)
    {}
}

#pragma endregion

static char g_name_buffer[MAX_NAME_LEN + 1] = {}; // + 1 for null termination

int main (int argc, char *argv[])
{
    const char *program = argv[0]; (void)program;

    // initalize config
    for (I32 i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (is_str(&argv[i][1], "h") && argv[i][2] == '\0')
            {
                g_config.host = true;
                continue;
            }
            else if (is_str(&argv[i][1], "ip"))
            {
                if (str_len(argv[i + 1]) < 7)
                {
                    TODO("handle ip must be longer");
                }
                char str_int_buffer[4];


                I32 j = 0;
                for (I32 bytes = 0; bytes < 4; ++bytes)
                {
                    if (!is_number(argv[i + 1][j]))
                    {
                        TODO("handle error has to be number after .");
                    }

                    I32 k = 0;
                    while (is_number(argv[i + 1][j]))
                    {
                        if (k > 2)
                        {
                            TODO("handle error more than 3 digits in ip byte");
                        }

                        str_int_buffer[k] = argv[i + 1][j];
                        k += 1;
                        j += 1;
                    }
                    if (!is_str(&argv[i + 1][j], "."))
                    {
                        TODO("handle error ip bytes have to be separated by .");
                    }
                    j += 1;

                    str_int_buffer[3] = '\0';
                    g_config.ip_bytes[bytes] = (U8)atoi(str_int_buffer);
                    memset(str_int_buffer, 0, sizeof(str_int_buffer));
                }
                i += 1;
                continue;
            }
            else if (is_str(&argv[i][1], "p"))
            {
                Usize length = str_len(argv[i + 1]);
                if (length == 0 || length > 5)
                {
                    TODO("Handle port length error");
                }
                U16 result;
                {
                    char str_int_buffer[6];
                    for (I32 j = 0; j < 5 && argv[i + 1][j] != '\0'; ++j)
                    {
                        if (!is_number(argv[i + 1][j]))
                        {
                            TODO("Handle \"Port must a number error\"");
                        }
                        str_int_buffer[j] = argv[i + 1][j]; 
                    }
                    str_int_buffer[5] = '\0';
                    result = (U16)atoi(str_int_buffer);
                }

                if (result < 0 || result > 65535)
                {
                    TODO("handle invalid port");
                }
                g_config.port = result;
                i += 1;
                continue;
            }
            else if (is_str(&argv[i][1], "n"))
            {
                Usize length = str_len(argv[i + 1]);
                if (length >= ARRAY_COUNT(g_name_buffer))
                {
                    TODO("handle error too long name, max characters reached");
                }

                memcpy(g_name_buffer, argv[i + 1], sizeof(char) * length);
                g_name_buffer[length] = '\0';


                i += 1;
                continue;
            }
        }
    }

    printf("Config [Host: %u, port: %u, Ip: %u.%u.%u.%u]\n", g_config.host, g_config.port, g_config.ip_bytes[0], g_config.ip_bytes[1], g_config.ip_bytes[2], g_config.ip_bytes[3]);
    
    init_WSA();

    if (g_config.host == true)
    {
        server();
    }
    else
    {
        client(g_name_buffer);
    }

    return 0;
} 