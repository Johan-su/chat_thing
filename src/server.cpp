#define _CRT_SECURE_NO_WARNINGS
#include "server.hpp"
#include "threads.hpp"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <Windows.h>



#define MAX_CONNECTIONS 16

struct socket_pool_t
{
    size_t active_socket_count;
    SOCKET sockets[MAX_CONNECTIONS];
    bool active_socket[MAX_CONNECTIONS];
};


socket_pool_t socket_pool = {};


static unsigned long receive_and_send_messages(void *unused)
{
    
}


static unsigned long accept_incoming_connections(void *server_socket_p)
{
    socket_pool.active_socket_count = 0;
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        socket_pool.sockets[i] = INVALID_SOCKET;
        socket_pool.active_socket[i] = false;
    }

    SOCKET server_socket = *(SOCKET *)server_socket_p;
    while (true)
    {
        if (socket_pool.active_socket_count < MAX_CONNECTIONS)
        {
            int pos = -1;
            
            for (int i = 0; i < MAX_CONNECTIONS; ++i)
            {
                if (socket_pool.active_socket[i] == false)
                {
                    pos = i;
                    break;
                }
            }
            assert(pos != -1); // should never be true

            SOCKET tmp_socket = accept(server_socket, NULL, NULL);
            if (tmp_socket == INVALID_SOCKET)
            {
                fprintf(stderr "ERROR: could not accept socket\n");
                exit(1);
            }
            socket_pool.active_socket[pos] = true;
            socket_pool.active_socket_count += 1;

            socket_pool.sockets[pos];
            
        }
        else
        {
            fprintf(stderr "WARNING: ignored connection, as max amount of connections [%d] has been reached\n", MAX_CONNECTIONS);
        }
    }
}


#define MESSAGE_BUFFER_LEN 2048

void server(ipv4_addr ip, uint16_t port)
{

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


    void *accept_thread = spawn_thread(accept_incoming_connections, &server_socket);

    char buffer [MESSAGE_BUFFER_LEN];


    int result;
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        result = recv(client_socket, buffer, MESSAGE_BUFFER_LEN, 0);

        if (result > 0)
        {
            printf("Recieved \n%s\n", buffer);
            int sendresult = send(client_socket, buffer, result, 0);
        }
        else if (result == 0)
        {
            printf("Connection closing\n");
        }
        else
        {
            fprintf(stderr, "ERROR: recv failed because, %d\n", WSAGetLastError());
            exit(1);
        }


    }


    destroy_thread(accept_thread);
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (socket_pool.active_socket[i])
        {
            closesocket(socket_pool.sockets[i]);
        }
    }
}