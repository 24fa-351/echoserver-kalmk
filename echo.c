#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 46645
#define LISTEN_BACKLOG 5

void handleConnection(int client_fd)
{
    char buffer[1024];
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);
        write(client_fd, buffer, bytes_read);
    }
    close(client_fd);
}

int main(int argc, char *argv[])
{
    int port = PORT; // default port
    if (argc > 2 && strcmp(argv[1], "-p") == 0)
    {
        port = atoi(argv[2]);
        if (port <= 0 || port > 65535)
        {
            fprintf(stderr, "Invalid port number. Please specify a port "
                            "between 1 and 65535.\n");
            return 1;
        }
    }
    else if (argc > 1)
    {
        fprintf(stderr, "Usage: %s [-p port]\n", argv[0]);
        return 1;
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Failed to create socket");
        return 1;
    }

    struct sockaddr_in socket_address;
    memset(&socket_address, '\0', sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(port);

    printf("Server listening on port %d\n", port);

    if (bind(socket_fd, (struct sockaddr *)&socket_address,
             sizeof(socket_address)) < 0)
    {
        perror("Failed to bind");
        close(socket_fd);
        return 1;
    }

    if (listen(socket_fd, LISTEN_BACKLOG) < 0)
    {
        perror("Failed to listen");
        close(socket_fd);
        return 1;
    }

    while (1)
    {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        int client_fd = accept(socket_fd, (struct sockaddr *)&client_address,
                               &client_address_len);
        if (client_fd < 0)
        {
            perror("Failed to accept connection");
            continue; // continue accepting new connections
        }

        handleConnection(client_fd);
    }

    close(socket_fd);
    return 0;
}