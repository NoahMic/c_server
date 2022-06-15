#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

struct server
{
    int domain;
    int service;
    int protocol;
    u_long interface;
    int port;
    int backlog;
    struct sockaddr_in address;
    int socket;
};

struct server server_constructor(int domain, int service, int protocol, u_long interface, int port, int backlog)
{
    struct server server;

    server.domain = domain;
    server.service = service;
    server.protocol = protocol;
    server.interface = interface;
    server.port = port;
    server.backlog = backlog;

    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.socket = socket(domain, service, protocol);

    if (server.socket == 0)
    {
        perror("connect error\n");
        exit(1);
    }

    if (bind(server.socket, (struct sockaddr *)&server.address, sizeof(server.address)) < 0)
    {
        perror("bind error\n");
        exit(1);
    }
    if (listen(server.socket, server.backlog) < 0)
    {

        perror("listen error\n");
        exit(1);
    }

    return server;
}

char *res(char *text)
{
    static char http[30000] = "";

    char path[3000] = "html";
    strcat(text, ".html");
    strcat(path, text);
    int fd;
    if (strcmp("html/.html", path) == 0)
    {
        fd = open("index.html", O_RDONLY);
    }
    else
    {
        fd = open(path, O_RDONLY);
    }

    char html[20000] = "";
    if (fd < 0)
    {
        fd = open("./404.html", O_RDONLY);
        read(fd, html, 20000);
    }
    else
    {
        read(fd, html, 20000);
    }

    sprintf(http, "HTTP/1.1 200 OK\nServer: Apache\nConnection: Closed\nContent-Type: text/html; charset=utf-8\nContent-Length: %ld\n\n%s", strlen(html), html);
    return http;
}

void lanch(struct server *server)
{
    char buffer[30000] = "";
    char http[30000] = "";
    int address_length = sizeof(server->address);
    int new_socket;
    while (1)
    {
        new_socket = accept(server->socket, (struct sockaddr *)&server->address, (socklen_t *)&address_length);
        read(new_socket, buffer, 30000);
        printf("%s\n", buffer);
        char *http_method = strtok(buffer, " ");
        char *path = strtok(NULL, " ");
        if (strcmp(http_method, "GET") == 0)
        {
            strcpy(http, res(path));
            write(new_socket, http, strlen(http));
        }
        close(new_socket);
    }
}

int main()
{
    struct server server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 3000, 10);
    lanch(&server);
}
