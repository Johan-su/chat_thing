#include "server.hpp"
#include "threads.hpp"
#include "buffer_lengths.hpp"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>

#define MAX_CONNECTIONS 16


volatile static bool is_active = false;


struct thread_pool_t
{
    size_t thread_count;
    bool init_threads[MAX_CONNECTIONS];
    void *threads[MAX_CONNECTIONS];
};


struct socket_pool_t
{
    bool init_sockets[MAX_CONNECTIONS];
    SOCKET sockets[MAX_CONNECTIONS];
};


volatile static thread_pool_t g_thread_pool = {};
volatile static socket_pool_t g_socket_pool = {};

static int find_empty_pos(volatile thread_pool_t *thread_pool) // not actually thread-safe
{
    int pos = -1;

    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (thread_pool->init_threads[i] == false)
        {
            pos = i;
            break;
        }
    }
    return pos;
}


static void init_thread_pool(volatile thread_pool_t *thread_pool)
{
    thread_pool->thread_count = 0;
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        thread_pool->init_threads[i] = false;
        thread_pool->threads[i] = NULL;
    }
}


static void init_socket_pool(volatile socket_pool_t *socket_pool)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        socket_pool->init_sockets[i] = false;
        socket_pool->sockets[i] = INVALID_SOCKET;
    }
}


static unsigned long receive_and_send_messages(void *client_socket_p)
{
    SOCKET client_socket = *(SOCKET *)client_socket_p;

    char buf[REC_BUF_LEN];

    while (is_active)
    {
        switch (recv(client_socket, buf, REC_BUF_LEN, 0))
        {
            case SOCKET_ERROR:
            {
                fprintf(stderr, "ERROR: recv failed with %d\n", WSAGetLastError());
                is_active = false;
            } break;


            case 0:
            {
                fprintf(stderr, "NOTE: gracefully shutting down...\n");
                is_active = false;
            } break;

            default:
            {
                printf("%s\n", buf);
                for (int i = 0; i < MAX_CONNECTIONS; ++i)
                {
                    if (g_socket_pool.init_sockets[i])
                    {
                        SOCKET socket = g_socket_pool.sockets[i];
                        if (send(socket, buf, REC_BUF_LEN, 0) == SOCKET_ERROR)
                        {
                            fprintf(stderr, "ERROR: send failed");
                        }
                    }
                }
            } break;
        }
    }
    if (shutdown(client_socket, 2) != 0) // SD_BOTH
    {
        fprintf(stderr, "ERROR: failed shutdown");
    }
    closesocket(client_socket);
    return 0;
}


static unsigned long accept_incoming_connections(void *server_socket_p)
{
    fprintf(stderr, "DEBUG: accept_incoming_connections\n");
    SOCKET server_socket = *(SOCKET *)server_socket_p;
    while (is_active)
    {
        if (g_thread_pool.thread_count < MAX_CONNECTIONS)
        {
            struct sockaddr_in socket_addr = {};

            socket_addr.sin_family = AF_INET;


            int socket_addres_len = sizeof(socket_addr);
            SOCKET tmp_socket = accept(server_socket, (struct sockaddr *)&socket_addr, &socket_addres_len);
            if (tmp_socket == INVALID_SOCKET)
            {
                fprintf(stderr, "ERROR: could not accept socket\n");
                is_active = false;
                return -1;
            }
            ipv4_addr socket_ip;
            socket_ip.bytes[0] = socket_addr.sin_addr.S_un.S_un_b.s_b1;
            socket_ip.bytes[1] = socket_addr.sin_addr.S_un.S_un_b.s_b2;
            socket_ip.bytes[2] = socket_addr.sin_addr.S_un.S_un_b.s_b3;
            socket_ip.bytes[3] = socket_addr.sin_addr.S_un.S_un_b.s_b4;

            int pos = find_empty_pos(&g_thread_pool);
            assert(pos != -1); // should never happen

            fprintf(stderr, "DEBUG: %u.%u.%u.%u connected\n", socket_ip.bytes[0], socket_ip.bytes[1], socket_ip.bytes[2], socket_ip.bytes[3]);

            g_thread_pool.init_threads[pos] = true;
            g_thread_pool.thread_count += 1;
            g_socket_pool.init_sockets[pos] = true;
            g_socket_pool.sockets[pos] = tmp_socket;
            
            g_thread_pool.threads[pos] = spawn_thread(receive_and_send_messages, &tmp_socket);
        }
        else
        {
            fprintf(stderr, "WARNING: ignoring connection, as max amount of connections [%d] has been reached\n", MAX_CONNECTIONS);
        }
    }
    return 0;
}


void server(ipv4_addr ip, uint16_t port)
{
    init_thread_pool(&g_thread_pool);
    init_socket_pool(&g_socket_pool);
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET)
    {
        fprintf(stderr, "ERROR: socket failed because, %d\n", WSAGetLastError());
        exit(1);
    }


    sockaddr_in socket_addr;

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = port;
    socket_addr.sin_addr.S_un.S_un_b.s_b1 = ip.bytes[0];
    socket_addr.sin_addr.S_un.S_un_b.s_b2 = ip.bytes[1];
    socket_addr.sin_addr.S_un.S_un_b.s_b3 = ip.bytes[2];
    socket_addr.sin_addr.S_un.S_un_b.s_b4 = ip.bytes[3];


    if (bind(server_socket, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "ERROR: bind failed because, %d\n", WSAGetLastError());
        exit(1);
    }


    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        fprintf(stderr, "ERROR: listen failed because, %d\n", WSAGetLastError());
        exit(1);
    }

    is_active = true;

    void *accept_thread = spawn_thread(accept_incoming_connections, &server_socket); (void)accept_thread;


    while (is_active)
    {}


}