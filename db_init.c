

static sqlite3 *db = NULL;

bool init_database(void) {
    const char *db_file = "ecommerce.db";
    int rc = sqlite3_open(db_file, &db);
    //int rc = sqlite3_open("ecommerce.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return false;
    }



    

    const char *sql[] = {
    // Users table
    "CREATE TABLE IF NOT EXISTS users ("
    "user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "username TEXT UNIQUE NOT NULL,"
    "password_hash TEXT NOT NULL,"
    "email TEXT NOT NULL,"
    "phone TEXT,"
    "address_default INTEGER,"
    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "last_login TIMESTAMP,"
    "status INTEGER DEFAULT 1,"
    "email_verified BOOLEAN DEFAULT 0,"
    "phone_verified BOOLEAN DEFAULT 0,"
    "profile_image BLOB,"
    "FOREIGN KEY(address_default) REFERENCES addresses(address_id));",

    // // Create user sequence table
    //     "CREATE TABLE IF NOT EXISTS users_sequence ("
    //     "id INTEGER PRIMARY KEY CHECK (id = 1),"
    //     "next_val INTEGER DEFAULT 0"
    //     ");",
    //     "INSERT OR IGNORE INTO users_sequence (id, next_val) VALUES (1, 0);",
    //     // Create trigger with fixed syntax
    //     "CREATE TRIGGER IF NOT EXISTS generate_user_id "
    //     "AFTER INSERT ON users "
    //     "WHEN NEW.user_id IS NULL "
    //     "BEGIN "
    //     "   UPDATE users_sequence SET next_val = next_val + 1 WHERE id = 1; "
    //     "   UPDATE users "
    //     "   SET user_id = ("
    //     "       SELECT 'user' || CAST(next_val AS TEXT) || SUBSTR(NEW.username, 1, 1) "
    //     "       FROM users_sequence WHERE id = 1"
    //     "   ) "
    //     "   WHERE rowid = NEW.rowid; "
    //     "END;",

    // Addresses table
    "CREATE TABLE IF NOT EXISTS addresses ("
    "address_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "user_id INTEGER NOT NULL,"
    "address_line TEXT NOT NULL,"
    "city TEXT NOT NULL,"
    "postal_code TEXT NOT NULL,"
    "country TEXT NOT NULL,"
    "type TEXT DEFAULT 'home',"
    "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE);",

    // Products table
    "CREATE TABLE IF NOT EXISTS products ("
    "product_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "name TEXT NOT NULL,"
    "description TEXT,"
    "price DECIMAL(10,2) NOT NULL,"
    "stock INTEGER DEFAULT 0,"
    "sku TEXT UNIQUE,"
    "discount DECIMAL(10,2) DEFAULT 0.00,"
    "rating DECIMAL(3,2) DEFAULT 0.00,"
    "category_id INTEGER,"
    "product_image BLOB,"
    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "is_active BOOLEAN DEFAULT 1"
    ");",
    
    //coupons table
    "CREATE TABLE IF NOT EXISTS coupons ("
    "coupon_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "code INTEGER UNIQUE ,"
    "discount_type TEXT CHECK(discount_type IN ('percentage', 'fixed')) NOT NULL,"
    "discount_value DECIMAL(10,2) NOT NULL,"
    "min_order_value DECIMAL(10,2) DEFAULT 0.00,"
    "start_date TIMESTAMP ,"
    "end_date TIMESTAMP ,"
    "is_active BOOLEAN DEFAULT 1,"
    "usage_limit INTEGER DEFAULT 1,"
    "usage_count INTEGER DEFAULT 0,"
    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);",

    "CREATE TRIGGER IF NOT EXISTS add_welcome_coupon "
    "AFTER INSERT ON users "
    "BEGIN "
    "   INSERT INTO coupons (code, discount_type, discount_value, min_order_value, usage_limit) "
    "   SELECT 50, 'percentage', 50.00, 100.00, 1 "
    "   WHERE NOT EXISTS ("
    "       SELECT 1 FROM coupons "
    "       WHERE code = 50 AND discount_type = 'percentage' "
    "       AND discount_value = 50.00 AND min_order_value = 0.00"
    "   ); "
    "END;",
    
    // Categories table
    "CREATE TABLE IF NOT EXISTS categories ("
    "category_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "name TEXT NOT NULL,"
    "description TEXT,"
    
    "parent_id INTEGER);",

    // Cart table
    "CREATE TABLE IF NOT EXISTS cart ("
    "cart_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "user_id INTEGER NOT NULL,"
    "product_id INTEGER NOT NULL,"
    "quantity INTEGER NOT NULL,"
    "last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE,"
    "FOREIGN KEY(product_id) REFERENCES products(product_id));",
    
    // // Admin table
    // "CREATE TABLE IF NOT EXISTS admins ("
    // "admin_id TEXT PRIMARY KEY ,"
    // "admin_name TEXT NOT NULL,"
    // "admin_password_hash TEXT NOT NULL);",

        
       
        // Drop existing admin-related objects
        //"DROP TABLE IF EXISTS admins;",
        //"DROP TABLE IF EXISTS admin_sequence;",
        //"DROP TRIGGER IF EXISTS generate_admin_id;",

        // Create admin sequence table
        "CREATE TABLE IF NOT EXISTS admin_sequence ("
        "id INTEGER PRIMARY KEY CHECK (id = 1),"
        "next_val INTEGER DEFAULT 0"
        ");",

        // Create admins table
        "CREATE TABLE IF NOT EXISTS admins ("
        "admin_id TEXT PRIMARY KEY,"
        "admin_name TEXT NOT NULL,"
        "admin_password_hash TEXT NOT NULL"
        ");",
        
        "INSERT OR IGNORE INTO admin_sequence (id, next_val) VALUES (1, 0);",
        // Create trigger with fixed syntax
        "CREATE TRIGGER IF NOT EXISTS generate_admin_id "
        "AFTER INSERT ON admins "
        "WHEN NEW.admin_id IS NULL "
        "BEGIN "
        "   UPDATE admin_sequence SET next_val = next_val + 1 WHERE id = 1; "
        "   UPDATE admins "
        "   SET admin_id = ("
        "       SELECT 'admin' || CAST(next_val AS TEXT) || SUBSTR(NEW.admin_name, 1, 1) "
        "       FROM admin_sequence WHERE id = 1"
        "   ) "
        "   WHERE rowid = NEW.rowid; "
        "END;",


    // "   CREATE TABLE IF NOT EXISTS   admin2( "
   
    // "admin_name TEXT NOT NULL,"
    // " admin_password_hash TEXT NOT NULL, "
    // "admin_counter INTEGER  PRIMARY KEY AUTOINCREMENT, "
    //  "admin_id TEXT ,  "
    // " );",

   // "UPDATE admin2 "
   // " (SELECT 'admin2' || CAST(admin_counter AS TEXT) || SUBSTR(admin_name, 1, 1) FROM admin2) WHERE admin_id IS NULL ;",

    
    // Orders table
    "CREATE TABLE IF NOT EXISTS orders ("
    "order_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "user_id INTEGER NOT NULL,"
    "status INTEGER DEFAULT 0,"
    "payment_status INTEGER DEFAULT 0,"
    "delivery_type INTEGER,"
    "address_id INTEGER,"
    "delivery_time TIME,"
    "estimated_delivery_date TIMESTAMP,"
    "order_status INTEGER DEFAULT 0,"
    "total_amount DECIMAL(10,2) NOT NULL,"
    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "bill_status INTEGER DEFAULT 0,"
    "is_delayed BOOLEAN DEFAULT 0,"
    "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE,"
    "FOREIGN KEY(address_id) REFERENCES addresses(address_id));",

    // Payments table
    "CREATE TABLE IF NOT EXISTS payments ("
    "payment_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "order_id INTEGER NOT NULL,"
    "amount DECIMAL(10,2) NOT NULL,"
    "method INTEGER DEFAULT 0,"
    "status INTEGER DEFAULT 0,"
    "transaction_id TEXT,"
    "payment_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "FOREIGN KEY(order_id) REFERENCES orders(order_id) ON DELETE CASCADE);",

    //bills table
    "CREATE TABLE IF NOT EXISTS bills ("
    "bill_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "order_id INTEGER NOT NULL,"
    "user_id INTEGER NOT NULL,"
    "amount DECIMAL(10,2),"
    "status INTEGER DEFAULT 0,"
    "bill_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "FOREIGN KEY(order_id) REFERENCES orders(order_id) "
    "FOREIGN KEY(user_id) REFERENCES users(user_id) );",

    // Delivery vans table
    "CREATE TABLE IF NOT EXISTS delivery_vans ("
    "van_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "is_available BOOLEAN DEFAULT 1,"
    "next_available TIMESTAMP,"
    "capacity INTEGER,"
    "current_location TEXT);",

    // Order items table
    "CREATE TABLE IF NOT EXISTS order_items ("
    "order_item_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "order_id INTEGER NOT NULL,"
    "product_id INTEGER NOT NULL,"
    "quantity INTEGER NOT NULL,"
    "price DECIMAL(10,2) NOT NULL,"
    "subtotal DECIMAL(10,2) GENERATED ALWAYS AS (quantity * price) STORED,"
    "FOREIGN KEY(order_id) REFERENCES orders(order_id) ON DELETE CASCADE,"
    "FOREIGN KEY(product_id) REFERENCES products(product_id));",

    // Product search log table
    "CREATE TABLE IF NOT EXISTS product_search_log ("
    "search_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "user_id INTEGER NOT NULL,"
    "search_term TEXT NOT NULL,"
    "results_count INTEGER,"
    "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE);",

    // Wishlists table
    "CREATE TABLE IF NOT EXISTS wishlists ("
    "wishlist_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "user_id INTEGER NOT NULL,"
    "product_id INTEGER NOT NULL,"
    "added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE,"
    "FOREIGN KEY(product_id) REFERENCES products(product_id));",

    // Reviews table
    "CREATE TABLE IF NOT EXISTS reviews ("
    "review_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "user_id INTEGER NOT NULL,"
    "product_id INTEGER NOT NULL,"
    "rating INTEGER NOT NULL CHECK(rating BETWEEN 1 AND 5),"
    "comment TEXT,"
    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE,"
    "FOREIGN KEY(product_id) REFERENCES products(product_id));",

    // Shipping table
    "CREATE TABLE IF NOT EXISTS shipping ("
    "shipping_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "order_id INTEGER NOT NULL,"
    "carrier_name TEXT NOT NULL,"
    "tracking_number TEXT,"
    "status TEXT,"
    "shipped_at TIMESTAMP,"
    "delivered_at TIMESTAMP,"
    "FOREIGN KEY(order_id) REFERENCES orders(order_id) ON DELETE CASCADE);",

    // Notifications table
    "CREATE TABLE IF NOT EXISTS notifications ("
    "notification_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "user_id INTEGER NOT NULL,"
    "message TEXT NOT NULL,"
    "is_read BOOLEAN DEFAULT 0,"
    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE);"

    // Now add the foreign key constraint to addresses after users exists
    "CREATE TRIGGER IF NOT EXISTS fk_addresses_user "
    "BEFORE INSERT ON addresses "
    "FOR EACH ROW "
    "BEGIN "
    "  SELECT CASE "
    "    WHEN NEW.user_id IS NOT NULL AND "
    "         (SELECT user_id FROM users WHERE user_id = NEW.user_id) IS NULL "
    "    THEN RAISE(ABORT, 'Foreign key violation: user_id does not exist in users table') "
    "  END; "
    "END;",
    };

 
    // return true;
    char *err_msg = NULL;
    rc = sqlite3_exec(db, *sql, 0, 0, &err_msg);
    
    // if (rc != SQLITE_OK) {
    //     fprintf(stderr, "SQL error: %s\n", err_msg);
    //     sqlite3_free(err_msg);
    //     return false;
    // }

    // // Enable foreign key support
    // rc = sqlite3_exec(db, "PRAGMA foreign_keys = OFF;", NULL, NULL, &err_msg);
    // if (rc != SQLITE_OK) {
    //     fprintf(stderr, "Failed to enable foreign keys: %s\n", err_msg);
    //     sqlite3_free(err_msg);
    //     sqlite3_close(db);
    //     return false;
    // }

    // Begin transaction for all table creations
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to begin transaction: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return false;
    }

    // Execute all CREATE TABLE statements
    for (size_t i = 0; i < sizeof(sql) / sizeof(sql[0]); i++) {
        rc = sqlite3_exec(db, sql[i], NULL, NULL, &err_msg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error in statement %zu: %s\n", i, err_msg);
            sqlite3_free(err_msg);
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            sqlite3_close(db);
            return false;
        }
    }

    // Commit transaction
    rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to commit transaction: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_close(db);
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

