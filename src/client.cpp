#define _CRT_SECURE_NO_WARNINGS
#include "client.hpp"
#include "threads.hpp"
#include "buffer_lengths.hpp"


#include <Windows.h>
#include <stdio.h>


volatile static bool is_active = false;

static unsigned long receive_print_loop(void *connect_socket_p)
{
    SOCKET connect_socket = *(SOCKET *)connect_socket_p;
    char rec_buf[REC_BUF_LEN];
    while (is_active)
    {
        recv(connect_socket, rec_buf, REC_BUF_LEN, 0);
        fprintf(stdout, "%s\n", rec_buf);
    }
    closesocket(connect_socket);
    return 0;
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
        fprintf(stderr, "ERROR: connecting failed\n");
        exit(1);        
    }

    is_active = true;

    void *receive_thread = spawn_thread(receive_print_loop, &connect_socket); (void)receive_thread;

    char name_buf[NAME_LEN];
    char message_buf[SEND_BUF_LEN];

    char send_buf[REC_BUF_LEN];


    sprintf(name_buf, "%llu :: ", connect_socket);

    while (is_active)
    {
        scanf("%s", message_buf);


        if (message_buf[0] == '/' && message_buf[1] == 'q' && message_buf[2] == '\0')
        {
            is_active = false;
            shutdown(connect_socket, 2); // SD_BOTH
            break;
        }

        strcpy(send_buf, name_buf);
        strcat(send_buf, message_buf);

        if (send(connect_socket, send_buf, REC_BUF_LEN, 0) == SOCKET_ERROR)
        {
            fprintf(stderr, "ERROR: send failed with, %d", WSAGetLastError());
            exit(1);
        }
    }
}