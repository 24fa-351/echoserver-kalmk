#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define PORT 46645
#define LISTEN_BACKLOG 5
#define BUFFER_SIZE 1024

void *handleConnection(void *client_socket)
{
  int a_client = *(int *)client_socket;
  free(client_socket);
  char buffer[BUFFER_SIZE];

  while (1)
  {
    int bytes_read = read(a_client, buffer, sizeof(buffer));
    if (bytes_read <= 0)
    {
      printf("Client disconnected or read error\n");
      break;
    }
    buffer[bytes_read] = '\0';
    printf("Received: %s\n", buffer);
    write(a_client, buffer, bytes_read);
  }

  close(a_client);
  return NULL;
}

int main(int argc, char *argv[])
{
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in socket_address;
  memset(&socket_address, '\0', sizeof(socket_address));
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
  socket_address.sin_port = htons(PORT);

  if (bind(socket_fd, (struct sockaddr *)&socket_address, sizeof(socket_address)) < 0)
  {
    perror("Bind failed");
    return 1;
  }

  if (listen(socket_fd, LISTEN_BACKLOG) < 0)
  {
    perror("Listen failed");
    return 1;
  }

  printf("Server listening on port %d\n", PORT);

  while (1)
  {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int *client_fd = malloc(sizeof(int));
    *client_fd = accept(socket_fd, (struct sockaddr *)&client_address, &client_address_len);
    if (*client_fd < 0)
    {
      perror("Accept failed");
      free(client_fd);
      continue;
    }

    printf("Client connected\n");

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, handleConnection, client_fd) != 0)
    {
      perror("Failed to create thread");
      close(*client_fd);
      free(client_fd);
    }
    else
    {
      pthread_detach(thread_id);
    }
  }

  close(socket_fd);
  return 0;
}
