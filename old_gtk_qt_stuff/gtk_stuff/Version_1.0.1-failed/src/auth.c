#include "auth.h"
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <crypt.h>
#include <glib.h>

// Constants for security settings
#define MAX_LOGIN_ATTEMPTS 5
#define LOCKOUT_DURATION_SECONDS 300  // 5 minutes
#define MIN_PASSWORD_LENGTH 8
#define PASSWORD_EXPIRY_DAYS 90

// Structure to track login attempts
typedef struct {
    time_t timestamp;
    int attempts;
    time_t lockout_until;
} LoginAttempts;

// Hash table to store login attempts
static GHashTable *login_attempts = NULL;

// Internal function prototypes
static bool is_password_strong(const char *password);
static char *generate_password_hash(const char *password);
static void cleanup_login_attempts(gpointer key, gpointer value, gpointer user_data);
static bool is_account_locked(const char *login_id);
static void update_login_attempts(const char *login_id, bool success);
static void log_auth_event(LogLevel level, const char *message, const char *login_id);

bool init_auth(void) {
    // Initialize the login attempts tracking
    if (login_attempts != NULL) {
        g_hash_table_destroy(login_attempts);
    }
    
    login_attempts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    if (login_attempts == NULL) {
        return false;
    }
    
    // Initialize the database
    if (!init_database()) {
        g_hash_table_destroy(login_attempts);
        login_attempts = NULL;
        return false;
    }
    
    return true;
}

void cleanup_auth(void) {
    if (login_attempts != NULL) {
        g_hash_table_foreach(login_attempts, cleanup_login_attempts, NULL);
        g_hash_table_destroy(login_attempts);
        login_attempts = NULL;
    }
    
    cleanup_database();
}

AuthResult authenticate_user(const char *login_id, const char *password, User *user) {
    if (login_id == NULL || password == NULL || user == NULL) {
        log_auth_event(LOG_LEVEL_ERROR, "Invalid authentication parameters", login_id);
        return AUTH_INVALID_INPUT;
    }
    
    // Check if account is locked
    if (is_account_locked(login_id)) {
        log_auth_event(LOG_LEVEL_WARN, "Account locked due to too many failed attempts", login_id);
        return AUTH_ACCOUNT_LOCKED;
    }
    
    // Attempt to get user from database
    if (!get_user(login_id, user)) {
        update_login_attempts(login_id, false);
        log_auth_event(LOG_LEVEL_WARN, "Authentication failed - user not found", login_id);
        return AUTH_FAILED;
    }
    
    // Verify password
    if (!verify_user(login_id, password)) {
        update_login_attempts(login_id, false);
        log_auth_event(LOG_LEVEL_WARN, "Authentication failed - invalid password", login_id);
        return AUTH_FAILED;
    }
    
    // Check password expiration
    time_t now = time(NULL);
    if (user->created_at + (PASSWORD_EXPIRY_DAYS * 24 * 60 * 60) < now) {
        log_auth_event(LOG_LEVEL_WARN, "Password expired", login_id);
        return AUTH_PASSWORD_EXPIRED;
    }
    
    // Update last login time
    if (!update_last_login(user->user_id)) {
        log_auth_event(LOG_LEVEL_ERROR, "Failed to update last login time", login_id);
        return AUTH_DATABASE_ERROR;
    }
    
    // Clear failed login attempts on successful login
    update_login_attempts(login_id, true);
    log_auth_event(LOG_LEVEL_INFO, "Authentication successful", login_id);
    
    return AUTH_SUCCESS;
}

AuthResult register_user(const RegistrationData *reg_data) {
    if (reg_data == NULL || reg_data->login_id == NULL || reg_data->password == NULL) {
        log_auth_event(LOG_LEVEL_ERROR, "Invalid registration data", NULL);
        return AUTH_INVALID_INPUT;
    }
    
    // Check if user already exists
    User existing_user;
    if (get_user(reg_data->login_id, &existing_user)) {
        log_auth_event(LOG_LEVEL_WARN, "Registration failed - user already exists", reg_data->login_id);
        return AUTH_USER_EXISTS;
    }
    
    // Validate password strength
    if (!is_password_strong(reg_data->password)) {
        log_auth_event(LOG_LEVEL_WARN, "Registration failed - weak password", reg_data->login_id);
        return AUTH_WEAK_PASSWORD;
    }
    
    // Create password hash
    char *password_hash = generate_password_hash(reg_data->password);
    if (password_hash == NULL) {
        log_auth_event(LOG_LEVEL_ERROR, "Failed to generate password hash", reg_data->login_id);
        return AUTH_ERROR;
    }
    
    // Prepare user data
    User new_user = {
        .login_id = strdup(reg_data->login_id),
        .password_hash = password_hash,
        .role = reg_data->role,
        .email = reg_data->email ? strdup(reg_data->email) : NULL,
        .phone = reg_data->phone ? strdup(reg_data->phone) : NULL,
        .created_at = time(NULL),
        .last_login = 0
    };
    
    // Create user in database
    if (!create_user(&new_user)) {
        free((void*)new_user.login_id);
        free(password_hash);
        free((void*)new_user.email);
        free((void*)new_user.phone);
        log_auth_event(LOG_LEVEL_ERROR, "Failed to create user in database", reg_data->login_id);
        return AUTH_DATABASE_ERROR;
    }
    
    log_auth_event(LOG_LEVEL_INFO, "User registration successful", reg_data->login_id);
    return AUTH_SUCCESS;
}

// Internal helper functions
static bool is_password_strong(const char *password) {
    if (strlen(password) < MIN_PASSWORD_LENGTH) {
        return false;
    }
    
    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;
    bool has_special = false;
    
    for (const char *p = password; *p; p++) {
        if (isupper(*p)) has_upper = true;
        else if (islower(*p)) has_lower = true;
        else if (isdigit(*p)) has_digit = true;
        else has_special = true;
    }
    
    return has_upper && has_lower && has_digit && has_special;
}

static char *generate_password_hash(const char *password) {
    char salt[32];
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    
    // Generate random salt
    salt[0] = '$';
    salt[1] = '6';  // Use SHA-512
    salt[2] = '$';
    
    for (int i = 3; i < 19; i++) {
        salt[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    salt[19] = '$';
    salt[20] = '\0';
    
    // Generate hash using crypt()
    return strdup(crypt(password, salt));
}

static void cleanup_login_attempts(gpointer key, gpointer value, gpointer user_data) {
    (void)key;      // Unused parameter
    (void)user_data;// Unused parameter
    free(value);
}

static bool is_account_locked(const char *login_id) {
    LoginAttempts *attempts = g_hash_table_lookup(login_attempts, login_id);
    if (attempts == NULL) {
        return false;
    }
    
    time_t now = time(NULL);
    if (attempts->lockout_until > now) {
        return true;
    }
    
    // Clear expired lockout
    if (attempts->lockout_until != 0 && attempts->lockout_until <= now) {
        attempts->attempts = 0;
        attempts->lockout_until = 0;
    }
    
    return false;
}

static void update_login_attempts(const char *login_id, bool success) {
    LoginAttempts *attempts = g_hash_table_lookup(login_attempts, login_id);
    time_t now = time(NULL);
    
    if (attempts == NULL) {
        attempts = g_new0(LoginAttempts, 1);
        g_hash_table_insert(login_attempts, g_strdup(login_id), attempts);
    }
    
    if (success) {
        attempts->attempts = 0;
        attempts->lockout_until = 0;
    } else {
        attempts->attempts++;
        attempts->timestamp = now;
        
        if (attempts->attempts >= MAX_LOGIN_ATTEMPTS) {
            attempts->lockout_until = now + LOCKOUT_DURATION_SECONDS;
        }
    }
}

static void log_auth_event(LogLevel level, const char *message, const char *login_id) {
    const char *level_str;
    switch (level) {
        case LOG_LEVEL_ERROR:
            level_str = "ERROR";
            break;
        case LOG_LEVEL_WARN:
            level_str = "WARN";
            break;
        case LOG_LEVEL_INFO:
            level_str = "INFO";
            break;
        case LOG_LEVEL_DEBUG:
            level_str = "DEBUG";
            break;
        default:
            level_str = "UNKNOWN";
    }
    
    time_t now = time(NULL);
    char timestamp[26];
    ctime_r(&now, timestamp);
    timestamp[24] = '\0';  // Remove newline
    
    fprintf(stderr, "[%s] %s: %s%s%s\n",
            timestamp,
            level_str,
            message,
            login_id ? " (User: " : "",
            login_id ? login_id : "");
}