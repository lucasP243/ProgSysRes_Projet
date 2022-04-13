#ifndef USER_DATABASE_ENGINE_H
#define USER_DATABASE_ENGINE_H

#include <stdint.h>

/**
 * Path to the persistent database.
 */
#define USER_DATABASE_PATH "./users.dat"

/**
 * Path to the backup persistent database.
 */
#define USER_DATABASE_BACKUP_PATH "./users.dat.bak"

/**
 * Application standard output stream.
 */
#define USER_DATABASE_OUT_STREAM stdout

/**
 * Application error output stream.
 */
#define USER_DATABASE_ERR_STREAM stderr


/** Operation successful. */
#define USER_DATABASE_OPERATION_OK 0

/** Operation failed : invalid username or password */
#define USER_DATABASE_INVALID_CREDENTIALS (-1)

/** Operation failed : user already exists */
#define USER_DATABASE_ALREADY_EXISTS (-2)

/** Operation failed : user does not exist */
#define USER_DATABASE_NOT_EXISTS (-3)

/** Operation failed : user already connected */
#define USER_DATABASE_ALREADY_CONNECTED (-4)

/** Operation failed : user not connected */
#define USER_DATABASE_NOT_CONNECTED (-5)

/**
 * Initializes the database and loads the persistent data.
 */
extern void user_database_init();

/**
 * Closes the database and saves to the persistent data.
 */
extern void user_database_close();

/**
 * Attempts to log in an user.
 *
 * @param username name of the user
 * @param hash produced by the user's password
 *
 * @return USER_DATABASE_OPERATION_OK
 *         <hr>
 *         USER_DATABASE_INVALID_CREDENTIALS<br>
 *         USER_DATABASE_ALREADY_CONNECTED
 */
extern uint8_t user_database_login(
        char *username,
        uint32_t hash
);

/**
 * Attempts to log out an user.
 *
 * @param username name of the user
 * @param hash produced by the user's password
 *
 * @return USER_DATABASE_OPERATION_OK
 *         <hr>
 *         USER_DATABASE_INVALID_CREDENTIALS<br>
 *         USER_DATABASE_NOT_CONNECTED
 */
extern uint8_t user_database_logout(
        char *username
);

/**
 * Attempts to create a new user.
 *
 * @param username name of the user
 * @param hash produced by the user's password
 *
 * @return USER_DATABASE_OPERATION_OK
 *         <hr>
 *         USER_DATABASE_ALREADY_EXISTS
 */
extern uint8_t user_database_create(
        char *username,
        uint32_t hash
);

/**
 * Attempts to delete an user.
 *
 * @param username name of the user
 * @param hash produced by the user's password
 *
 * @return USER_DATABASE_OPERATION_OK
 *         <hr>
 *         USER_DATABASE_INVALID_CREDENTIALS<br>
 *         USER_DATABASE_NOT_EXISTS
 */
extern uint8_t user_database_delete(
        char *username,
        uint32_t hash
);

/**
 * Attempts to delete an user.
 *
 * @param username name of the user
 * @param old_hash produced by the user's old password
 * @param new_hash produced by the user's new password
 *
 * @return USER_DATABASE_OPERATION_OK
 *         <hr>
 *         USER_DATABASE_INVALID_CREDENTIALS<br>
 *         USER_DATABASE_NOT_EXISTS
 */
extern uint8_t user_database_password(
        char *username,
        uint32_t old_hash,
        uint32_t new_hash
);

/**
 * Gets the list of online users.
 *
 * @return The usernames of online users, CSV-style
 */
extern char* user_database_list();

#endif
