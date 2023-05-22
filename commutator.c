#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int server_socket, client_socket, read_size;
    struct sockaddr_in server_addr, client_addr;
    char client_message[200];

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        printf("Could not create socket \n");
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Bind failed \n");
        return 1;
    }

    // Listen
    listen(server_socket, 3);

    // Accept and incoming connection
    printf("Waiting for incoming connections... \n");
    int c = sizeof(struct sockaddr_in);
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&c)))
    {
        printf("Connection accepted \n");

        // Receive a message from client
        while ((read_size = recv(client_socket, client_message, 200, 0)) > 0)
        {
            // Send the message back to client
            write(client_socket, client_message, strlen(client_message));
            memset(client_message, '\0', sizeof(client_message));
        }

        if (read_size == 0)
        {
            printf("Client disconnected \n");
        }
        else if (read_size == -1)
        {
            printf("Receive failed \n");
        }
    }

    if (client_socket < 0)
    {
        printf("Accept failed \n");
        return 1;
    }

    return 0;
}