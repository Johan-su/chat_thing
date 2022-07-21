#define _CRT_SECURE_NO_WARNINGS

#include "data.hpp"
#include "client.hpp"
#include "server.hpp"


#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>



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

static void init_config(int argc, char *argv[])
{
    char *program = *argv++;
/*
    if (argc <= 1)
    {
        fprintf(stderr, "ERROR: Incorrect amount of arguments\n");
        usage_and_quit(program);
    }
*/

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
}



static void init_windows_socket_lib()
{
    WORD version = MAKEWORD(2, 2);
    WSADATA data;
    if (WSAStartup(version, &data) != 0)
    {
        fprintf(stderr, "ERROR: WSAstartup failed because, %d\n", WSAGetLastError());
        exit(1);
    }
}


int main(int argc, char *argv[])
{
    init_config(argc, argv);
    init_windows_socket_lib();


    if (config.host)
    {
        server(config.ip, config.port);
    }
    else
    {
        client(config.ip, config.port);
    }
    
    WSACleanup();
    return 0;
}