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
#define assert(expression)                                                                                 \
    do                                                                                                     \
    {                                                                                                      \
        if (!(expression))                                                                                 \
        {                                                                                                  \
            fprintf(stderr, "ERROR: assertion failed %s, at %s:%d in \n", __FILE__, __LINE__);             \
            exit(1);                                                                                       \
        }                                                                                                  \
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
    U32 id;
    HANDLE win_handle;
};

static Thread spawn_thread(unsigned long (thread_func)(void *), void *parameters)
{
    Thread thread = {};
    {
        U32 id;
        HANDLE handle = CreateThread(nullptr, 0, thread_func, parameters, 0, (unsigned long *)&id);
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


static void send_message(const char *name, const char *message, char *send_buffer, I32 send_buf_len, volatile bool *send_lock)
{
    Usize name_len = str_len(name);
    Usize message_len = str_len(message);
    if (name_len + message_len >= send_buf_len)
    {
        TODO("handle message is too long");
    }

    {
        while (send_lock) {}
        *send_lock = true;
        //TODO(Johan): check if it works
        memcpy(send_buffer, name, sizeof(char) * name_len);
        memcpy(send_buffer, message, sizeof(char) * message_len);
        send_buffer[name_len + message_len] = '\0';

        *send_lock = false;
    }
}

#pragma region Client

struct ReceiverParams
{
    SOCKET socket;
    char *receive_buffer;
    I32 receive_length;
    volatile bool *receive_lock;
};

static void receiver(SOCKET socket, char *receive_buffer, I32 receive_length, volatile bool *receive_lock)
{
    while (true)
    {
        if (!*receive_lock)
        {
            I32 flags = 0;
            I32 bytes_received = recv(socket, receive_buffer, receive_length, flags);
            if (bytes_received == SOCKET_ERROR)
            {
                TODO("Handle failed to receive bytes");
            }
            printf("received %d bytes\n", bytes_received);
            printf("%s\n", receive_buffer);
            *receive_lock = true;
        }
    }
}

struct SenderParams
{
    SOCKET socket;
    char *send_buffer;
    I32 send_length;
    volatile bool *send_lock;
};

static void sender(SOCKET socket, char *send_buffer, I32 send_length, volatile bool *send_lock)
{
    while (true)
    {
        if (!*send_lock)
        {
            I32 flags = 0;
            I32 bytes_sent = send(socket, send_buffer, send_length, flags);
            if (bytes_sent == SOCKET_ERROR)
            {
                TODO("handle failed to send bytes");
            }
            printf("sent %d bytes\n", bytes_sent);
            *send_lock = true;
        }
    }
}


static unsigned long receiver_threaded(void *param)
{
    ReceiverParams *params = (ReceiverParams *)param; 
    receiver(params->socket, params->receive_buffer, params->receive_length, params->receive_lock);
    return 0;
}


static unsigned long sender_threaded(void *param)
{
    SenderParams *params = (SenderParams *)param; 
    sender(params->socket, params->send_buffer, params->send_length, params->send_lock);
    return 0;
}





enum ClientSettings
{
    RECEIVE_BUF_LEN = 256,
    SEND_BUF_LEN = RECEIVE_BUF_LEN,
    NAME_LEN = 32,
    MESSAGE_LEN = RECEIVE_BUF_LEN - NAME_LEN,
    COMMAND_BUF_LEN = MESSAGE_LEN,
};

static void client(const char *name_buffer, I32 name_length)
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
        char send_buf[SEND_BUF_LEN];
        volatile bool receive_lock = true;
        volatile bool send_lock = true;

        Thread send_thread = {};
        Thread receive_thread = {};
        {
            SenderParams sparams = {connect_socket, send_buf, SEND_BUF_LEN, &send_lock};
            ReceiverParams rparams = {connect_socket, receive_buf, RECEIVE_BUF_LEN, &receive_lock};

            send_thread = spawn_thread(receiver_threaded, &sparams);
            receive_thread = spawn_thread(sender_threaded, &rparams);
        }

        {
            char command_buffer[COMMAND_BUF_LEN];
            
            while (true)
            {
                memset(command_buffer, 0, sizeof(command_buffer));
                {
                    char *result = fgets(command_buffer, COMMAND_BUF_LEN, stdin);
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
                    send_message(name_buffer, command_buffer, send_buf, SEND_BUF_LEN, &send_lock);
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

static void client_to_server_receiver(SOCKET client_socket, char *receive_buffer, I32 receive_len, volatile bool *receieve_lock)
{
    while (true)
    {
        if (!*receieve_lock)
        {
            I32 flags = 0;
            I32 bytes_received = recv(client_socket, receive_buffer, receive_len, flags);
            if (bytes_received == SOCKET_ERROR)
            {
                TODO("handle error receiving message from client");
            }

            *receieve_lock = true;
        }
    }
}

struct ToServerReceiverParams
{
    SOCKET client_socket;
    char *receive_buffer;
    I32 receive_len;
    volatile bool *receieve_lock;
};

static unsigned long client_to_server_receiver_threaded(void *param)
{
    ToServerReceiverParams *tsrp = (ToServerReceiverParams *)param;
    client_to_server_receiver(tsrp->client_socket, tsrp->receive_buffer, tsrp->receive_len, tsrp->receieve_lock);
    return 0;
}

static void accept_connections(SOCKET server_socket, Client *client_pool, Usize client_pool_length, Usize *connection_count, volatile bool *accept_lock, char *send_buffer, I32 send_buffer_len, volatile bool *send_lock)
{
    while (true)
    {
        struct sockaddr_in socket_addr = {};
        socket_addr.sin_family = AF_INET;
        I32 addr_len;
        while (*accept_lock) 
        {}
        if (*connection_count < MAX_ACTIVE_THREADS)
        {
            SOCKET tmp_socket = accept(server_socket, (struct sockaddr *)&socket_addr, &addr_len);
            if (tmp_socket == INVALID_SOCKET)
            {
                TODO("handle error failed to accept connection");
            }
            while (*accept_lock)
            {}
            send_message("Server", "Someone connected", send_buffer, send_buffer_len, send_lock);
            //TODO(Johan): add printing of connection and send into to every active thread
            {
                I32 i = 0;
                for (i = 0; i < client_pool_length; ++i)
                {
                    if (!client_pool[i].active)
                    {
                        *connection_count += 1;
                        client_pool[i].active = true;
                        client_pool[i].receive_lock = true; 
                        client_pool[i].socket = tmp_socket;

                        {
                            ToServerReceiverParams tsrp = 
                            {
                                .client_socket = client_pool[i].socket,
                                .receive_buffer = client_pool[i].receive_buf,
                                .receive_len = SERVER_RECEIVE_BUF_LEN,
                                .receieve_lock = &client_pool[i].receive_lock,
                            }; 

                            client_pool[i].thread = spawn_thread(client_to_server_receiver_threaded, (void *)&tsrp);
                        }
                        break;
                    }
                }
                assert(i != client_pool_length - 1);

            }
        }
    }
}

static void server_sender(Client *client_pool, Usize client_pool_len, char *send_buffer, I32 send_length, volatile bool *send_lock)
{
    while (true)
    {
        if (!*send_lock)
        {
            for (I32 i = 0; i < client_pool_len; ++i)
            {
                if (client_pool[i].active)
                {
                    I32 flags = 0;
                    I32 bytes_sent = send(client_pool[i].socket, send_buffer, send_length, flags);
                    if (bytes_sent == SOCKET_ERROR)
                    {
                        TODO("handle failed to send bytes");
                    }
                    printf("sent %d bytes\n", bytes_sent);
                }
            }
            *send_lock = true;
        }
    }
}



struct ConnectionsParams
{
    SOCKET server_socket;
    Client *client_pool;
    Usize client_pool_length;
    Usize *connection_count;
    volatile bool *accept_lock;
    char *send_buffer;
    I32 send_buffer_len;
    volatile bool *send_lock;
};

static unsigned long accept_connections_threaded(void *param)
{
    ConnectionsParams *cp = (ConnectionsParams *)param;
    accept_connections(cp->server_socket, cp->client_pool, cp->client_pool_length, cp->connection_count, cp->accept_lock, cp->send_buffer, cp->send_buffer_len, cp->send_lock);
    return 0;
}

struct ServerSenderParams
{
    Client *client_pool;
    Usize client_pool_len;
    char *send_buffer;
    I32 send_length;
    volatile bool *send_lock;
};

static unsigned long server_sender_threaded(void *param)
{
    ServerSenderParams *ssp = (ServerSenderParams *)param;
    server_sender(ssp->client_pool, ssp->client_pool_len, ssp->send_buffer, ssp->send_length, ssp->send_lock);
    return 0;
}


static Usize g_connection_count = 0;
static Client g_client_pool[MAX_ACTIVE_THREADS] = {};

static void server(const char *name_buffer, I32 name_len)
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
        char send_buffer[SERVER_SEND_BUF_LEN] = {};
        volatile bool send_lock = true;

        ServerSenderParams ssp = {};
        ssp.client_pool = g_client_pool;
        ssp.client_pool_len = MAX_ACTIVE_THREADS;
        ssp.send_buffer = send_buffer;
        ssp.send_length = SERVER_SEND_BUF_LEN;
        ssp.send_lock = &send_lock;

        Thread sender_thread = {};
        {
            sender_thread = spawn_thread(server_sender_threaded, (void *)&ssp);
        }

        volatile bool accept_lock = true;

        ConnectionsParams cp =
        {
            .server_socket = server_socket,
            .client_pool = g_client_pool,
            .client_pool_length = MAX_ACTIVE_THREADS,
            .connection_count = &g_connection_count,
            .accept_lock = &accept_lock,
            .send_buffer = send_buffer,
            .send_buffer_len = SERVER_SEND_BUF_LEN,
            .send_lock = &send_lock,
        };


        Thread connection_thread = {};
        {
            connection_thread =  spawn_thread(accept_connections_threaded, (void *)&cp);
        }
    }

    while (true)
    {}
}

#pragma endregion

static char g_name_buffer[NAME_LEN] = {};

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
                if (length >= ClientSettings::NAME_LEN)
                {
                    TODO("handle error too long name, max 31 characters");
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
        server(g_name_buffer, NAME_LEN);
    }
    else
    {
        client(g_name_buffer, NAME_LEN);
    }

    return 0;
} 