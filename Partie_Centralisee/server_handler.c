#ifdef WIN32

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#elif defined(linux)

#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else

#error platform unsupported

#endif

#include "server_handler.h"
#include "user_database_handler.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#define SERVER_PORT 24020
#define SERVER_QUEUE 10

struct client_t {
    pthread_t thread_id;
    SOCKET socket;
    SOCKADDR_IN addr;
    socklen_t addr_len;
};

void sock_err(char *action);

void *client_handler(void *arg);

_Noreturn void *server_handler(void *arg) {

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        sock_err("Creating server socket");
    }

    SOCKADDR_IN sin = {
            .sin_addr.s_addr = htonl(INADDR_ANY),
            .sin_family = AF_INET,
            .sin_port = htons(SERVER_PORT)
    };

    if (bind(server_socket, (SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR) {
        sock_err("Binding server socket");
    }

    if (listen(server_socket, SERVER_QUEUE) == SOCKET_ERROR) {
        sock_err("Setting server socket as listener");
    }

    while (1) {
        struct client_t *client = malloc( sizeof *client);
        client->addr_len = sizeof client->addr;

        client->socket = accept(
                server_socket,
                (SOCKADDR *) &client->addr,
                (socklen_t *) &client->addr_len
        );

        if (client->socket == INVALID_SOCKET) {
            sock_err("Accepting incoming client connection");
        }

        pthread_create(
                &client->thread_id,
                NULL,
                &client_handler,
                client
        );
    }
}

void *client_handler(void *arg) {

    struct client_t *client = (struct client_t *) arg;

    printf("Accepted connection for client #%d\n", client->socket);

    char buffer[1024];

    ssize_t n;
    while ((n = recvfrom(
            client->socket,
            buffer, sizeof buffer - 1,
            0,
            (SOCKADDR *) &client->addr,
            &client->addr_len
    )) > 0) {
        buffer[n] = '\0';
        printf("Request submitted by client #%d : %s\n", client->socket, buffer);

        user_database_request(buffer);

        if (sendto(
                client->socket,
                buffer, (size_t) strlen(buffer),
                0,
                (SOCKADDR *) &client->addr,
                client->addr_len
        ) <= 0) {
            sock_err("Sending data to client");
        }
    }


    printf("Client #%d disconnected\n", client->socket);
    free(client);
    pthread_exit(EXIT_SUCCESS);
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
