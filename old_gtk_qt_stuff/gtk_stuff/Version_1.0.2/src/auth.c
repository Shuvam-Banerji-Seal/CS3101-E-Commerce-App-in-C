#include "headers/auth.h"
#include "headers/database.h"
#include <crypt.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define SALT_LENGTH 16
#define MIN_PASSWORD_LENGTH 8
#define MAX_LOGIN_ATTEMPTS 3
#define LOCKOUT_DURATION 300 // 5 minutes in seconds

typedef struct {
    char *username;
    int failed_attempts;
    time_t lockout_time;
} LoginAttempt;

static GHashTable *login_attempts = NULL;

static char *generate_salt(void) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    char *salt = g_malloc(SALT_LENGTH + 1);
    
    for (int i = 0; i < SALT_LENGTH; i++) {
        salt[i] = charset[g_random_int_range(0, sizeof(charset) - 1)];
    }
    salt[SALT_LENGTH] = '\0';
    
    return salt;
}

bool init_auth(void) {
    login_attempts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    return login_attempts != NULL;
}

void cleanup_auth(void) {
    if (login_attempts) {
        g_hash_table_destroy(login_attempts);
        login_attempts = NULL;
    }
}

static bool is_password_strong(const char *password) {
    if (strlen(password) < MIN_PASSWORD_LENGTH) return false;
    
    bool has_upper = false, has_lower = false, has_digit = false, has_special = false;
    
    for (const char *p = password; *p; p++) {
        if (isupper(*p)) has_upper = true;
        else if (islower(*p)) has_lower = true;
        else if (isdigit(*p)) has_digit = true;
        else has_special = true;
    }
    
    return has_upper && has_lower && has_digit && has_special;
}

static bool is_account_locked(const char *username) {
    LoginAttempt *attempt = g_hash_table_lookup(login_attempts, username);
    if (!attempt) return false;
    
    if (attempt->failed_attempts >= MAX_LOGIN_ATTEMPTS) {
        time_t current_time = time(NULL);
        if (current_time - attempt->lockout_time < LOCKOUT_DURATION) {
            return true;
        } else {
            // Reset attempts after lockout period
            attempt->failed_attempts = 0;
            attempt->lockout_time = 0;
        }
    }
    return false;
}

AuthResult authenticate_user(const char *username, const char *password) {
    if (is_account_locked(username)) {
        return AUTH_ACCOUNT_LOCKED;
    }

    if (!verify_user(username, password)) {
        LoginAttempt *attempt = g_hash_table_lookup(login_attempts, username);
        if (!attempt) {
            attempt = g_new0(LoginAttempt, 1);
            attempt->username = g_strdup(username);
            g_hash_table_insert(login_attempts, g_strdup(username), attempt);
        }
        
        attempt->failed_attempts++;
        if (attempt->failed_attempts >= MAX_LOGIN_ATTEMPTS) {
            attempt->lockout_time = time(NULL);
            return AUTH_ACCOUNT_LOCKED;
        }
        return AUTH_FAILED;
    }

    // Reset failed attempts on successful login
    g_hash_table_remove(login_attempts, username);
    return AUTH_SUCCESS;
}

AuthResult register_user(const char *username, const char *password) {
    if (!is_password_strong(password)) {
        return AUTH_WEAK_PASSWORD;
    }

    char *salt = generate_salt();
    char *salted_password = g_strdup_printf("$6$%s", salt);
    char *hashed_password = crypt(password, salted_password);
    g_free(salted_password);
    g_free(salt);

    if (!hashed_password) {
        return AUTH_ERROR;
    }

    if (!create_user(username, hashed_password)) {
        return AUTH_USER_EXISTS;
    }

    return AUTH_SUCCESS;
}