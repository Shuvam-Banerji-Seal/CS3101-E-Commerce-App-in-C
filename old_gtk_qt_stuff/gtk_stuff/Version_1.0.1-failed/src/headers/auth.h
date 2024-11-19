#ifndef GTK4_ECOMMERCE_AUTH_H
#define GTK4_ECOMMERCE_AUTH_H

#include <stdbool.h>
#include <glib.h>
#include "database.h"

// auth.h improvements
typedef enum {
    AUTH_SUCCESS,
    AUTH_FAILED,
    AUTH_ACCOUNT_LOCKED,
    AUTH_USER_EXISTS,
    AUTH_WEAK_PASSWORD,
    AUTH_ERROR,
    AUTH_INVALID_INPUT,
    AUTH_DATABASE_ERROR,
    AUTH_PASSWORD_EXPIRED
} AuthResult;

typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevel;

typedef struct {
    const char *login_id;
    const char *password;
    UserRole role;
    const char *email;
    const char *phone;
    time_t password_expires;  // New field for password expiration
} RegistrationData;

/**
 * @brief Initialize authentication system
 * 
 * Sets up the authentication system including the login attempts tracking
 * and database initialization.
 * 
 * @return true if initialization successful, false otherwise
 */
bool init_auth(void);

/**
 * @brief Clean up authentication system resources
 * 
 * Frees all resources used by the authentication system including
 * login attempts tracking and database connections.
 */
void cleanup_auth(void);

/**
 * @brief Authenticate a user
 * 
 * Attempts to authenticate a user with the given credentials. Implements
 * account lockout after multiple failed attempts.
 * 
 * @param login_id The user's login identifier
 * @param password The user's password
 * @param user Pointer to User structure to store authenticated user data
 * @return Authentication result indicating success, failure, or error condition
 */
AuthResult authenticate_user(const char *login_id, const char *password, User *user);

/**
 * @brief Register a new user
 * 
 * Registers a new user in the system with the provided registration data.
 * Enforces password strength requirements and checks for existing users.
 * 
 * @param reg_data Pointer to RegistrationData structure containing user information
 * @return Registration result indicating success or specific failure reason
 */
AuthResult register_user(const RegistrationData *reg_data);

#endif // GTK4_AUTH_AUTH_H