#ifdef WIN32

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#elif defined(linux)

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#define INVALID_SOCKET (-1)
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else

#error platform unsupported

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user_database_handler.h"

#define DATABASE_ADDR "localhost"
#define DATABASE_PORT 24030

/**
 * Displays a message corresponding to the last error, depending on the
 * implementation given by the platform.
 *
 * @param action a string describing the action which failed
 */
static void sock_err(char *action);

static SOCKET database_socket;

static SOCKADDR_IN to = {0};

void user_database_open() {

    database_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (database_socket == INVALID_SOCKET) {
        sock_err("Creating socket");
    }

    struct hostent *hostinfo = gethostbyname(DATABASE_ADDR);
    if (hostinfo == NULL) {
        fprintf(stderr, "Internal error");
        exit(EXIT_FAILURE);
    }

    to.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
    to.sin_port = htons(DATABASE_PORT);
    to.sin_family = AF_INET;
}

void user_database_close() {
    closesocket(database_socket);
    to = (SOCKADDR_IN) {0};
}

int user_database_request(char* request) {

    char buffer[1024];
    strcpy(buffer, request);

    size_t n = sendto(
            database_socket,
            buffer, (int) strlen(buffer),
            0,
            (SOCKADDR *) &to,
            sizeof to
    );
    if (n < 0) {
        sock_err("Sending request");
    }

    n = recvfrom(
            database_socket,
            buffer, sizeof buffer - 1,
            0,
            NULL, NULL
    );
    if (n < 0) {
        sock_err("Acquiring response");
    }

    buffer[n] = '\0';
    strcpy(request, buffer);

    return (int) n;
}

/* -------------------------------------------------------------------------- */

void sock_err(char *action) {
#ifdef WIN32
    int err_code = WSAGetLastError();
    char msg[256];
    msg[0] = '\0';
    FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            msg, sizeof msg,
            NULL
    );
    fprintf(stderr, "%s: %s\n", action, msg);
    exit(err_code);
#elif defined(linux)
    perror(action);
    exit(errno);
#else
#error platform unsupported
#endif
}