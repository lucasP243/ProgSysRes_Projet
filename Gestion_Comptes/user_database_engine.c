#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "user_database_engine.h"

const size_t DEFAULT_SIZE = 10000;

struct userinfo {
    char username[10];
    uint64_t hash;
    size_t id;
    uint8_t flags;
};

#define USERINFO_FLAG_ONLINE 0

int8_t user_database_insert(struct userinfo *user);

int8_t user_database_check_hash(size_t id, uint64_t hash);

size_t user_database_next_id();

size_t user_database_size = DEFAULT_SIZE;

struct userinfo **user_database = NULL;

int8_t user_database_init() {

    user_database = calloc(user_database_size, sizeof(void *));

    if (user_database == NULL) {
        fprintf(
                USER_DATABASE_ERR_STREAM,
                "Failed to initialize database.\n"
        );
        return USER_DATABASE_INIT_FAILED;
    }

    FILE *database_read, *database_write;

    /*
     * If the persistent database file doesn't exist, read from the backup and
     * write to data. Otherwise, read from data and write to the backup.
     */
    if (access(USER_DATABASE_PATH, F_OK) < 0) {

        database_read = fopen(USER_DATABASE_BACKUP_PATH, "rb");

        if (database_read == NULL) {
            return USER_DATABASE_OPERATION_OK; // No persistent data
        }

        database_write = fopen(USER_DATABASE_PATH, "wb");

        if (database_write == NULL) {
            fprintf(
                    USER_DATABASE_ERR_STREAM,
                    "Failed to create database persistent file.\n"
            );
            return USER_DATABASE_INIT_FAILED;
        }
    } else {

        database_read = fopen(USER_DATABASE_PATH, "rb");

        if (database_read == NULL) {
            fprintf(
                    USER_DATABASE_ERR_STREAM,
                    "Failed to open database persistent file.\n"
            );
            return USER_DATABASE_INIT_FAILED;
        }

        database_write = fopen(USER_DATABASE_BACKUP_PATH, "wb");

        if (database_write == NULL) {
            fprintf(
                    USER_DATABASE_ERR_STREAM,
                    "Failed to create database backup file.\n"
            );
            return USER_DATABASE_INIT_FAILED;
        }
    }

    struct userinfo *user = NULL;
    while (!feof(database_read)) {
        fread(user, sizeof *user, 1, database_read);
        user_database_insert(user);
        fwrite(user, sizeof *user, 1, database_write);
    }

    fclose(database_read);
    fclose(database_write);

    return USER_DATABASE_OPERATION_OK;
}

int8_t user_database_close() {

    FILE *database_persistent = fopen(USER_DATABASE_PATH, "wb");

    if (database_persistent == NULL) {
        fprintf(
                USER_DATABASE_ERR_STREAM,
                "Failed to create database persistent file.\n"
        );
        return USER_DATABASE_CLOSE_FAILED;
    }

    for (size_t i = 0; i < user_database_size; i++) {

        struct userinfo *user = user_database[i];

        if (user == NULL) continue;

        // Set user offline
        user->flags &= ~(1UL << USERINFO_FLAG_ONLINE);

        fwrite(user_database[i], sizeof(*user), 1, database_persistent);

        free(user);
        user_database[i] = NULL;
    }

    free(user_database);
    user_database = NULL;

    return USER_DATABASE_OPERATION_OK;
}

int8_t user_database_create(const char *username, uint64_t hash, size_t *id) {

    *id = user_database_next_id();

    if (*id == 0) return USER_DATABASE_TOO_MANY_USERS;

    struct userinfo user = {
            .username = *username,
            .hash = hash,
            .id = *id,
            .flags = 0
    };

    return user_database_insert(&user);
}

int8_t user_database_delete(size_t id, uint64_t hash) {

    int8_t check = user_database_check_hash(id, hash);
    if (check < 0) return check;

    free(user_database[id]);
    user_database[id] = NULL;

    return USER_DATABASE_OPERATION_OK;
}

int8_t user_database_login(size_t id, uint64_t hash) {

    int8_t check = user_database_check_hash(id, hash);
    if (check < 0) return check;

    struct userinfo *user = user_database[id];

    if (((user->flags >> USERINFO_FLAG_ONLINE) & 1UL) == 1UL) {
        return USER_DATABASE_ALREADY_CONNECTED;
    }

    user->flags |= (1UL << USERINFO_FLAG_ONLINE);

    return USER_DATABASE_OPERATION_OK;
}

int8_t user_database_logout(size_t id, uint64_t hash) {

    int8_t check = user_database_check_hash(id, hash);
    if (check < 0) return check;

    struct userinfo *user = user_database[id];

    if (((user->flags >> USERINFO_FLAG_ONLINE) & 1UL) == 0UL) {
        return USER_DATABASE_NOT_CONNECTED;
    }

    user->flags &= ~(1UL << USERINFO_FLAG_ONLINE);

    return USER_DATABASE_OPERATION_OK;
}

int8_t user_database_password(size_t id, uint64_t old_hash, uint64_t new_hash) {

    int8_t check = user_database_check_hash(id, old_hash);
    if (check < 0) return check;

    user_database[id]->hash = new_hash;

    return USER_DATABASE_OPERATION_OK;
}

int8_t user_database_insert(struct userinfo *user) {

    if (user == NULL) {
        return USER_DATABASE_INSERT_FAILED;
    }

    if (user->id >= user_database_size) {
        return USER_DATABASE_TOO_MANY_USERS;
    }

    if (user_database[user->id] != NULL) {
        return USER_DATABASE_ALREADY_EXISTS;
    }

    // Set user offline
    user->flags &= ~(1UL << USERINFO_FLAG_ONLINE);

    user_database[user->id] = user;

    return USER_DATABASE_OPERATION_OK;
}

int8_t user_database_check_hash(size_t id, uint64_t hash) {

    if (id >= user_database_size || user_database[id] == NULL) {
        return USER_DATABASE_NOT_EXISTS;
    }

    return (user_database[id]->hash == hash)
           ? USER_DATABASE_OPERATION_OK
           : USER_DATABASE_INVALID_CREDENTIALS;
}

size_t user_database_next_id() {
    for (size_t i = 1; i < user_database_size; i++) {
        if (user_database[i] == NULL) return i;
    }
    return 0;
}

// TODO add thread-safety