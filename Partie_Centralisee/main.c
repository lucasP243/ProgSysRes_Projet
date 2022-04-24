#include <stdio.h>
#include <pthread.h>
#include "user_database_handler.h"
#include "server_handler.h"

int main() {

#ifdef WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (err == 0) {
        puts("WSAStartup failed.");
        exit(EXIT_FAILURE);
    }
#endif

    // TODO Manage closing signals

    user_database_open();

    pthread_t server_thread;
    pthread_create(
            &server_thread,
            NULL,
            &server_handler,
            NULL
    );


    // TODO Create thread or fork for sending messages process

    pthread_join(server_thread, NULL);
    user_database_close();

#ifdef WIN32
    if (WSACleanup() != 0) {
        puts("WSACleanup failed");
        exit(EXIT_FAILURE);
    }
#endif

    return 0;
}
