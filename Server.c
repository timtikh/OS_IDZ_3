#include <pthread.h>   /* for POSIX threads */
#include <semaphore.h> /* for unnamed "mutexes" */
#include <signal.h>    /* for handling SIGINT */
#include "TCPLib.h"

#define MAX_BOLTUNS 5

void HandleTCPClient(int clntSocket);           /* TCP client handling function */
int CreateTCPServerSocket(unsigned short port); /* Create TCP server socket */
int AcceptTCPConnection(int servSock);          /* Accept TCP connection request */

void *ThreadMain(void *arg); /* Main program of a thread */

void printCallsInfo(); /* Print status info for logging */

/* Structure of arguments to pass to client thread */
struct ThreadArgs
{
    int clntSock; /* Socket descriptor for client */
};

struct call calls[MAX_BOLTUNS];
int calls_count, complete_count = 0;

sem_t sem;
sem_t print;

int servSock;       /* Socket descriptor for server */
pthread_t threadID; /* Thread ID from pthread_create() */

void closeAll()
{
    printf("\n\nFINISH USING SIGINT\n\n");
    printCallsInfo();

    sem_destroy(&sem);   // Уничтожение семафора
    sem_destroy(&print); // Уничтожение семафора

    close(servSock);
}

void handleSigInt(int sig)
{
    if (sig != SIGINT)
    {
        return;
    }
    closeAll();
    // kill thread
    pthread_kill(threadID, SIGKILL);
    exit(0);
}

void initPulls()
{
    // initialize calls
    for (int i = 0; i < calls_count; ++i)
    {
        struct call call = {.id = i, .caller_id = -1, .receiver_id = -1};
        calls[i] = call;
    }
}

void *ThreadMain(void *threadArgs)
{
    int clntSock; /* Socket descriptor for client connection */

    /* Guarantees that thread resources are deallocated upon return */
    pthread_detach(pthread_self());

    /* Extract socket file descriptor from argument */
    clntSock = ((struct ThreadArgs *)threadArgs)->clntSock;
    free(threadArgs); /* Deallocate memory for argument */

    HandleTCPClient(clntSock);

    return (NULL);
}

int CreateTCPServerSocket(unsigned short port)
{
    int sock;                        /* socket to create */
    struct sockaddr_in echoServAddr; /* Local address */

    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(port);              /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    {
        DieWithError("bind() failed");
    }

    /* Mark the socket so it will listen for incoming connections */
    if (listen(sock, MAX_BOLTUNS) < 0)
    {
        DieWithError("listen() failed");
    }

    return sock;
}

int AcceptTCPConnection(int servSock)
{
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int clntLen;            /* Length of client address data structure */

    /* Set the size of the in-out parameter */
    clntLen = sizeof(echoClntAddr);

    /* Wait for a client to connect */
    if ((clntSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen)) < 0)
    {
        DieWithError("[-]\tAccept error");
    }
    printf("[+]\tClient connected\n");
    /* clntSock is connected to a client! */

    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

    return clntSock;
}

//////  TASK LOGIC  //////

void printCallsInfo()
{
    sem_wait(&print);
    for (int j = 0; j < calls_count; ++j)
    {
        printf("call with id = %d must be called by boltun #%d, must be received by boltun %d\n", calls[j].id, calls[j].caller_id, calls[j].receiver_id);
    }
    sem_post(&print);
}

void findCallers(struct response *response, struct call *call, int requestAuthor)
{
    calls[call->id] = *call;
    response->call = *call;
    response->response_code = NO_CALL;
    for (int i = 0; i < calls_count; ++i)
    {
        if (i != call->id)
        {
            if (calls[i].receiver_id == requestAuthor)
            {
                printf("Boltun #%d received call from boltun %d\n", requestAuthor, i);
                response->response_code = CALL_RECEIVED;
                response->call = calls[i];
                response->call.receiver_id = requestAuthor;
                response->call.caller_id = i;
            }
        }
    }
}

void findWaiters(struct response *response, struct call *call, int requestAuthor)
{
    calls[call->id] = *call;
    response->call = *call;
    response->response_code = NO_ANSWER;

    for (int i = 0; i < calls_count; ++i)
    {
        if (i != call->id)
        {
            if (call->receiver_id == calls[i].receiver_id && calls[i].caller_id == -1)
            {
                printf("Boltun #%d accepted call from boltun %d\n", i + 1, requestAuthor + 1);
                calls[i].caller_id = requestAuthor;
                // response->call = *call;
                response->response_code = CALL_ACCEPTED;
            }
            if (call->receiver_id == calls[i].caller_id && calls[i].receiver_id == call->caller_id)
            {
                printf("Boltun #%d accepted call from boltun %d\n", i + 1, requestAuthor + 1);
                // response->call = calls[i];
                response->response_code = CALL_ACCEPTED;
            }
            // add when its just waits;
        }
    }
}

void endCall(struct response *response, struct call *call, int requestAuthor)
{
    calls[call->id] = *call;
    response->response_code = END;
}

int handleClientRequest(int clntSocket, struct request *request)
{

    struct call null_task = {-1, -1, -1};
    struct response response = {-1, null_task};

    // critical section, access to calls array
    sem_wait(&sem);

    if (complete_count == calls_count)
    {
        response.response_code = FINISH;

        printf("\n\nFINISH\n\n");
        printCallsInfo();
    }
    else
    {
        int requestAuthor = request->boltun_id;
        struct call call = request->call;

        switch (request->request_code)
        {
        case 0: // wait for calls
            findCallers(&response, &call, requestAuthor);
            break;
        case 1: // make call
            findWaiters(&response, &call, requestAuthor);
            break;
        case 2: // end call
            endCall(&response, &call, requestAuthor);
            break;
        default: // ub
            break;
        }
    }

    // end of critical section
    sem_post(&sem);

    // Send the response back to the client
    send(clntSocket, &response, sizeof(response), 0);

    return response.response_code;
}

//////  END OF TASK LOGIC  //////

void receiveRequest(int sock, struct request *request)
{
    /* Receive the current i to the server */
    if (recv(sock, (struct request *)request, sizeof(*request), 0) < 0)
    {
        DieWithError("recv() bad");
    }
    printf("Server has received request = %d from Boltun %d\n", request->request_code, request->boltun_id + 1);
}

void HandleTCPClient(int clntSocket)
{
    while (1) // until complete-pull is complete
    {
        struct request request = {-1, -1, -1};

        receiveRequest(clntSocket, &request);

        if (handleClientRequest(clntSocket, &request) == FINISH)
        {
            break;
        }
    }

    close(clntSocket); /* Close client socket */
}

int main(int argc, char *argv[])
{
    (void)signal(SIGINT, handleSigInt);

    int clntSock;                  /* Socket descriptor for client */
    unsigned short port;           /* Server port */
    struct ThreadArgs *threadArgs; /* Pointer to argument structure for thread */

    sem_init(&sem, 0, 1);   // Инициализация семафора
    sem_init(&print, 0, 1); // Инициализация семафора

    port = 7004; /* First arg:  local port */

    if (argc < 2) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Аргументы:  %s [SERVER PORT] [BOLTUN_COUNT]\n", argv[0]);
        // exit(1);
    }
    else
    {
        port = atoi(argv[1]); /* First arg:  local port */
    }

    calls_count = MAX_BOLTUNS;
    if (argc > 2)
    {
        calls_count = atoi(argv[2]);
        // in order to have calls_count lower (or equal) than max (and 2 and more)
        calls_count = (calls_count > MAX_BOLTUNS || calls_count < 2) ? MAX_BOLTUNS : calls_count;
    }
    initPulls();

    servSock = CreateTCPServerSocket(port);

    while (complete_count < calls_count) /* run forever */
    {
        clntSock = AcceptTCPConnection(servSock);

        /* Create separate memory for client argument */
        if ((threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs))) == NULL)
            DieWithError("malloc() failed");
        threadArgs->clntSock = clntSock;

        /* Create client thread */
        if (pthread_create(&threadID, NULL, ThreadMain, (void *)threadArgs) != 0)
            DieWithError("pthread_create() failed");
        printf("with thread %ld\n", (long int)threadID);
    }
    /* NOT REACHED */
}