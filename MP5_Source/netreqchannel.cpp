
#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include <errno.h>

#include "netreqchannel.hpp"

NetworkRequestChannel::NetworkRequestChannel(const string _server_host_name, const unsigned short _port_no)
{
    my_side = Side::CLIENT;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    struct hostent *server;
    server = gethostbyname(_server_host_name.c_str());
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    struct sockaddr_in server_addr;
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(_port_no);
    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("ERROR connecting");
        exit(1);
    }
}

NetworkRequestChannel::NetworkRequestChannel(const unsigned short _port_no, void *(*connection_handler)(int *), int backlog)
{
    my_side = Side::SERVER;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port_no);
    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("ERROR binding");
        exit(1);
    }

    if (listen(fd, backlog) < 0)
    {
        perror("ERROR listening");
        exit(1);
    }
}

NetworkRequestChannel::~NetworkRequestChannel()
{
    close(fd);
}

const int MAX_MESSAGE = 255;

string NetworkRequestChannel::send_request(string _request)
{
    cwrite(_request);
    string s = cread();
    return s;
}

string NetworkRequestChannel::cread()
{
    char buf[MAX_MESSAGE];
    if (my_side == Side::SERVER)
    {
        struct sockaddr_in client_addr;
        int cliLen = sizeof(client_addr);
        int fdNew = accept(fd, (struct sockaddr *)&client_addr, &cliLen);
        if (fdNew < 0)
        {
            perror("ERROR accepting");
            exit(1);
        }
        if (read(fdNew, buf, MAX_MESSAGE) < 0)
        {
            perror(string("Network Request Channel : Error reading from socket!").c_str());
            exit(1);
        }
    }
    else
    {
        if (read(fd, buf, MAX_MESSAGE) < 0)
        {
            perror(string("Network Request Channel : Error reading from socket!").c_str());
            exit(1);
        }
    }
    string s = buf;
    return s;
}

int NetworkRequestChannel::cwrite(string _msg)
{
    if (_msg.length() >= MAX_MESSAGE)
    {
        cerr << "Message too long for Channel!" << std::endl;
    }
    const char *s = _msg.c_str(); // NOTE: c_str() is NOT thread safe!!

    if (my_side == Side::SERVER)
    {
        struct sockaddr_in client_addr;
        int cliLen = sizeof(client_addr);
        int fdNew = accept(fd, (struct sockaddr *)&client_addr, &cliLen);
        if (fdNew < 0)
        {
            perror("ERROR accepting");
            exit(1);
        }
        if (write(fdNew, s, strlen(s) + 1) < 0)
        {
            perror(string("Network Request Channel : Error writing to socket!").c_str());
            exit(1);
        }
    }
    else
    {
        if (write(fd, s, strlen(s) + 1) < 0)
        {
            perror(string("Network Request Channel : Error writing to socket!").c_str());
            exit(1);
        }
    }
}