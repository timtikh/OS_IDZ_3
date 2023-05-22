#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 32 /* Size of receive buffer */
#define NUM_BOLTUNS 5
// Change waiting time for call in seconds here
#define MAX_WAIT_TIME 5

int make_call(int id);
int get_random_phone_number(int id);
int get_random_wait_time();

void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[])
{
    int sock, id;                    /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *echoString;                /* String to send to echo server */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv()
                                        and total bytes read */

    if ((argc < 3) || (argc > 4)) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>] <boltun id>\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1];     /* First arg: server IP address (dotted quad) */
    echoString = argv[3]; /* Second arg: string to echo */

    if (argc == 4)
        echoServPort = atoi(argv[2]); /* Use given port, if any */
    else
        echoServPort = 7; /* 7 is the well-known port for the echo service */

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");

    id = atoi(echoString);
    echoStringLen = strlen(echoString); /* Determine input length */

    // sending id
    if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
        DieWithError("send() sent a different number of bytes than expected");
    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed or connection closed prematurely");

    for (;;)
    {

        // decide what to do
        int command = make_call(id);
        sprintf(echoString, "%d", command);
        echoStringLen = strlen(echoString); /* Determine input length */
        /* Send the call to the server */
        if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
            DieWithError("send() sent a different number of bytes than expected");

        /* Receive the same string back from the server */
        sleep(2);

        totalBytesRcvd = 0;
        while (totalBytesRcvd < echoStringLen)
        {
            /* Receive up to the buffer size (minus 1 to leave space for
            a null terminator) bytes from the sender */
            if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                DieWithError("recv() failed or connection closed prematurely");
            totalBytesRcvd += bytesRcvd; /* Keep tally of total bytes */
        }
        if (atoi(echoBuffer) == NUM_BOLTUNS || atoi(echoBuffer) == id)
        {
            printf("Boltun %d was unable to do its action\n", id + 1);
            int wait_time = get_random_wait_time();
            usleep(wait_time * 1000000);
        }
        else
        {
            printf("Boltun %d is talking with Boltun %d\n", id + 1, atoi(echoBuffer) + 1);
            int wait_time = get_random_wait_time();
            usleep(wait_time * 1000000);
        }
        printf("\n"); /* Print a final linefeed */
        sleep(2);
    }

    close(sock);
    exit(0);
}

int make_call(int id)
{
    int phone_number = get_random_phone_number(id);
    if (phone_number == NUM_BOLTUNS)
    {
        printf("Boltun %d is waiting for call \n", id + 1);
    }
    else
    {
        printf("Boltun %d is calling Boltun %d\n", id + 1, phone_number + 1);
    }
    return phone_number;
}

int get_random_phone_number(int id)
{
    int phone_number;
    do
    {
        phone_number = rand() % (NUM_BOLTUNS + 1);
    } while (phone_number == id);
    return phone_number;
}

int get_random_wait_time()
{
    return rand() % MAX_WAIT_TIME + 1;
}