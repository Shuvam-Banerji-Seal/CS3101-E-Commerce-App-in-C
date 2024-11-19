// // database.h
// #ifndef GTK4_AUTH_DATABASE_H
// #define GTK4_AUTH_DATABASE_H

// #include <sqlite3.h>
// #include <stdbool.h>

// bool init_database(void);
// bool create_user(const char *username, const char *password);
// bool verify_user(const char *username, const char *password);
// void close_database(void);

// #endif


// database.h
// #ifndef DATABASE_H
// #define DATABASE_H

// #include <sqlite3.h>
// #include <stdbool.h>

// bool init_database(void);
// bool create_user(const char *username, const char *password);
// bool verify_user(const char *username, const char *password);
// void close_database(void);

// #endif // DATABASE_H



// database.h
// database.h
// #ifndef GTK4_AUTH_DATABASE_H
// #define GTK4_AUTH_DATABASE_H

// #include <stdbool.h>
// #include <stdbool.h>

// bool init_database(void);
// void cleanup_database(void);
// bool create_user(const char *username, const char *hashed_password);
// bool verify_user(const char *username, const char *password);
// void close_database(void);
// #endif // GTK4_AUTH_DATABASE_H










// database.h
#ifndef GTK4_AUTH_DATABASE_H
#define GTK4_AUTH_DATABASE_H

#include <sqlite3.h>
#include <stdbool.h>

/**
 * @brief Initialize the database connection
 * @return true if initialization successful, false otherwise
 */
bool init_database(void);

/**
 * @brief Create a new user in the database
 * @param username Username to create
 * @param password Hashed password to store
 * @return true if user created successfully, false otherwise
 */





void cleanup_database(void);



bool create_user(const char *username, const char *password);

/**
 * @brief Verify user credentials
 * @param username Username to verify
 * @param password Password to check
 * @return true if credentials are valid, false otherwise
 */
bool verify_user(const char *username, const char *password);

/**
 * @brief Close the database connection
 */
void close_database(void);

#endif // GTK4_AUTH_DATABASE_H
