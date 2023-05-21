#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include "ipaddress.h"

#define PORT 8080

struct Response
{
    char *protocol;
    char *content_type;
    char *response;
    int status;
};

char *create_protocol(int num)
{
    char *protocol = malloc(sizeof(char) * num == 1 ? 8 : 6);
    if (num == 2)
    {
        protocol = "HTTP/2";
    }
    else
    {
        protocol = "HTTP/1.1";
    }
    return protocol;
}

int get_size_of_response(int protocol, char status[3])
{
    if (protocol == 1)
    {
        return 21;
    }
}

char *create_response_header_line(int protoc, char status[3])
{
    char *protocol = create_protocol(protoc);
    printf("%s\n", protocol);
    char *status_line = malloc(sizeof(char) * get_size_of_response(protoc, status));
    memset(status_line, 0, sizeof(status_line));

    int len_protoc = strlen(protocol);

    int space = -1;
    for (int i = 0; i < len_protoc; i++)
    {
        status_line[++space] = protocol[i];
    }

    status_line[++space] = ' ';

    for (int i = 0; i < 3; i++)
    {
        status_line[++space] = status[i];
    }

    status_line[++space] = ' ';

    char *stat = "OK";
    for (int i = 0; i < 2; i++)
    {
        status_line[++space] = stat[i];
    }
    status_line[++space] = '\r';
    status_line[++space] = '\n';

    return status_line;
}

void send_response(FILE *f, char *key, char *value)
{
    fprintf(f, "%s", create_response_header_line(1, "200"));
    printf("Repsonse Status : %s", create_response_header_line(1, "200"));
    fprintf(f, "Content-Type: application/json\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "{\"%s\": \"%s\"}", key, value);
}

uint16_t make_server(uint16_t port)
{
    int sock;
    struct sockaddr_in addr_in;
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // handle error, -1 means failed to create socket
    // todo - do better handling
    if (sock == -1)
    {
        printf("Error creating socket, exiting.");
        exit(EXIT_FAILURE);
    }

    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(port);

    bind(sock, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in));

    if (listen(sock, 8) == -1)
    {
        printf("Failed to get socket to listen (%d)\n", errno);
        exit(EXIT_FAILURE);
    }

    return sock;
}

void accept_client(int sock, char *key)
{
    struct sockaddr_in client_addr;
    socklen_t clientaddr_len;
    FILE *f;

    int client_sock = accept(sock, (struct sockaddr *)&client_addr, &clientaddr_len);

    if (client_sock == -1)
    {
        printf("Failed to accept connection (%d)\n", errno);
        exit(EXIT_FAILURE);
    }

    printf("server::accepted connection\n");

    f = fdopen(client_sock, "w+");

    send_response(f, key, get_ip_address(f));

    printf("server::sent response\n");

    fclose(f);

    // stdout needs to be flushed in order for heroku to read the logs
    fflush(stdout);
    printf("simpleserver::endfflush\n");
}

void send_only(int port, char *key)
{
    int sock = make_server(port);

    while (1)
    {
        accept_client(sock, key);
    }
    close(sock);
}

int main()
{
    int port = PORT;
    send_only(port, "address");
    exit(EXIT_SUCCESS);
    // printf("no clue\n");
}