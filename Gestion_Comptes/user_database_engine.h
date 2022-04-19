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

/** Operation failed : database is full */
#define USER_DATABASE_TOO_MANY_USERS (-5)

/** Server error : failed to initialize database */
#define USER_DATABASE_INIT_FAILED (-11)

/** Server error : failed to close database */
#define USER_DATABASE_CLOSE_FAILED (-12)

/** Server error : failed to insert user */
#define USER_DATABASE_INSERT_FAILED (-13)

/**
 * Initializes the database and loads the persistent data.
 */
extern int8_t user_database_init();

/**
 * Closes the database and saves to the persistent data.
 */
extern int8_t user_database_close();

/**
 * Attempts to create a new user.
 *
 * @param username name of the user
 * @param hash produced by the user's password
 * @param id the id of the newly created user
 *
 * @return USER_DATABASE_OPERATION_OK
 *         <hr>
 *         USER_DATABASE_INSERT_FAILED<br>
 *         USER_DATABASE_ALREADY_EXISTS<br>
 *         USER_DATABASE_TOO_MANY_USERS
 */
extern int8_t user_database_create(
        const char *username,
        uint64_t hash,
        size_t *id
);

/**
 * Attempts to delete an user.
 *
 * @param id id of the user
 * @param hash produced by the user's password
 *
 * @return USER_DATABASE_OPERATION_OK
 *         <hr>
 *         USER_DATABASE_INVALID_CREDENTIALS<br>
 *         USER_DATABASE_NOT_EXISTS
 */
extern int8_t user_database_delete(
        size_t id,
        uint64_t hash
);

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
extern int8_t user_database_login(
        size_t id,
        uint64_t hash
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
extern int8_t user_database_logout(
        size_t id,
        uint64_t hash
);

/**
 * Attempts to change an user's password.
 *
 * @param id id of the user
 * @param old_hash produced by the user's old password
 * @param new_hash produced by the user's new password
 *
 * @return USER_DATABASE_OPERATION_OK
 *         <hr>
 *         USER_DATABASE_INVALID_CREDENTIALS<br>
 *         USER_DATABASE_NOT_EXISTS
 */
extern int8_t user_database_password(
        size_t id,
        uint64_t old_hash,
        uint64_t new_hash
);

/**
 * Gets the list of online users.
 *
 * @return The usernames of online users, CSV-style
 */
extern char* user_database_list();

#endif
