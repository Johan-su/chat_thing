#include "client.hpp"
#include "threads.hpp"

#include <Windows.h>
#include <stdio.h>

#define BUF_LEN 2048

static unsigned long receive_print_loop(void *connect_socket_p)
{
    SOCKET connect_socket = *(SOCKET *)connect_socket_p;
    char rec_buf[BUF_LEN];
    while (true)
    {
        recv(connect_socket, rec_buf, BUF_LEN, 0);
        fprintf(stdout, "%s\n", rec_buf);
    }
}


void client(ipv4_addr ip, uint16_t port)
{
    sockaddr_in SOCK_addr;

    SOCK_addr.sin_family = AF_INET;
    SOCK_addr.sin_port = port;
    SOCK_addr.sin_addr.S_un.S_un_b.s_b1 = ip.bytes[0];
    SOCK_addr.sin_addr.S_un.S_un_b.s_b2 = ip.bytes[1];
    SOCK_addr.sin_addr.S_un.S_un_b.s_b3 = ip.bytes[2];
    SOCK_addr.sin_addr.S_un.S_un_b.s_b4 = ip.bytes[3];

    SOCKET connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (connect_socket == INVALID_SOCKET)
    {
        fprintf(stderr, "ERROR: creating socket failed");
        exit(1);
    }

    if (connect(connect_socket, (struct sockaddr *)&SOCK_addr, sizeof(SOCK_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "ERROR: connecting failed");
        exit(1);        
    }

    void *receive_thread = spawn_thread(receive_print_loop, &connect_socket);

    char buf[BUF_LEN];

    while (true)
    {
        scanf("%s", buf);

        if (buf[0] == '/' && buf[1] == 'q' && buf[2] == '\0')

        if (send(connect_socket, buf, BUF_LEN, 0) == SOCKET_ERROR)
        {
            fprintf(stderr, "ERROR: send failed with, %d", WSAGetLastError());
            exit(1);
        }
    }

    destroy_thread(receive_thread);
    closesocket(connect_socket);
}