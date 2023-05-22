#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int socket_desc;
    struct sockaddr_in server;
    char message[200], server_reply[200];

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket \n");
    }

    // Prepare the sockaddr_in structure
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    // Connect to remote server
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connect failed\n");
        return 1;
    }

    printf("Connected to server\n");

    while (1)
    {
        printf("Enter message: ");
        scanf("%s", message);

        // Send some data
        if (send(socket_desc, message, strlen(message), 0) < 0)
        {
            printf("Send failed\n");
            return 1;
        }

        // Receive a reply from the server
        memset(server_reply, '\0', sizeof(server_reply));
        if (recv(socket_desc, server_reply, 200, 0) < 0)
        {
            printf("Receive failed\n");
            break;
        }

        printf("Server reply: %s \n", server_reply);
    }

    close(socket_desc);
    return 0;
}