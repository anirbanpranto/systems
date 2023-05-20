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

void send_response(FILE *f, char *key, char *value)
{
    fprintf(f, "HTTP/1.1 200 OK\r\n");
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

    
    f = fdopen(client_sock, "w+");

    send_response(f, key, get_ip_address(f));

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
}