#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <stdlib.h>

#define RCVBUFSIZE 32 /* Size of receive buffer */
#define NUM_BOLTUNS 5

static int phone_statuses[NUM_BOLTUNS];

void DieWithError(char *errorMessage); /* Error handling function */

void HandleTCPClient(int clntSocket)
{
    for (int i = 0; i < NUM_BOLTUNS; i++)
    {
        phone_statuses[i] = NUM_BOLTUNS + 1;
    }

    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    int recvMsgSize;             /* Size of received message */
    int command, id;

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    id = atoi(echoBuffer);

    /* Echo message back to client */
    if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
        DieWithError("send() failed");

    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0) /* zero indicates end of transmission */
    {

        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
        command = atoi(echoBuffer);
        phone_statuses[id] = command;
        if (command == NUM_BOLTUNS)
        {
            for (int i = 0; i < NUM_BOLTUNS; i++)
            {
                // someone wants to call waiting boltun
                if (phone_statuses[i] == id)
                {
                    sprintf(echoBuffer, "%d", i);
                    phone_statuses[i] = NUM_BOLTUNS + 1;
                    phone_statuses[id] = NUM_BOLTUNS + 1;
                }
                else
                {
                    sprintf(echoBuffer, "%d", NUM_BOLTUNS);
                }
            }
        }
        /* Echo message back to client */
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed");
    }

    close(clntSocket); /* Close client socket */
}
