#ifdef WIN32

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#elif defined(linux)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
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
#include <signal.h>
#include "user_database_engine.h"

#define PORT 24030

SOCKET sock;

#ifdef WIN32

static BOOL WINAPI stop() {
    closesocket(sock);
    user_database_close();
    WSACleanup();
    exit(EXIT_SUCCESS);
}

#elif defined (linux)
static void stop(void) {
    closesocket(sock);
    user_database_close();
}
#endif

/**
 * Runs a given command to the user database engine.
 *
 * @param buffer contains the command to run as input, and the result as output
 */
static void run(char *buffer);

/**
 * Displays a message corresponding to the last error, depending on the
 * implementation given by the platform.
 *
 * @param action a string describing the action which failed
 */
static void sock_err(char *action);

/**
 * Main program.
 */
int main(void) {

#ifdef WIN32
    // If on Windows system, loads Winsock DLL
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (err < 0) {
        puts("WSAStartup failed");
        exit(EXIT_FAILURE);
    }
#endif

    // Initializes user database
    if (user_database_init() < 0) {
        fprintf(stderr, "Failed to initialize user database\n");
    }

    puts("Initialization done.");

    // Catch closing signals
#ifdef WIN32
    if (SetConsoleCtrlHandler(stop, TRUE) == 0) {
        sock_err("Windows CtrlHandler");
    }
#elif defined (linux)
    signal(SIGTERM, stop);
    signal(SIGINT, stop);
#endif

    // Create socket structure
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        sock_err("Creating socket");
    }

    // Create socket address (any)
    SOCKADDR_IN sin = {0};
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);

    // Bind address to socket
    if (bind(sock, (SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR) {
        sock_err("Binding socket");
    }

    int bytes_read, bytes_write;
    char msg_buffer[1024];

    char addr_buffer[INET_ADDRSTRLEN];

    SOCKADDR_IN from = {0};
    int from_size = sizeof from;

#pragma ide diagnostic ignored "EndlessLoop"
    while (1) {
        puts("Waiting for datagram...");
        bytes_read = recvfrom(
                sock,
                msg_buffer, sizeof msg_buffer - 1,
                0,
                (SOCKADDR *) &from, &from_size
        );
        if (bytes_read < 0) {
            sock_err("Receiving data");
        }
        msg_buffer[bytes_read] = '\0';

        inet_ntop(
                from.sin_family, &from.sin_addr,
                addr_buffer, sizeof addr_buffer
        );
        printf("Data received from [%s]\n", addr_buffer);
        run(msg_buffer);
        printf("Done treating command from [%s]\n", addr_buffer);

        bytes_write = sendto(
                sock,
                msg_buffer, sizeof msg_buffer - 1,
                0,
                (SOCKADDR *) &from, from_size
        );
        if (bytes_write < 0) {
            sock_err("Sending data");
        }
        printf("Data sent to [%s]\n", addr_buffer);
    }
}

void run(char *buffer) {
    char *command = strtok(buffer, " ");

    // >> create username password
    if (strcmpi(command, "create") == 0) {
        const char *username = strtok(NULL, " ");
        const char *password = strtok(NULL, " ");
        size_t id;
        int8_t res = user_database_create(
                username,
                strtoull(password, NULL, 10),
                &id
        );
        switch (res) {
            case USER_DATABASE_OPERATION_OK:
                sprintf(
                        buffer,
                        "User %s#%d created.",
                        username, id
                );
                break;
            case USER_DATABASE_ALREADY_EXISTS:
                sprintf(
                        buffer,
                        "User %s#%d already exists.",
                        username, id
                );
                break;
            case USER_DATABASE_INSERT_FAILED:
            case USER_DATABASE_TOO_MANY_USERS:
            default:
                sprintf(
                        buffer,
                        "Internal error."
                );
                break;
        }
    }

        // >> delete id password
    else if (strcmpi(command, "delete") == 0) {
        const char *id = strtok(NULL, " ");
        const char *password = strtok(NULL, " ");
        int8_t res = user_database_delete(
                strtoull(id, NULL, 10),
                strtoull(password, NULL, 10)
        );
        switch (res) {
            case USER_DATABASE_OPERATION_OK:
                sprintf(
                        buffer,
                        "User #%s deleted.",
                        id
                );
                break;
            case USER_DATABASE_INVALID_CREDENTIALS:
                sprintf(
                        buffer,
                        "Invalid credentials for user #%s.",
                        id
                );
                break;
            case USER_DATABASE_NOT_EXISTS:
                sprintf(
                        buffer,
                        "User #%s not found.",
                        id
                );
                break;
            default:
                sprintf(
                        buffer,
                        "Internal error."
                );
                break;
        }
    }

        // >> login id password
    else if (strcmpi(command, "login") == 0) {
        const char *id = strtok(NULL, " ");
        const char *password = strtok(NULL, " ");
        int8_t res = user_database_login(
                strtoull(id, NULL, 10),
                strtoull(password, NULL, 10)
        );
        switch (res) {
            case USER_DATABASE_OPERATION_OK:
                sprintf(
                        buffer,
                        "User #%s logged in.",
                        id
                );
                break;
            case USER_DATABASE_INVALID_CREDENTIALS:
                sprintf(
                        buffer,
                        "Invalid credentials for user #%s.",
                        id
                );
                break;
            case USER_DATABASE_ALREADY_CONNECTED:
                sprintf(
                        buffer,
                        "User #%s is already connected.",
                        id
                );
                break;
            default:
                sprintf(
                        buffer,
                        "Internal error."
                );
                break;
        }
    }

        // >> logout id password
    else if (strcmpi(command, "logout") == 0) {
        const char *id = strtok(NULL, " ");
        const char *password = strtok(NULL, " ");
        int8_t res = user_database_logout(
                strtoull(id, NULL, 10),
                strtoull(password, NULL, 10)
        );
        switch (res) {
            case USER_DATABASE_OPERATION_OK:
                sprintf(
                        buffer,
                        "User #%s logged in.",
                        id
                );
                break;
            case USER_DATABASE_INVALID_CREDENTIALS:
                sprintf(
                        buffer,
                        "Invalid credentials for user #%s.",
                        id
                );
                break;
            case USER_DATABASE_NOT_CONNECTED:
                sprintf(
                        buffer,
                        "User #%s is not connected.",
                        id
                );
                break;
            default:
                sprintf(
                        buffer,
                        "Internal error."
                );
                break;
        }
    }

        // >> password id old_password new_password
    else if (strcmpi(command, "password") == 0) {
        const char *id = strtok(NULL, " ");
        const char *old_pwd = strtok(NULL, " ");
        const char *new_pwd = strtok(NULL, " ");
        int8_t res = user_database_password(
                strtoull(id, NULL, 10),
                strtoull(old_pwd, NULL, 10),
                strtoull(new_pwd, NULL, 10)
        );
        switch (res) {
            case USER_DATABASE_OPERATION_OK:
                sprintf(
                        buffer,
                        "Password changed for user #%s.",
                        id
                );
                break;
            case USER_DATABASE_INVALID_CREDENTIALS:
                sprintf(
                        buffer,
                        "Invalid credentials for user #%s.",
                        id
                );
                break;
            case USER_DATABASE_NOT_EXISTS:
                sprintf(
                        buffer,
                        "User #%s does not exist.",
                        id
                );
                break;
            default:
                sprintf(
                        buffer,
                        "Internal error."
                );
                break;
        }
    }

        // >> list
    else if (strcmpi(command, "list") == 0) {
        sprintf(
                buffer,
                user_database_list()
        );
    }

        // >> Unknown command
    else {
        sprintf(buffer, "Unknown command: %s", command);
    }
}

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