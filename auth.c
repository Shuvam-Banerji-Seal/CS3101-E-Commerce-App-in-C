
bool user_signup(
    const char *username, 
    const char *hashed_password, 
    const char *email, 
    const char *phone
) {
    if (!db) 
    {
        printf("Database is not initialized.\n");
        return false;    
    }


    sqlite3_stmt *stmt;
    const char *check_sql = "SELECT username FROM users WHERE username = ?;";

    // Check if the username already exists
    if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Username exists
        fprintf(stderr, "User already exists: %s\n", username);
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    // Insert the new user
    const char *insert_sql = 
        "INSERT INTO users (username, password_hash, email, phone) "
        "VALUES (?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing insert statement: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, phone, -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) {
        fprintf(stderr, "Error inserting new user: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return success;
}

//checking the admin
bool verify_admin(const char *username, const char *password) {
    if (!db) {
        printf("Database is not initialized.\n");
        return false;
    }

    sqlite3_stmt *stmt;
    // Let's first verify the user exists and check their stored password.. hopefully there is only one
    const char *sql = "SELECT admin_password_hash FROM admins WHERE admin_name = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return false;
    }

    // Debug: Print the username we're looking for... Remove later
    //printf("DEBUG: Searching for username: '%s'\n", username);

    
    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Error binding parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    // Debug: Print the SQL statement
    //printf("DEBUG: SQL statement: %s\n", sql);

    // Execute the query and get the result
    int step_result = sqlite3_step(stmt);
    
    // Debug: Print step result
   // printf("DEBUG: Step result: %d (SQLITE_ROW=%d)\n", step_result, SQLITE_ROW);

    if (step_result != SQLITE_ROW) {
      //  printf("DEBUG: No user found or error occurred\n");
        sqlite3_finalize(stmt);
        return false;
    }

    // Get the stored password hash
    const unsigned char *stored_password = sqlite3_column_text(stmt, 0);
    if (!stored_password) {
       // printf("DEBUG: Stored password is NULL\n");
        sqlite3_finalize(stmt);
        return false;
    }

    // Debug: Print the passwords being compared
    // need to add more debug because nothing seems to work
    // printf("DEBUG: Comparing passwords:\n");
    // printf("DEBUG: Input password: '%s'\n", password);
    // printf("DEBUG: Stored password: '%s'\n", stored_password);

    // check those paaaasssswords
    bool matches = (strcmp(password, (const char *)stored_password) == 0);
   // printf("DEBUG: Password match result: %d\n", matches);
    
    // Cleanning
    sqlite3_finalize(stmt);
    return matches;
}


bool verify_user(const char *username, const char *password) {
    if (!db) {
        printf("Database is not initialized.\n");
        return false;
    }

    sqlite3_stmt *stmt;
    // Let's first verify the user exists and check their stored password
    const char *sql = "SELECT password_hash FROM users WHERE username = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return false;
    }

    // Debug: Print the username we're looking for
    //printf("DEBUG: Searching for username: '%s'\n", username);

    // Bind the username parameter
    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Error binding parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    // Debug: Print the SQL statement
   // printf("DEBUG: SQL statement: %s\n", sql);

    // Execute the query and get the result
    int step_result = sqlite3_step(stmt);
    
    // Debug: Print step result
   // printf("DEBUG: Step result: %d (SQLITE_ROW=%d)\n", step_result, SQLITE_ROW);

    if (step_result != SQLITE_ROW) {
        //printf("DEBUG: No user found or error occurred\n");
        sqlite3_finalize(stmt);
        return false;
    }

    // Get the stored password hash
    const unsigned char *stored_password = sqlite3_column_text(stmt, 0);
    if (!stored_password) {
     //   printf("DEBUG: Stored password is NULL\n");
        sqlite3_finalize(stmt);
        return false;
    }

    // Debug: Print the passwords being compared
    // need to add more debug because nothing seems to work
    // printf("DEBUG: Comparing passwords:\n");
    // printf("DEBUG: Input password: '%s'\n", password);
    // printf("DEBUG: Stored password: '%s'\n", stored_password);

    // check those paaaasssswords
    bool matches = (strcmp(password, (const char *)stored_password) == 0);
    //printf("DEBUG: Password match result: %d\n", matches);
    
    // Cleanning
    sqlite3_finalize(stmt);
    return matches;
}



bool get_user(const char *username, User *user) {
    if (!db || !user) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM users WHERE username = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    user->user_id = sqlite3_column_int(stmt, 0);
    user->username = strdup((const char *)sqlite3_column_text(stmt, 1));
    user->password_hash = strdup((const char *)sqlite3_column_text(stmt, 2));
    user->role = sqlite3_column_int(stmt, 3);
    user->email = sqlite3_column_text(stmt, 4) ? strdup((const char *)sqlite3_column_text(stmt, 4)) : NULL;
    user->phone = sqlite3_column_text(stmt, 5) ? strdup((const char *)sqlite3_column_text(stmt, 5)) : NULL;
    user->address_default = sqlite3_column_int(stmt, 6);
    user->created_at = sqlite3_column_int64(stmt, 7);
    user->last_login = sqlite3_column_int64(stmt, 8);

    sqlite3_finalize(stmt);
    return true;
}

bool update_last_login(int user_id) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE user_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}



// Helper function to check if username exists
bool check_username_exists(sqlite3 *db, const char *username, int exclude_user_id) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM users WHERE username = ? AND user_id != ?";
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, exclude_user_id);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return count > 0;
}

// Helper function to check if email exists
bool check_email_exists(sqlite3 *db, const char *email, int exclude_user_id) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM users WHERE email = ? AND user_id != ?";
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, exclude_user_id);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return count > 0;
}

// Modify user function (only by admin)... Soumya add some auth checker in it
int admin_modify_user(sqlite3 *db, const User *modified_user) {
    if (check_username_exists(db, modified_user->username, modified_user->user_id) ||
        check_email_exists(db, modified_user->email, modified_user->user_id)) {
        return SQLITE_CONSTRAINT;
    }

    const char *sql = "UPDATE users SET username = COALESCE(?,username), password_hash =COALESCE(?,password_hash), "      // should be added a coalesce statement( does not cheange anything if your input is null)
                     "email = COALESCE(?,email), phone =COALESCE(?,phone), address_default =COALESCE(?,address_default), last_login = ? "
                     "WHERE user_id = ?";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, modified_user->username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, modified_user->password_hash, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, modified_user->email, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, modified_user->phone, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, modified_user->address_default);
        sqlite3_bind_int64(stmt, 6, (sqlite3_int64)modified_user->last_login);
        sqlite3_bind_int(stmt, 7, modified_user->user_id);
        
        rc = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    return rc;
}

// Delete user function
int admin_delete_user(sqlite3 *db, int user_id) {
    const char *sql = "DELETE FROM users WHERE user_id = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        rc = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    return rc;
}

// Add user function
int admin_add_user(sqlite3 *db, const User *new_user) {
    if (check_username_exists(db, new_user->username, -1) ||
        check_email_exists(db, new_user->email, -1)) {
        return SQLITE_CONSTRAINT;
    }

    const char *sql = "INSERT INTO users (username, password_hash, email, phone, "
                     "address_default, created_at, last_login) VALUES "
                     "(?, ?, ?, ?, ?, ?, ?)";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, new_user->username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, new_user->password_hash, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, new_user->email, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, new_user->phone, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, new_user->address_default);
        sqlite3_bind_int64(stmt, 6, (sqlite3_int64)time(NULL));
        sqlite3_bind_int64(stmt, 7, (sqlite3_int64)time(NULL));
        
        rc = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    return rc;
}

// elevate user to admin function remastered kai ultra
int admin_elevate_user_to_admin(sqlite3 *db, int user_id) {
    sqlite3_stmt *stmt;
    int rc;
    char *username = NULL;
    
    // Get that user ka name
    const char *sql_get_user = "SELECT username FROM users WHERE user_id = ?";
    rc = sqlite3_prepare_v2(db, sql_get_user, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        return rc;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *temp_username = sqlite3_column_text(stmt, 0);
        username = strdup((const char *)temp_username);
    } else {
        sqlite3_finalize(stmt);
        return SQLITE_ERROR; // User not found
    }
    
    sqlite3_finalize(stmt);
    
    // Is he an admin already?
    if (check_admin_exists(db, username)) {
        free(username);
        return SQLITE_CONSTRAINT; 
    }
    
    // If we get here, we can proceed with the elevation
    const char *sql_elevate = "INSERT INTO admins (admin_name, admin_password_hash) "
                            "SELECT username, password_hash FROM users WHERE user_id = ?";
    
    rc = sqlite3_prepare_v2(db, sql_elevate, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        rc = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    free(username);
    return rc;
}

// Modify product
int admin_modify_product(sqlite3 *db, const Product *modified_product) {                                          //again better use coalesce
    const char *sql = "UPDATE products SET name = ?, description = ?, price = ?, "
                     "stock = ?, category_id = ?, updated_at = ?, is_active = ? " 
                     "WHERE product_id = ?";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, modified_product->name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, modified_product->description, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, modified_product->price);
        sqlite3_bind_int(stmt, 4, modified_product->stock);
        sqlite3_bind_int(stmt, 5, modified_product->category_id);
        sqlite3_bind_int64(stmt, 6, (sqlite3_int64)time(NULL));                    // updated at should take  current time
        sqlite3_bind_int(stmt, 7, modified_product->is_active);
        sqlite3_bind_int(stmt, 8, modified_product->product_id);
        
        rc = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    return rc;
}

// Add product
int admin_add_product(sqlite3 *db, const Product *new_product) {
    const char *sql = "INSERT INTO products (name, description, price, stock, "
                     "category_id, created_at, updated_at, is_active) VALUES "
                     "(?, ?, ?, ?, ?, ?, ?, ?)";                                    // I have to add intert to the text file laterr for search functionality
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        time_t current_time = time(NULL);
        
        sqlite3_bind_text(stmt, 1, new_product->name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, new_product->description, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, new_product->price);
        sqlite3_bind_int(stmt, 4, new_product->stock);
        sqlite3_bind_int(stmt, 5, new_product->category_id);
        sqlite3_bind_int64(stmt, 6, (sqlite3_int64)current_time);                               
        sqlite3_bind_int64(stmt, 7, (sqlite3_int64)current_time);
        sqlite3_bind_int(stmt, 8, new_product->is_active);
        
        rc = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    return rc;
}

// Delete product
int admin_delete_product(sqlite3 *db, int product_id) {                                          // contrary to our prev database functions it is not checking !db condition
    const char *sql = "DELETE FROM products WHERE product_id = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, product_id);
        rc = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    return rc;
}
                                                                                               //need add category


// Modify category
int admin_modify_category(sqlite3 *db, const Category *modified_category) {
    const char *sql = "UPDATE categories SET name = ?, description = ? "
                     "WHERE category_id = ?";
                                                                                                 //again better use coalesce
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, modified_category->name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, modified_category->description, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, modified_category->category_id);
        
        rc = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    return rc;
}



bool check_admin_exists(sqlite3 *db, const char *admin_name) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM admins WHERE admin_name = ?";
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, admin_name, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return count > 0;
}

