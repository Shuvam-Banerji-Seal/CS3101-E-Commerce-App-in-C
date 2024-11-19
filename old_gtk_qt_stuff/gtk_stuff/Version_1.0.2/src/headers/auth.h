// #ifndef GTK4_AUTH_AUTH_H
// #define GTK4_AUTH_AUTH_H

// #include <stdbool.h>

// typedef enum {
//     AUTH_SUCCESS,
//     AUTH_FAILED,
//     AUTH_ACCOUNT_LOCKED,
//     AUTH_USER_EXISTS,
//     AUTH_WEAK_PASSWORD,
//     AUTH_ERROR
// } AuthResult;

// bool init_auth(void);
// void cleanup_auth(void);
// AuthResult authenticate_user(const char *username, const char *password);
// AuthResult register_user(const char *username, const char *password);

// #endif

// auth.h
// #ifndef AUTH_H
// #define AUTH_H

// #include <stdbool.h>
// #include <glib.h>

// typedef enum {
//     AUTH_SUCCESS,
//     AUTH_FAILED,
//     AUTH_ERROR,
//     AUTH_WEAK_PASSWORD,
//     AUTH_USER_EXISTS,
//     AUTH_ACCOUNT_LOCKED
// } AuthResult;

// bool init_auth(void);
// void cleanup_auth(void);
// AuthResult authenticate_user(const char *username, const char *password);
// AuthResult register_user(const char *username, const char *password);

// #endif // AUTH_H















// auth.h
#ifndef GTK4_AUTH_AUTH_H
#define GTK4_AUTH_AUTH_H

#include <stdbool.h>
#include <glib.h>

/**
 * @brief Authentication result codes
 */
typedef enum {
    AUTH_SUCCESS,        /**< Authentication successful */
    AUTH_FAILED,         /**< Authentication failed */
    AUTH_ACCOUNT_LOCKED, /**< Account is temporarily locked */
    AUTH_USER_EXISTS,    /**< Username already exists */
    AUTH_WEAK_PASSWORD,  /**< Password doesn't meet requirements */
    AUTH_ERROR          /**< System/internal error */
} AuthResult;

/**
 * @brief Initialize authentication system
 * @return true if initialization successful, false otherwise
 */
bool init_auth(void);

/**
 * @brief Clean up authentication system resources
 */
void cleanup_auth(void);

/**
 * @brief Authenticate a user
 * @param username The username to authenticate
 * @param password The password to verify
 * @return Authentication result
 */
AuthResult authenticate_user(const char *username, const char *password);

/**
 * @brief Register a new user
 * @param username The username to register
 * @param password The password to set
 * @return Registration result
 */
AuthResult register_user(const char *username, const char *password);

#endif // GTK4_AUTH_AUTH_H