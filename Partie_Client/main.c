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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TOKEN_DELIMITER " "

#define SERVER_ADDR "localhost"
#define SERVER_PORT 24020

SOCKET client_socket = {0};

unsigned long hash(const char *str);

int close();

void sock_err(char *action);

int main(void) {

#ifdef WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (err != 0) {
        sock_err("WSAStartup");
    }
#endif

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        sock_err("Creating client socket");
    }

    struct hostent *host_info = gethostbyname(SERVER_ADDR);
    if (host_info == NULL) {
        fprintf(stderr, "Unknown host %d\n", SERVER_ADDR);
        return EXIT_FAILURE;
    }

    SOCKADDR_IN server_addr = {
            .sin_addr = *(IN_ADDR *) host_info->h_addr,
            .sin_port = htons(SERVER_PORT),
            .sin_family = AF_INET
    };

    int res = connect(
            client_socket,
            (SOCKADDR *) &server_addr,
            sizeof(SOCKADDR)
    );
    if (res == SOCKET_ERROR) {
        sock_err("Connecting socket");
    }

    char buffer[1024];
    int n = 0;

    while (1) {
        printf(">> ");
        gets(buffer);

        char *cmd = strtok(buffer, TOKEN_DELIMITER);
        if (strcmp(cmd, "help") == 0) {
            puts(
                    "List of commands :\n"
                    "register <username> <password> : register as a new user\n"
                    "delete <id> <password> : delete your account\n"
                    "login <id> <password> : log in to the server\n"
                    "logout <id> <password> : log out of the server\n"
                    "passwd <id> <old password> <new password> : change your password"
            );
            continue;

        } else if (strcmp(cmd, "register") == 0) {
            const char *username = strtok(NULL, TOKEN_DELIMITER);
            const char *password = strtok(NULL, TOKEN_DELIMITER);
            sprintf(buffer, "create %s %lu", username, hash(password));

        } else if (strcmp(cmd, "delete") == 0) {
            const char *id = strtok(NULL, TOKEN_DELIMITER);
            const char *password = strtok(NULL, TOKEN_DELIMITER);
            sprintf(buffer, "delete %s %lu", id, hash(password));

        } else if (strcmp(cmd, "login") == 0) {
            const char *id = strtok(NULL, TOKEN_DELIMITER);
            const char *password = strtok(NULL, TOKEN_DELIMITER);
            sprintf(buffer, "login %s %lu", id, hash(password));

        } else if (strcmp(cmd, "logout") == 0) {
            const char *id = strtok(NULL, TOKEN_DELIMITER);
            const char *password = strtok(NULL, TOKEN_DELIMITER);
            sprintf(buffer, "logout %s %lu", id, hash(password));

        } else if (strcmp(cmd, "passwd") == 0) {
            const char *id = strtok(NULL, TOKEN_DELIMITER);
            const char *old_pass = strtok(NULL, TOKEN_DELIMITER);
            const char *new_pass = strtok(NULL, TOKEN_DELIMITER);
            sprintf(buffer, "password %s %lu %lu", id, hash(old_pass), hash(new_pass));

        } else if (strcmp(cmd, "exit") == 0) {
            return close();

        } else {
            puts("Unknown command");
            continue;
        }

        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            sock_err("Sending request");
        }

        if ((n = recv(client_socket, buffer, sizeof buffer - 1, 0)) < 0) {
            sock_err("Acquiring server response");
        }
        buffer[n] = '\0';
        puts(buffer);
    }
}

int close() {
    closesocket(client_socket);

#ifdef WIN32
    if (WSACleanup() != 0) {
        puts("WSACleanup failed");
        return EXIT_FAILURE;
    }
#endif
    return EXIT_SUCCESS;
}

/* -------------------------------------------------------------------------- */

unsigned long hash(const char *str) {
    // djb2 algorithm, referenced in http://www.cse.yorku.ca/~oz/hash.html
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
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

