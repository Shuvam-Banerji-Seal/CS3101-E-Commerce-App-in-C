
// Implementation of other functions would follow...
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <time.h>

static sqlite3 *db = NULL;

bool init_database(void) {
    int rc = sqlite3_open("ecommerce.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return false;
    }

    const char *sql[] = {
        // Users table
        "CREATE TABLE IF NOT EXISTS users ("
        "user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "login_id TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL,"
        "role INTEGER NOT NULL,"
        "email TEXT,"
        "phone TEXT,"
        "address_default INTEGER,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "last_login TIMESTAMP,"
        "FOREIGN KEY(address_default) REFERENCES addresses(address_id));",

        // Addresses table
        "CREATE TABLE IF NOT EXISTS addresses ("
        "address_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "address_line TEXT NOT NULL,"
        "city TEXT NOT NULL,"
        "postal_code TEXT NOT NULL,"
        "country TEXT NOT NULL,"
        "FOREIGN KEY(user_id) REFERENCES users(user_id));",

        // Products table
        "CREATE TABLE IF NOT EXISTS products ("
        "product_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "description TEXT,"
        "price DECIMAL(10,2) NOT NULL,"
        "stock INTEGER NOT NULL,"
        "category_id INTEGER,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "is_active BOOLEAN DEFAULT 1,"
        "FOREIGN KEY(category_id) REFERENCES categories(category_id));",

        // Categories table
        "CREATE TABLE IF NOT EXISTS categories ("
        "category_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "description TEXT,"
        "parent_id INTEGER,"
        "FOREIGN KEY(parent_id) REFERENCES categories(category_id));",

        // Cart table
        "CREATE TABLE IF NOT EXISTS cart ("
        "cart_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "product_id INTEGER NOT NULL,"
        "quantity INTEGER NOT NULL,"
        "added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(user_id) REFERENCES users(user_id),"
        "FOREIGN KEY(product_id) REFERENCES products(product_id));",

        // Orders table
        "CREATE TABLE IF NOT EXISTS orders ("
        "order_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "status TEXT NOT NULL,"
        "payment_status TEXT NOT NULL,"
        "delivery_type TEXT NOT NULL,"
        "address_id INTEGER,"
        "delivery_time TIME,"
        "total_amount DECIMAL(10,2) NOT NULL,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(user_id) REFERENCES users(user_id),"
        "FOREIGN KEY(address_id) REFERENCES addresses(address_id));",

        // Payments table
        "CREATE TABLE IF NOT EXISTS payments ("
        "payment_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "order_id INTEGER NOT NULL,"
        "amount DECIMAL(10,2) NOT NULL,"
        "method TEXT NOT NULL,"
        "status TEXT NOT NULL,"
        "transaction_id TEXT,"
        "payment_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(order_id) REFERENCES orders(order_id));",

        // Delivery vans table
        "CREATE TABLE IF NOT EXISTS delivery_vans ("
        "van_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "is_available BOOLEAN DEFAULT 1,"
        "next_available TIMESTAMP);",

        // Order items table
        "CREATE TABLE IF NOT EXISTS order_items ("
        "order_item_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "order_id INTEGER NOT NULL,"
        "product_id INTEGER NOT NULL,"
        "quantity INTEGER NOT NULL,"
        "price DECIMAL(10,2) NOT NULL,"
        "FOREIGN KEY(order_id) REFERENCES orders(order_id),"
        "FOREIGN KEY(product_id) REFERENCES products(product_id));",

        // Product search log table
        "CREATE TABLE IF NOT EXISTS product_search_log ("
        "search_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "search_term TEXT NOT NULL,"
        "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(user_id) REFERENCES users(user_id));"
    };

    char *err_msg = NULL;
    for (size_t i = 0; i < sizeof(sql) / sizeof(sql[0]); i++) {
        rc = sqlite3_exec(db, sql[i], 0, 0, &err_msg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
            return false;
        }
    }

    return true;
}

void cleanup_database(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

// User Management Functions
bool create_user(const User *user) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO users (login_id, password_hash, role, email, phone) "
                     "VALUES (?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, user->login_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user->password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, user->role);
    sqlite3_bind_text(stmt, 4, user->email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, user->phone, -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool verify_user(const char *login_id, const char *password) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT password_hash FROM users WHERE login_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, login_id, -1, SQLITE_STATIC);
    
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

bool get_user(const char *login_id, User *user) {
    if (!db || !user) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM users WHERE login_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, login_id, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    user->user_id = sqlite3_column_int(stmt, 0);
    user->login_id = strdup((const char *)sqlite3_column_text(stmt, 1));
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

// Address Management Functions
bool add_address(const Address *address) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO addresses (user_id, address_line, city, postal_code, country) "
                     "VALUES (?, ?, ?, ?, ?);";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, address->user_id);
    sqlite3_bind_text(stmt, 2, address->address_line, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, address->city, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, address->postal_code, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, address->country, -1, SQLITE_STATIC);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool get_user_addresses(int user_id, Address **addresses, int *count) {
    if (!db || !addresses || !count) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM addresses WHERE user_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    // Count results first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }
    sqlite3_reset(stmt);
    
    // Allocate memory for results
    *addresses = malloc(sizeof(Address) * (*count));
    if (!*addresses) {
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch results
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Address *addr = &(*addresses)[i++];
        addr->address_id = sqlite3_column_int(stmt, 0);
        addr->user_id = sqlite3_column_int(stmt, 1);
        addr->address_line = strdup((const char *)sqlite3_column_text(stmt, 2));
        addr->city = strdup((const char *)sqlite3_column_text(stmt, 3));
        addr->postal_code = strdup((const char *)sqlite3_column_text(stmt, 4));
        addr->country = strdup((const char *)sqlite3_column_text(stmt, 5));
    }
    
    sqlite3_finalize(stmt);
    return true;
}

// Product Management Functions
bool add_product(const Product *product) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO products (name, description, price, stock, category_id, is_active) "
                     "VALUES (?, ?, ?, ?, ?, ?);";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, product->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, product->description, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, product->price);
    sqlite3_bind_int(stmt, 4, product->stock);
    sqlite3_bind_int(stmt, 5, product->category_id);
    sqlite3_bind_int(stmt, 6, product->is_active);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool get_product(int product_id, Product *product) {
    if (!db || !product) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM products WHERE product_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, product_id);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    product->product_id = sqlite3_column_int(stmt, 0);
    product->name = strdup((const char *)sqlite3_column_text(stmt, 1));
    product->description = sqlite3_column_text(stmt, 2) ? strdup((const char *)sqlite3_column_text(stmt, 2)) : NULL;
    product->price = sqlite3_column_double(stmt, 3);
    product->stock = sqlite3_column_int(stmt, 4);
    product->category_id = sqlite3_column_int(stmt, 5);
    product->created_at = sqlite3_column_int64(stmt, 6);
    product->updated_at = sqlite3_column_int64(stmt, 7);
    product->is_active = sqlite3_column_int(stmt, 8);

    sqlite3_finalize(stmt);
    return true;
}

// Order Management Functions
bool create_order(const Order *order) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO orders (user_id, status, payment_status, delivery_type, "
                     "address_id, delivery_time, total_amount) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, order->user_id);
    sqlite3_bind_int(stmt, 2, order->status);
    sqlite3_bind_int(stmt, 3, order->payment_status);
    sqlite3_bind_int(stmt, 4, order->delivery_type);
    sqlite3_bind_int(stmt, 5, order->address_id);
    sqlite3_bind_text(stmt, 6, order->delivery_time, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 7, order->total_amount);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

// Payment Management Functions
bool create_payment(const Payment *payment) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO payments (order_id, amount, method, status, transaction_id) "
                     "VALUES (?, ?, ?, ?, ?);";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, payment->order_id);
    sqlite3_bind_double(stmt, 2, payment->amount);
    sqlite3_bind_int(stmt, 3, payment->method);
    sqlite3_bind_text(stmt, 4, payment->status, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, payment->transaction_id, -1, SQLITE_STATIC);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

// Address management
bool set_default_address(int user_id, int address_id) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE users SET address_default = ? WHERE user_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, address_id);
    sqlite3_bind_int(stmt, 2, user_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

// Product management
bool update_product(const Product *product) {
    if (!db || !product) return false;

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE products SET name = ?, description = ?, price = ?, "
                     "stock = ?, category_id = ?, is_active = ?, "
                     "updated_at = CURRENT_TIMESTAMP "
                     "WHERE product_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, product->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, product->description, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, product->price);
    sqlite3_bind_int(stmt, 4, product->stock);
    sqlite3_bind_int(stmt, 5, product->category_id);
    sqlite3_bind_int(stmt, 6, product->is_active);
    sqlite3_bind_int(stmt, 7, product->product_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool delete_product(int product_id) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    // Soft delete by setting is_active to false
    const char *sql = "UPDATE products SET is_active = 0, "
                     "updated_at = CURRENT_TIMESTAMP WHERE product_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, product_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool get_all_products(Product **products, int *count) {
    if (!db || !products || !count) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM products WHERE is_active = 1;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    // Count results first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }
    sqlite3_reset(stmt);
    
    // Allocate memory for results
    *products = malloc(sizeof(Product) * (*count));
    if (!*products) {
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch results
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Product *prod = &(*products)[i++];
        prod->product_id = sqlite3_column_int(stmt, 0);
        prod->name = strdup((const char *)sqlite3_column_text(stmt, 1));
        prod->description = sqlite3_column_text(stmt, 2) ? 
            strdup((const char *)sqlite3_column_text(stmt, 2)) : NULL;
        prod->price = sqlite3_column_double(stmt, 3);
        prod->stock = sqlite3_column_int(stmt, 4);
        prod->category_id = sqlite3_column_int(stmt, 5);
        prod->created_at = sqlite3_column_int64(stmt, 6);
        prod->updated_at = sqlite3_column_int64(stmt, 7);
        prod->is_active = sqlite3_column_int(stmt, 8);
    }
    
    sqlite3_finalize(stmt);
    return true;
}

// Category management
bool add_category(const Category *category) {
    if (!db || !category) return false;

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO categories (name, description, parent_id) "
                     "VALUES (?, ?, ?);";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, category->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, category->description, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, category->parent_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool get_category_products(int category_id, Product **products, int *count) {
    if (!db || !products || !count) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM products WHERE category_id = ? AND is_active = 1;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, category_id);
    
    // Count results first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }
    sqlite3_reset(stmt);
    
    // Allocate memory for results
    *products = malloc(sizeof(Product) * (*count));
    if (!*products) {
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch results
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Product *prod = &(*products)[i++];
        prod->product_id = sqlite3_column_int(stmt, 0);
        prod->name = strdup((const char *)sqlite3_column_text(stmt, 1));
        prod->description = sqlite3_column_text(stmt, 2) ? 
            strdup((const char *)sqlite3_column_text(stmt, 2)) : NULL;
        prod->price = sqlite3_column_double(stmt, 3);
        prod->stock = sqlite3_column_int(stmt, 4);
        prod->category_id = sqlite3_column_int(stmt, 5);
        prod->created_at = sqlite3_column_int64(stmt, 6);
        prod->updated_at = sqlite3_column_int64(stmt, 7);
        prod->is_active = sqlite3_column_int(stmt, 8);
    }
    
    sqlite3_finalize(stmt);
    return true;
}

// Cart management
bool update_cart_quantity(int cart_id, int quantity) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE cart SET quantity = ? WHERE cart_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, quantity);
    sqlite3_bind_int(stmt, 2, cart_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool remove_from_cart(int cart_id) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM cart WHERE cart_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, cart_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

// Order management
bool update_order_status(int order_id, OrderStatus status) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE orders SET status = ?, updated_at = CURRENT_TIMESTAMP "
                     "WHERE order_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, status);
    sqlite3_bind_int(stmt, 2, order_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

// Payment management
bool update_payment_status(int payment_id, const char *status) {
    if (!db || !status) return false;

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE payments SET status = ? WHERE payment_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, payment_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

// Delivery van management
bool is_van_available(const char *delivery_time) {
    if (!db || !delivery_time) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM delivery_vans "
                     "WHERE is_available = 1 AND "
                     "(next_available IS NULL OR next_available <= datetime(?));";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, delivery_time, -1, SQLITE_STATIC);
    
    bool available = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        available = sqlite3_column_int(stmt, 0) > 0;
    }
    
    sqlite3_finalize(stmt);
    return available;
}

bool update_van_availability(int van_id, bool is_available, const char *next_available) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE delivery_vans SET is_available = ?, next_available = ? "
                     "WHERE van_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, is_available);
    sqlite3_bind_text(stmt, 2, next_available, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, van_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

// Search functionality
bool search_products(const char *term, Product **results, int *count) {
    if (!db || !term || !results || !count) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM products WHERE "
                     "(name LIKE ? OR description LIKE ?) "
                     "AND is_active = 1;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    char *search_term = sqlite3_mprintf("%%%s%%", term);
    sqlite3_bind_text(stmt, 1, search_term, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, search_term, -1, SQLITE_STATIC);
    
    // Count results first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }
    sqlite3_reset(stmt);
    
    // Allocate memory for results
    *results = malloc(sizeof(Product) * (*count));
    if (!*results) {
        sqlite3_free(search_term);
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch results
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Product *prod = &(*results)[i++];
        prod->product_id = sqlite3_column_int(stmt, 0);
        prod->name = strdup((const char *)sqlite3_column_text(stmt, 1));
        prod->description = sqlite3_column_text(stmt, 2) ? 
            strdup((const char *)sqlite3_column_text(stmt, 2)) : NULL;
        prod->price = sqlite3_column_double(stmt, 3);
        prod->stock = sqlite3_column_int(stmt, 4);
        prod->category_id = sqlite3_column_int(stmt, 5);
        prod->created_at = sqlite3_column_int64(stmt, 6);
        prod->updated_at = sqlite3_column_int64(stmt, 7);
        prod->is_active = sqlite3_column_int(stmt, 8);
    }
    
    sqlite3_free(search_term);
    sqlite3_finalize(stmt);
    return true;
}

// Helper function to free product array memory
void free_products(Product *products, int count) {
    if (!products) return;
    
    for (int i = 0; i < count; i++) {
        free((char*)products[i].name);
        free((char*)products[i].description);
    }
    free(products);
}