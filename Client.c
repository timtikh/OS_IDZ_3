#include "TCPLib.h"
#include <time.h>

int boltun_id;

void sendRequest(int sock, struct request *request)
{
    /* Send the current i to the server */
    if (send(sock, (struct request *)request, sizeof(*request), 0) < 0)
    {
        DieWithError("send() bad");
    }
    printf("Boltun #%d has sent his request = %d to server\n", boltun_id, request->request_code);
}

int chooseState()
{
    // Возвращает случайное число 0 или 2
    srand(time(NULL));
    return rand() % 2;
}

int getOtherBoltunNumber()
{
    int boltun = boltun_id - 1;
    while (boltun == boltun_id - 1)
    {
        boltun = rand() % 5;
    }
    return boltun;
}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */

    servIP = "127.0.0.1";

    boltun_id = -1;

    if (argc > 1)
    {
        boltun_id = atoi(argv[1]);
    }
    else
    {
        exit(1);
    }

    if ((argc < 3) || (argc > 4)) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Arguments: %s <Boltun id> [Server IP] [Server Port]\n",
                argv[0]);
        // exit(1);
    }
    else
    {
        servIP = argv[2]; /* First arg: server IP address (dotted quad) */
    }

    if (argc == 4)
    {
        echoServPort = atoi(argv[3]); /* Use given port, if any */
    }
    else
    {
        echoServPort = 7004; // default
    }

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    {
        DieWithError("connect() failed");
    }

    // TASK LOGIC //

    struct call call = {boltun_id - 1, boltun_id, 1 + rand() % 5};
    struct request request = {2, 0, call};

    // logic on processes

    while (1)
    {
        // deciding what to do
        if (request.request_code == END_CALL)
        {
            if (chooseState())
            {
                call.id = boltun_id - 1;
                call.caller_id = boltun_id - 1;
                call.receiver_id = getOtherBoltunNumber();
                request.boltun_id = boltun_id - 1;
                request.request_code = 1;
                request.call = call;
                printf("Boltun  #%d is trying to call %d from server\n", boltun_id, call.receiver_id + 1);
            }
            else
            {
                call.id = boltun_id - 1;
                call.caller_id = -1;
                call.receiver_id = boltun_id - 1;
                request.boltun_id = boltun_id - 1;
                request.request_code = 0;
                request.call = call;
                printf("Boltun  #%d is waiting calls from server\n", boltun_id);
            }
        }

        sendRequest(sock, &request);

        struct response response = {-1, -1, -1, -1};
        if (recv(sock, &response, sizeof(response), 0) < 0)
        {
            DieWithError("recv() bad");
        }
        // printf("Boltun  #%d has got the response = %d from server\n", boltun_id, response.response_code);

        if (response.response_code == FINISH)
        {
            break;
        }
        // Logic of making a decision
        switch (response.response_code)
        {
        case END:
            break;
        case NO_ANSWER:
            sleep(1);
            // request.call = response.call;
            call.receiver_id = getOtherBoltunNumber();
            request.call = call;
            request.request_code = MAKE_CALL;
            printf("Boltun  #%d is trying to call %d from server\n", boltun_id, call.receiver_id + 1);
            break;
        case CALL_ACCEPTED:
            sleep(1);
            // imitating conversation
            printf("!!!This Boltun talking to Boltun recv #%d \n", response.call.receiver_id + 1);
            request.call = response.call;
            request.request_code = END_CALL;

            sendRequest(sock, &request);

            struct response response = {-1, -1, -1, -1};
            if (recv(sock, &response, sizeof(response), 0) < 0)
            {
                DieWithError("recv() bad");
            }
            // printf("Boltun  #%d has got the response = %d from server\n", boltun_id, response.response_code);

            break;
        case CALL_RECEIVED:
            sleep(1);
            // imitating conversation
            printf("!!!This Boltun talking to Boltun caller #%d \n", response.call.receiver_id + 1);
            request.call = response.call;
            request.request_code = END_CALL;

            sendRequest(sock, &request);

            if (recv(sock, &response, sizeof(response), 0) < 0)
            {
                DieWithError("recv() bad");
            }
            // printf("Boltun  #%d has got the response = %d from server\n", boltun_id, response.response_code);

            break;
        case NO_CALL:
            request.request_code = WAIT_CALL;
            break;
        }
        sleep(5);
    }
    // END OF TASK LOGIC //

    close(sock);
    exit(0);
}