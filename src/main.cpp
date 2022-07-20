#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>




struct ipv4_addr
{
    uint8_t bytes[4];
};



static void usage_and_quit(const char *program)
{
    FILE *f_desc = stderr;

    fprintf(f_desc, "Usage: %s -i [Ipv4 (127.0.0.1)] -p [port (25565)]\n", program);
    fprintf(f_desc, "-h    specifies if host or client\n");
    fprintf(f_desc, "-p    port\n");
    fprintf(f_desc, "-i    ipv4 ip-address\n");
    exit(1);
}



static uint16_t parse_port(const char *port, const char *program)
{
    if (port == NULL)
    {
        fprintf(stderr, "ERROR: failed to parse port, port number must be provided\n");
        usage_and_quit(program);
    }

    int port_val = atoi(port);

    if (port_val < 0 || port_val > 65535)
    {
        fprintf(stderr, "ERROR: failed to parse port, port must be within [0-65535]\n");
        usage_and_quit(program);
    }

    return (uint16_t)port_val;
}


static ipv4_addr parse_ip(const char *ip, const char *program)
{
    if (ip == NULL)
    {
        fprintf(stderr, "ERROR: failed to parse ip, ip-address must be provided\n");
        usage_and_quit(program);
    }


    ipv4_addr ip_obj = {};

    const char *it = ip;

    for (int byte_it = 0; byte_it < 4; ++byte_it)
    {
        char ip_buffer[4];
        int period_pos = 0;

        while (*it != '.' && *it != '\0')
        {
            ip_buffer[period_pos] = *it;
            ++it;
            period_pos += 1;
        }
        ++it;

        if (period_pos == 0 || period_pos > 3)
        {
            fprintf(stderr, "ERROR: failed to parse ip, ip part can only have [1-3] digits between .\n");
            usage_and_quit(program);
        }
        
        ip_buffer[period_pos] = '\0';

        int byte_val = atoi(ip_buffer);

        if (byte_val < 0 || byte_val > 255)
        {
            fprintf(stderr, "ERROR: failed to parse ip, ip part must be within [0-255]\n");
            usage_and_quit(program);
        }

        ip_obj.bytes[byte_it] = (uint8_t)byte_val;
    }
    return ip_obj;
}

static void listener(SOCKET sockfd)
{
if (listen(sockfd, SOMAXCONN) == SOCKET_ERROR)
    {
        fprintf(stderr, "ERROR: listen failed because, %d\n", WSAGetLastError());
        exit(1);
    }


    SOCKET client_socket = accept(sockfd, NULL, NULL);

    closesocket(sockfd);

    #define MESSAGE_BUFFER_LEN 2048

    char buffer [MESSAGE_BUFFER_LEN];

    int result;
    do
    {
        result = recv(client_socket, buffer, MESSAGE_BUFFER_LEN, 0);

        if (result > 0)
        {
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


    } while (result > 0);
    
    int shutdown_r = shutdown(client_socket, 1); // SD_SEND
    closesocket(client_socket);
}


static void client(const sockaddr *addr)
{
    connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int result = connect(connect_socket, addr, sizeof(*addr));
    //TODO(Johan) finish client
}

enum FLAGS
{
    None,
    Host,
    Port,
    Ip,
};


static FLAGS parse_flag(const char *flag)
{
    if (strlen(flag) < 2 || flag[0] != '-')
    {
        return FLAGS::None;
    }


    switch (flag[1])
    {
        case 'h':
        {
            return FLAGS::Host;
        } break;


        case 'p':
        {
            return FLAGS::Port;
        } break;


        case 'i':
        {
            return FLAGS::Ip;
        } break;


        default:
        {
            return FLAGS::None;
        } break;
    }
}



struct config_t
{
    ipv4_addr ip;
    uint16_t port;
    bool host;
};


static config_t config = {
    .ip = {127, 0, 0, 1},
    .port = 25565,
    .host = false,

};

int main(int argc, char *argv[])
{
    char *program = *argv++;
    if (argc <= 1)
    {
        fprintf(stderr, "ERROR: Incorrect amount of arguments\n");
        usage_and_quit(program);
    }


    while (*argv != NULL)
    {
        switch (parse_flag(*(argv)++))
        {
            case FLAGS::None:
            {
                usage_and_quit(program);
            } break;


            case FLAGS::Host:
            {
                config.host = true;
            } break;


            case FLAGS::Port:
            {
                config.port = parse_port(*argv++, program);
            } break;


            case FLAGS::Ip:
            {
                config.ip = parse_ip(*argv++, program);
            } break;
        }
    }

    fprintf(stderr, "DEBUG: config values [ip: %d.%d.%d.%d port: %d host: %d]\n", config.ip.bytes[0], config.ip.bytes[1], config.ip.bytes[2], config.ip.bytes[3], config.port, config.host);


    WORD version = MAKEWORD(2, 2);

    WSADATA data;

    int error_code = WSAStartup(version, &data);    

    if (error_code != 0)
    {
        fprintf(stderr, "ERROR: WSAstartup failed because, %d\n", WSAGetLastError());
        exit(1);
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, "ERROR: socket failed because, %d\n", WSAGetLastError());
        exit(1);
    }


    sockaddr_in sockaddress;

    sockaddress.sin_family = AF_INET;
    sockaddress.sin_port = config.port;
    sockaddress.sin_addr.S_un.S_un_b.s_b1 = config.ip.bytes[0];
    sockaddress.sin_addr.S_un.S_un_b.s_b2 = config.ip.bytes[1];
    sockaddress.sin_addr.S_un.S_un_b.s_b3 = config.ip.bytes[2];
    sockaddress.sin_addr.S_un.S_un_b.s_b4 = config.ip.bytes[3];

    if (bind(sockfd, (struct sockaddr *)&sockaddress, sizeof(sockaddress)) == SOCKET_ERROR)
    {
        fprintf(stderr, "ERROR: bind failed because, %d\n", WSAGetLastError());
        exit(1);
    }

    if (config.host)
    {
        listener(sockfd);
    }
    else
    {

    }
    
    WSACleanup();
    return 0;
}