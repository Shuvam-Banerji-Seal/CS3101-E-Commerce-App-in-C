
// // database.c
// #include "headers/database.h"
// #include <stdio.h>
// #include <string.h>

// static sqlite3 *db = NULL;

// bool init_database(void) {
//     char *err_msg = NULL;
//     int rc = sqlite3_open("auth.db", &db);
    
//     if (rc != SQLITE_OK) {
//         return false;
//     }

//     const char *sql = 
//         "CREATE TABLE IF NOT EXISTS users ("
//         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
//         "username TEXT UNIQUE NOT NULL,"
//         "password TEXT NOT NULL);";

//     rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
//     if (rc != SQLITE_OK) {
//         sqlite3_free(err_msg);
//         return false;
//     }
    
//     return true;
// }

// bool create_user(const char *username, const char *password) {
//     sqlite3_stmt *stmt;
//     const char *sql = "INSERT INTO users (username, password) VALUES (?, ?);";
    
//     int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
//     if (rc != SQLITE_OK) {
//         return false;
//     }

//     sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
//     sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

//     rc = sqlite3_step(stmt);
//     sqlite3_finalize(stmt);
    
//     return rc == SQLITE_DONE;
// }

// bool verify_user(const char *username, const char *password) {
//     sqlite3_stmt *stmt;
//     const char *sql = "SELECT password FROM users WHERE username = ?;";
    
//     int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
//     if (rc != SQLITE_OK) {
//         return false;
//     }

//     sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
//     rc = sqlite3_step(stmt);
//     bool result = false;
    
//     if (rc == SQLITE_ROW) {
//         const char *stored_password = (const char *)sqlite3_column_text(stmt, 0);
//         result = strcmp(password, stored_password) == 0;
//     }
    
//     sqlite3_finalize(stmt);
//     return result;
// }

// void close_database(void) {
//     if (db) {
//         sqlite3_close(db);
//         db = NULL;
//     }
// }
#include "headers/database.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <crypt.h>

static sqlite3 *db = NULL;

bool init_database(void) {
    const char *db_file = "users.db";
    int rc = sqlite3_open(db_file, &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return false;
    }

    // Create users table if it doesn't exist
    const char *sql = "CREATE TABLE IF NOT EXISTS users ("
                     "username TEXT PRIMARY KEY,"
                     "password TEXT NOT NULL"
                     ");";

    char *err_msg = NULL;
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

void cleanup_database(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

bool create_user(const char *username, const char *hashed_password) {
    if (!db) return false;

    // Check if user already exists
    sqlite3_stmt *stmt;
    const char *sql = "SELECT username FROM users WHERE username = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false; // User already exists
    }
    sqlite3_finalize(stmt);

    // Insert new user
    sql = "INSERT INTO users (username, password) VALUES (?, ?);";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_STATIC);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool verify_user(const char *username, const char *password) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT password FROM users WHERE username = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    const char *stored_hash = (const char *)sqlite3_column_text(stmt, 0);
    char *computed_hash = crypt(password, stored_hash);
    bool matches = strcmp(computed_hash, stored_hash) == 0;
    
    sqlite3_finalize(stmt);
    return matches;
}
