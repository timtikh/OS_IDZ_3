#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

void DieWithError(char *errorMessage); /* Error handling function */

enum REQUEST_CODE
{
    WAIT_CALL = 0,
    MAKE_CALL = 1,
    END_CALL = 2
};

enum RESPONSE_CODE
{
    UB = -1,
    CALL_ACCEPTED = 0,
    NO_ANSWER = 1,
    END = 2,
    NO_CALL = 3,
    CALL_RECEIVED = 4,
    FINISH = 5
};

struct call
{
    int id;
    int caller_id;
    int receiver_id;
};

struct request
{
    int request_code;
    int boltun_id;

    struct call call;
};

struct response
{
    int response_code;
    struct call call;
};