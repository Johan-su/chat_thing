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

static Usize str_nlen(const char *str, Usize max)
{
    Usize count = 0;
    while (str[count] != '\0' && count < max)
    {
        count += 1;
    }
    return count;
}

#pragma region Threading

struct Thread
{
    bool initalized = false;
    bool running = false;
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
    if (!GetExitCodeThread(thread->win_handle, (unsigned long *)&exit_code))
    {
        fprintf(stderr, "ERROR: GetExitCodeThread failed with %ld\n", GetLastError());
        exit(1);
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
    if (!thread->running)
    {
        ResumeThread(thread->win_handle);
    }
}

static void suspend_thread(Thread *thread)
{
    if (thread->running)
    {
        SuspendThread(thread->win_handle);
    }
}

static Thread get_current_thread()
{   
    U32 thread_id = GetCurrentThreadId();
    HANDLE win_handle = OpenThread(THREAD_ALL_ACCESS, false, thread_id);
    // DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), nullptr, &win_handle, 0, false, DUPLICATE_SAME_ACCESS);

    Thread thread = {
        .initalized = true,
        .running = true,
        .id = thread_id,
        .win_handle = win_handle,
    };
    return thread;
}


struct Mutex
{
    HANDLE win_handle;
};

static Mutex create_mutex()
{
    Mutex mutex = {};

    {
        HANDLE temp_handle = CreateMutexExA(nullptr, nullptr, 0, SYNCHRONIZE);
        if (temp_handle == nullptr)
        {
            TODO("handle error failed to create mutex");
        }
        mutex.win_handle = temp_handle;
    }
    return mutex;
}

static void clean_mutex(Mutex *mutex)
{
    CloseHandle(mutex->win_handle);
}

static void lock_mutex(Mutex *mutex)
{
    WaitForSingleObjectEx(mutex->win_handle, INFINITE, false);
}

static void release_mutex(Mutex *mutex)
{
    ReleaseMutex(mutex->win_handle);
}


struct Semaphore
{
    HANDLE win_handle;
};

static Semaphore create_semaphore(I32 inital_count, I32 max_count)
{
    Semaphore semaphore = {};
    {
        HANDLE temp_handle = CreateSemaphoreExA(nullptr, inital_count, max_count, nullptr, 0, SEMAPHORE_ALL_ACCESS);
        if (temp_handle == nullptr)
        {
            TODO("handle error failed to create semaphore");
        }
        semaphore.win_handle = temp_handle;
    }
    return semaphore;
};

static void clean_semaphore(Semaphore *semaphore)
{
    CloseHandle(semaphore->win_handle);
}

static void wait_semaphore(Semaphore *semaphore)
{
    WaitForSingleObjectEx(semaphore->win_handle, INFINITE, false);
}

static void release_semaphore(Semaphore *semaphore)
{
    ReleaseSemaphore(semaphore->win_handle, 1, nullptr);
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

enum CommandType
{
    INVALID_COMMAND,
    QUIT_SEVER,
    ABORT_QUITTING_SERVER,
};

enum Settings
{
    RECEIVE_BUF_LEN = 256,
    SEND_BUF_LEN = RECEIVE_BUF_LEN,
    DATA_HEADER_LEN = sizeof(DataFormat),
    MAX_DATA_LEN = RECEIVE_BUF_LEN - DATA_HEADER_LEN,
    MAX_NAME_LEN = 32,
    MAX_MESSAGE_LEN = MAX_DATA_LEN - MAX_NAME_LEN - 1, // -1 for separator
    COMMAND_HEADER_LEN = DATA_HEADER_LEN + sizeof(CommandType),
    MAX_COMMAND_LEN = RECEIVE_BUF_LEN - COMMAND_HEADER_LEN,
};


struct PublicMessage
{
    DataFormat format = PUBLIC_MESSAGE;
    char message[MAX_MESSAGE_LEN];
};



struct Command
{
    DataFormat format = COMMAND;
    CommandType command_type = INVALID_COMMAND;
    char data[MAX_COMMAND_LEN];
};

struct ServerBroadcast
{
    DataFormat format = SERVER_BROADCAST;
    char name[MAX_NAME_LEN];
    char message[MAX_MESSAGE_LEN];
};


static void assert_struct_sizes()
{
    assert(sizeof(PublicMessage) <= RECEIVE_BUF_LEN);
    assert(sizeof(Command) <= RECEIVE_BUF_LEN);
    assert(sizeof(ServerBroadcast) <= RECEIVE_BUF_LEN);
}


#pragma region Client

enum ClientSettings
{
    MAX_ACTIVE_CLIENT_THREADS = 5,
};


static volatile bool g_client_send_lock = true;
static Semaphore g_client_sending_semaphore = {};
static Mutex g_client_sending_mutex = {};
static char g_client_message_buffer[SEND_BUF_LEN] = {};

static volatile bool g_client_active = false;

static unsigned long client_sender(void *p)
{
    SOCKET socket = *(SOCKET *)p;
    while (true)
    {
        // wait on sender thread to copy data to send buffer
        wait_semaphore(&g_client_sending_semaphore);
        if (!g_client_active)
        {
            return 0;
        }

        I32 flags = 0;
        I32 bytes_sent = send(socket, g_client_message_buffer, SEND_BUF_LEN, flags);
        if (bytes_sent == SOCKET_ERROR)
        {
            TODO("handle failed to send bytes");
        }
        printf("sent %d bytes\n", bytes_sent);


        memset(g_client_message_buffer, 0, sizeof(g_client_message_buffer));
        g_client_send_lock = true;
    }
    return 0;
}


static void send_data(char *byte_buffer, Usize buf_size)
{
    if (buf_size > sizeof(g_client_message_buffer))
    {
        TODO("handle data is too big");
    }

    lock_mutex(&g_client_sending_mutex);

    memcpy(g_client_message_buffer, byte_buffer, buf_size);
    g_client_send_lock = false;
    release_semaphore(&g_client_sending_semaphore);

    while (g_client_send_lock == false)
    {}

    release_mutex(&g_client_sending_mutex);
}

static void send_message(const char *message)
{
    Usize message_len = str_nlen(message, MAX_MESSAGE_LEN);

    PublicMessage sb = {
        .format = PUBLIC_MESSAGE,
        .message = {},
    };

    memcpy(&sb.message, message, message_len);
    send_data((char *)&sb, sizeof(sb));
}



static void receive_data_client(char *raw_data)
{
    DataFormat *format = (DataFormat *)raw_data;
    switch (*format)
    {
        case DataFormat::INVALID:
        {
            TODO("handle client receiving");
        } return;

        case DataFormat::PUBLIC_MESSAGE:
        {
            TODO("handle client receiving");
        } return;

        case DataFormat::PRIVATE_MESSAGE:
        {
            TODO("handle client receiving");
        } return;

        case DataFormat::NAME_CHANGE:
        {
            TODO("handle client receiving");
        } return;

        case DataFormat::COMMAND:
        {
            TODO("handle client receiving");

        } return;
        case DataFormat::SERVER_BROADCAST:
        {
            ServerBroadcast *sb = (ServerBroadcast *)raw_data;
            printf("%.*s : %.*s\n", MAX_NAME_LEN, sb->name, MAX_MESSAGE_LEN, sb->message);
        } return;

    }
}

struct ReceiverParams
{
    SOCKET socket;
    char *receive_buffer;
    I32 receive_len;
};

static unsigned long receiver(void *param)
{
    ReceiverParams *p = (ReceiverParams *)param; 
    while (g_client_active)
    {
        I32 flags = 0;
        I32 bytes_received = recv(p->socket, p->receive_buffer, p->receive_len, flags);
        if (bytes_received == SOCKET_ERROR)
        {
            I32 error_code = WSAGetLastError();
            switch (error_code)
            {
                case WSAESHUTDOWN:
                {
                    if (g_client_active)
                    {
                        TODO("handle error client active, but WSA is shutdown");
                    }
                } break;

                case WSAECONNRESET:
                {
                    printf("Lost connection to server, server severed connection\n");
                    g_client_active = false;
                    return 0;
                } break;
                
                default:
                {
                    printf("%d\n", error_code);
                    TODO("handle error receiving message from server");
                } break;
            }
        }
        printf("received %d bytes\n", bytes_received);
        if (bytes_received == 0)
        {
            return 0;
        }

        receive_data_client(p->receive_buffer); 
    }
    return 0;
}


static void client(const char *name)
{
    g_client_active = true;
    g_client_sending_semaphore = create_semaphore(0, 1);
    g_client_sending_mutex = create_mutex();

    SOCKET connect_socket;
    // initalize socket
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
            printf("%d\n", WSAGetLastError());
            TODO("handle failed connection");
        }
    }
    
    {
        char receive_buf[RECEIVE_BUF_LEN];

        Thread send_thread = spawn_thread(client_sender, false, &connect_socket);

        ReceiverParams rparams = {
            .socket = connect_socket,
            .receive_buffer = receive_buf,
            .receive_len = ARRAY_COUNT(receive_buf),
        };

        Thread receive_thread = spawn_thread(receiver, false, &rparams);

        {
            char command_buffer[MAX_COMMAND_LEN + 1]; // + 1 for null termination
            
            while (g_client_active)
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
                        g_client_active = false;
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
                    send_message(command_buffer);
                }
            }
        }

        // gracefully exit client
        {
            shutdown(connect_socket, 2); // SD_BOTH
            closesocket(connect_socket);

            release_semaphore(&g_client_sending_semaphore);

            join_thread(&send_thread);
            join_thread(&receive_thread);

            clean_mutex(&g_client_sending_mutex);
            clean_semaphore(&g_client_sending_semaphore);

        }
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
    Thread thread;
    SOCKET socket;
    char name[MAX_NAME_LEN];
    char receive_buf[SERVER_RECEIVE_BUF_LEN];
};


static volatile Usize g_connection_count = 0;
static Client g_client_pool[MAX_ACTIVE_THREADS] = {};

static volatile bool g_server_data_send_lock = true;
static char g_server_data_buffer[SERVER_SEND_BUF_LEN] = {};

static Mutex g_server_sending_mutex = {};
static Semaphore g_server_sending_semaphore = {};

static volatile bool g_server_active = false;


static unsigned long server_sender(void *)
{
    while (true)
    {
        // wait on broadcasting thread to copy data to send buffer
        wait_semaphore(&g_server_sending_semaphore);
        if (!g_server_active)
        {
            break;
        }

        for (Usize i = 0; i < ARRAY_COUNT(g_client_pool); ++i)
        {
            if (g_client_pool[i].active)
            {
                I32 flags = 0;
                I32 bytes_sent = send(g_client_pool[i].socket, g_server_data_buffer, SEND_BUF_LEN, flags);
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
    return 0;
}


static void broadcast_message_to_all_clients(const char *name, const char *message)
{
    lock_mutex(&g_server_sending_mutex);

    Usize name_len = str_nlen(name, MAX_NAME_LEN);
    Usize message_len = str_nlen(message, MAX_MESSAGE_LEN);


    ServerBroadcast sb = {
        .format = SERVER_BROADCAST,
        .name = {},
        .message = {},
    };

    memcpy(sb.name, name, sizeof(char) * name_len);
    memcpy(sb.message, message, sizeof(char) * message_len);
    memcpy(g_server_data_buffer, &sb, sizeof(sb));
    printf("%.*s : %.*s\n", MAX_NAME_LEN, sb.name, MAX_MESSAGE_LEN, sb.message);
    g_server_data_send_lock = false;
    release_semaphore(&g_server_sending_semaphore);

    while (g_server_data_send_lock == false)
    {}

    release_mutex(&g_server_sending_mutex);
}

static void receive_data_server(Client *client)
{
    char *raw_data = client->receive_buf;
    DataFormat *format = (DataFormat *)raw_data;
    switch (*format)
    {
        case DataFormat::INVALID:
        {
            TODO("handle invalid data");
        } return;

        case DataFormat::PUBLIC_MESSAGE:
        {
            PublicMessage *message = (PublicMessage *)raw_data;
            broadcast_message_to_all_clients(client->name, message->message);

        } return;

        case DataFormat::PRIVATE_MESSAGE:
        {
            TODO("handle private message");
        } return;

        case DataFormat::NAME_CHANGE:
        {
            TODO("handle name change");
        } return;

        case DataFormat::COMMAND:
        {
            TODO("handle command");

        } return;
        case DataFormat::SERVER_BROADCAST:
        {
            TODO("handle server receiving server broadcast somehow");
        } return;

    }
}

static unsigned long client_to_server_receiver(void *param)
{
    Client *client = (Client *)param; 
    while (g_server_active)
    {
        I32 flags = 0;
        I32 bytes_received = recv(client->socket, client->receive_buf, RECEIVE_BUF_LEN, flags);
        if (bytes_received == SOCKET_ERROR)
        {
            TODO("handle error receiving message from client");
        }
        printf("received %d bytes from %.*s\n", bytes_received, MAX_NAME_LEN, client->name);
        
        if (bytes_received == 0)
        {
            client->active = false;
            closesocket(client->socket);
            {
                char message_buffer[MAX_NAME_LEN + 14 + 1];
                snprintf(message_buffer, sizeof(message_buffer), "%s disconnected\n", client->name);
                broadcast_message_to_all_clients("Server", message_buffer);   
            }
            memset(client, 0, sizeof(Client));
            return 0;
        }

        receive_data_server(client);
    }
    return 0;
}


struct AcceptConnectionsParams
{
    SOCKET server_socket;
};

static unsigned long accept_connections(void *p)
{
    SOCKET socket = *(SOCKET *)p;
    while (g_server_active)
    {
        struct sockaddr_in socket_addr = {};
        socket_addr.sin_family = AF_INET;
        I32 addr_len = sizeof(socket_addr);
        if (g_connection_count < MAX_ACTIVE_THREADS)
        {
            SOCKET tmp_socket = accept(socket, (struct sockaddr *)&socket_addr, &addr_len);
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
                        break;
                    }
                }
                    g_connection_count += 1;
                    g_client_pool[i].receive_lock = true; 
                    g_client_pool[i].socket = tmp_socket;
                    g_client_pool[i].active = true;
                    
                    snprintf(g_client_pool[i].name, MAX_NAME_LEN, "%d.%d.%d.%d", 
                        socket_addr.sin_addr.S_un.S_un_b.s_b1, 
                        socket_addr.sin_addr.S_un.S_un_b.s_b2, 
                        socket_addr.sin_addr.S_un.S_un_b.s_b3, 
                        socket_addr.sin_addr.S_un.S_un_b.s_b4);


                    g_client_pool[i].thread = spawn_thread(client_to_server_receiver, false, &g_client_pool[i]);

                assert(i != ARRAY_COUNT(g_client_pool) - 1); // max client amount reached
            }

            char ip_buf[30] = {};

            snprintf(ip_buf, sizeof(ip_buf), "%d.%d.%d.%d has connected", 
                socket_addr.sin_addr.S_un.S_un_b.s_b1, 
                socket_addr.sin_addr.S_un.S_un_b.s_b2, 
                socket_addr.sin_addr.S_un.S_un_b.s_b3, 
                socket_addr.sin_addr.S_un.S_un_b.s_b4);
                

            broadcast_message_to_all_clients("Server", ip_buf);
        }
    }
    return 0;
}



static void server()
{
    g_server_active = true;
    g_server_sending_semaphore = create_semaphore(0, 1);
    g_server_sending_mutex = create_mutex();

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

    Thread sender_thread = spawn_thread(server_sender, false, nullptr);

    // main thread accepts connections
    accept_connections((void *)&server_socket);

    // gracefully exit server
    {
        shutdown(server_socket, 2); // SD_Both
        closesocket(server_socket);

        release_semaphore(&g_server_sending_semaphore);

        join_thread(&sender_thread);

        clean_mutex(&g_server_sending_mutex);
        clean_semaphore(&g_server_sending_semaphore);

    }
}

#pragma endregion

static char g_name_buffer[MAX_NAME_LEN + 1] = {}; // + 1 for null termination

int main(int argc, char *argv[])
{
    assert_struct_sizes();
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