#include "database.h"
#include "db_init.c"
#include "auth.c"
#include "cart.c"
#include "search_bk.c"
#include "other_functions.c"
//#include "tokenizer.c"

//bool search_products(sqlite3 *db, const char *search_term, ProductSearchResult **results, int *result_count) ;



//no idea why but this function likes to be here in this c file
bool search_products( Product **results, int *count);



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
       strcpy( (addr->address_line) ,sqlite3_column_text(stmt, 2));
        strcpy(addr->city , sqlite3_column_text(stmt, 3));
       strcpy( addr->postal_code ,sqlite3_column_text(stmt, 4));
       strcpy( addr->country ,sqlite3_column_text(stmt, 5));
    }
    
    sqlite3_finalize(stmt);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product Management Functions


bool add_product(const Product *product) {
    if (!db) {
        printf("Database connection not initialized.\n");
        return false;
    }

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO products (name, description, price, stock, category_id, is_active) "
                      "VALUES (?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return false;
    }
    char *words1[1024];
    char buffer1[1024];
    char buffer2[1024];
    int count_file_appender1 = 0;

    char *words2[1024];
    int count_file_appender2 = 0;

    sqlite3_bind_text(stmt, 1, product->name, -1, SQLITE_STATIC);
    printf("Bound name: %s\n", product->name);
    strcpy(buffer1, product->name);
    file_appender1(buffer1, words1, count_file_appender1);
    sqlite3_bind_text(stmt, 2, product->description, -1, SQLITE_STATIC);
    strcpy(buffer2, product->description);
    file_appender1(buffer2, words2, count_file_appender2);
    printf("Bound description: %s\n", product->description);
    sqlite3_bind_double(stmt, 3, product->price);
    printf("Bound price: %.2f\n", product->price);
    sqlite3_bind_int(stmt, 4, product->stock);
    printf("Bound stock: %d\n", product->stock);
    sqlite3_bind_int(stmt, 5, product->category_id);
    printf("Bound category ID: %d\n", product->category_id);
    sqlite3_bind_int(stmt, 6, product->is_active);
    printf("Bound is_active: %d\n", product->is_active);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    if (success) {
        printf("Product added successfully.\n");
    } else {
        printf("Failed to add product: %s\n", sqlite3_errmsg(db));
    }

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
    //product->name = sqlite3_column_text(stmt, 1) ? strdup((const char *)sqlite3_column_text(stmt, 1)) : NULL;
    strcpy( product->name, sqlite3_column_text(stmt, 1) ? strdup((const char *)sqlite3_column_text(stmt, 1)) : NULL);
   // product->description = sqlite3_column_text(stmt, 2) ? strdup((const char *)sqlite3_column_text(stmt, 2)) : NULL;
    strcpy(product->description,sqlite3_column_text(stmt, 2) ? strdup((const char *)sqlite3_column_text(stmt, 2)) : NULL);
    product->price = sqlite3_column_double(stmt, 3);
    product->stock = sqlite3_column_int(stmt, 4);
    product->category_id = sqlite3_column_int(stmt, 5);
    product->created_at = sqlite3_column_int64(stmt, 6);
    product->updated_at = sqlite3_column_int64(stmt, 7);
    product->is_active = sqlite3_column_int(stmt, 8);

    sqlite3_finalize(stmt);
    return true;
}


bool update_product(const Product *product) {
    if (!db || !product) return false;

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE products SET name = COALESCE(?, name),  description = COALESCE(?, description), price =COALESCE(?, price), "
                     "stock =COALESCE(?, stock), category_id =COALESCE(?, category_id), is_active = COALESCE(?, is_active), "
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





bool get_all_products(Product **products, int *count) {
    if (!db || !count) return false;
    sqlite3_stmt *stmt;
    const char *sql = "SELECT product_id, name, description, price, stock, sku, "
                     "discount, rating, category_id, created_at, updated_at, is_active "
                     "FROM products WHERE is_active = 1;";
    
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
    *products = calloc(*count, sizeof(Product));
    if (!*products) {
        sqlite3_finalize(stmt);
        return false;
    }

    // Fetch results
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Product *prod = &(*products)[i++];
        
        // Handle required fields
        prod->product_id = sqlite3_column_int(stmt, 0);
        
        // Handle name
        const char *name_text = (const char *)sqlite3_column_text(stmt, 1);
        if (name_text) {
            strncpy(prod->name, name_text, sizeof(prod->name) - 1);
            prod->name[sizeof(prod->name) - 1] = '\0';
        } else {
            prod->name[0] = '\0';
        }

        // Handle description
        const char *desc_text = (const char *)sqlite3_column_text(stmt, 2);
        if (desc_text) {
            strncpy(prod->description, desc_text, sizeof(prod->description) - 1);
            prod->description[sizeof(prod->description) - 1] = '\0';
        } else {
            prod->description[0] = '\0';
        }

        // Handle numeric fields
        prod->price = sqlite3_column_double(stmt, 3);
        prod->stock = sqlite3_column_int(stmt, 4);
        
        // Handle SKU
        const char *sku_text = (const char *)sqlite3_column_text(stmt, 5);
        if (sku_text) {
            strncpy(prod->sku, sku_text, sizeof(prod->sku) - 1);
            prod->sku[sizeof(prod->sku) - 1] = '\0';
        } else {
            prod->sku[0] = '\0';
        }

        prod->discount = sqlite3_column_double(stmt, 6);
        prod->rating = sqlite3_column_double(stmt, 7);
        prod->category_id = sqlite3_column_int(stmt, 8);
        
        // Handle timestamps directly as integers
        // prod->created_at = sqlite3_column_int64(stmt, 9);
        // prod->updated_at = sqlite3_column_int64(stmt, 10);
        
        prod->is_active = sqlite3_column_int(stmt, 11);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool get_category_products(int category_id, Product **products, int *count) {
    if (!db || !count) return false;

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
    *products = (Product*)malloc(sizeof(Product) * (*count));
    if (!*products) {
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch results
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Product *prod = &(*products)[i++];
        prod->product_id = sqlite3_column_int(stmt, 0);
       // prod->name = strdup((const char *)sqlite3_column_text(stmt, 1));
        strcpy(prod->name,sqlite3_column_text(stmt, 1) ? strdup((const char *)sqlite3_column_text(stmt, 1)) : NULL);
        //prod->description = sqlite3_column_text(stmt, 2) ? 
          //  strdup((const char *)sqlite3_column_text(stmt, 2)) : NULL;
        strcpy(prod->description,sqlite3_column_text(stmt, 2) ? strdup((const char *)sqlite3_column_text(stmt, 2)) : NULL);
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

bool update_product_active_status() {
    if (!db) {
        return false;
    }

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE products SET is_active = 0 WHERE stock <= 0 AND is_active = 1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing active status update query: %s\n", sqlite3_errmsg(db));
        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error updating product active status: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}



bool search_products( Product **results, int *count) {
    if (!db || !results || !count) return false;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM products WHERE "
                     "(name LIKE ? OR name LIKE ? OR description LIKE ? OR description LIKE ?) "
                     "AND is_active = 1;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    char searcher1[256];
    char searcher2[256];
    char searcher3[256];
    char searcher4[256];
    stng_breaker(searcher1,searcher2);
    search_helper(searcher1,searcher3);
    search_helper(searcher2,searcher4);
    char *search_term1 = sqlite3_mprintf("%%%s%%", searcher1);
    char *search_term2 = sqlite3_mprintf("%%%s%%", searcher2);
    char *search_term3 = sqlite3_mprintf("%%%s%%", searcher3);
    char *search_term4 = sqlite3_mprintf("%%%s%%", searcher4);
    sqlite3_bind_text(stmt, 1, search_term1, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, search_term2, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, search_term3, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, search_term4, -1, SQLITE_STATIC);

    
    // Count results first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }
    sqlite3_reset(stmt);
    
    // Allocate memory for results
    *results = malloc(sizeof(Product) * (*count));
    if (!*results) {
        sqlite3_free(search_term1);
        sqlite3_free(search_term2);
        sqlite3_free(search_term3);
        sqlite3_free(search_term4);
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch results
    // int i = 0;
    // while (sqlite3_step(stmt) == SQLITE_ROW) {
    //     Product *prod = &(*results)[i++];
    //     prod->product_id = sqlite3_column_int(stmt, 0);
    //     printf("product id is %d\n", prod->product_id);
    //     strcpy(prod->name, (const char *)sqlite3_column_text(stmt, 1));
    //     printf("%s\n", prod->name);
    //     strcpy(prod->description, (const char *)sqlite3_column_text(stmt, 2));
    //     prod->price = sqlite3_column_double(stmt, 3);
    //     prod->stock = sqlite3_column_int(stmt, 4);
    //     prod->category_id = sqlite3_column_int(stmt, 5);
    //     prod->created_at = sqlite3_column_int64(stmt, 6);
    //     prod->updated_at = sqlite3_column_int64(stmt, 7);
    //     prod->is_active = sqlite3_column_int(stmt, 8);
    // }

    int i = 0;
while (sqlite3_step(stmt) == SQLITE_ROW) {
    Product *prod = &(*results)[i++];
    prod->product_id = sqlite3_column_int(stmt, 0);
    
    // Enhanced print formatting
    printf("Product #%d (ID: %d)\n", i, prod->product_id);
    printf("--------------------\n");
    
    // Safely handle potential NULL values
    const char *name = (const char *)sqlite3_column_text(stmt, 1);
    strcpy(prod->name, name ? name : "N/A");
    printf("Name: %s\n", prod->name);
    
    const char *description = (const char *)sqlite3_column_text(stmt, 2);
    strcpy(prod->description, description ? description : "No description");
    printf("Description: %s\n", prod->description);
    
    prod->price = sqlite3_column_double(stmt, 3);
    printf("Price: $%.2f\n", prod->price);
    
    prod->stock = sqlite3_column_int(stmt, 4);
    printf("Stock: %d\n", prod->stock);
    
    prod->category_id = sqlite3_column_int(stmt, 5);
    printf("Category ID: %d\n", prod->category_id);
    
    prod->created_at = sqlite3_column_int64(stmt, 6);
    prod->updated_at = sqlite3_column_int64(stmt, 7);
    
    prod->is_active = sqlite3_column_int(stmt, 8);
    printf("Active: %s\n\n", prod->is_active ? "Yes" : "No");
}
    


    
    // sqlite3_free(search_term);
    sqlite3_finalize(stmt);
    sqlite3_free(search_term1);
        sqlite3_free(search_term2);
        sqlite3_free(search_term3);
        sqlite3_free(search_term4);
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

float total_price(int product_id){
    float price;
    sqlite3_stmt *stmt;
    const char* sql= "SELECT price FROM products WHERE product_id=?;";
    if(sqlite3_prepare_v2(db , sql,-1,&stmt,NULL)!=SQLITE_OK){
        exit(1);

    }
    sqlite3_bind_int(stmt,1,product_id);
    if (sqlite3_step(stmt)==SQLITE_ROW){
        price=sqlite3_column_double(stmt,0);
        return price;

    } else{
        exit(1);}
    sqlite3_finalize(stmt);
    


}



bool get_all_categorys(Category **categories, int *count){

    sqlite3_stmt *stmt;
    const char *sql="SELECT * FROM categories;";
    if(sqlite3_prepare_v2(db,sql,-1,&stmt,NULL)!=SQLITE_OK){
        return false;
    }
    *count=0;
    while(sqlite3_step(stmt)==SQLITE_ROW){
        (*count)++;
    }
    sqlite3_reset(stmt);
    *categories= (Category *)malloc(sizeof(Category)*(*count));
    if (!*categories){
        sqlite3_finalize(stmt);
        return false;
    }
    int i=0;
    while(sqlite3_step(stmt)==SQLITE_ROW){
        Category *category= &(*categories)[i++];
        category->category_id=sqlite3_column_int(stmt,0);
       //strcpy( category->name,sqlite3_column_text(stmt,1));
        strcpy(category->name,(char *)sqlite3_column_text(stmt,1));
        strcpy(category->description, (char *)sqlite3_column_text(stmt,2));
    }
    sqlite3_finalize(stmt);
    return true;
}

bool update_category(const Category *category,int category_id){
    sqlite3_stmt *stmt;
    const char *sql="UPDATE categories SET name=COALESCE(?,name),description=COALESCE( ?,description) WHERE category_id=?;";
    if(sqlite3_prepare_v2(db,sql,-1,&stmt,NULL)!=SQLITE_OK){
        return false;
    }
    sqlite3_bind_text(stmt,1,category->name,-1,NULL);
    sqlite3_bind_text(stmt,2,category->description,-1,NULL);
    sqlite3_bind_int(stmt,2,category->category_id);
    bool success=sqlite3_step(stmt)==SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I know that these blocks will not be maintained


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
    //sqlite3_bind_int(stmt, 6, order->delivery_time);
    sqlite3_bind_text(stmt, 6, order->delivery_time, -1, SQLITE_STATIC);  // bind as text

    sqlite3_bind_double(stmt, 7, order->total_amount);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}


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






/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



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
    sqlite3_bind_int(stmt, 4, payment->status);
    sqlite3_bind_text(stmt, 5, payment->transaction_id, -1, SQLITE_STATIC);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}


bool update_payment_status(int payment_id, PaymentStatus status) {
    if (!db || !status) return false;

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE payments SET status = ? WHERE payment_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, status);
    sqlite3_bind_int(stmt, 2, payment_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Delivery van management
// bool is_van_available(const char *delivery_time) {
//     if (!db || !delivery_time) return false;

//     sqlite3_stmt *stmt;
//     const char *sql = "SELECT COUNT(*) FROM delivery_vans "
//                      "WHERE is_available = 1 AND "
//                      "(next_available IS NULL OR next_available <= datetime(?));";
    
//     if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
//         return false;
//     }
    
//     sqlite3_bind_text(stmt, 1, delivery_time, -1, SQLITE_STATIC);
    
//     bool available = false;
//     if (sqlite3_step(stmt) == SQLITE_ROW) {
//         available = sqlite3_column_int(stmt, 0) > 0;
//     }
    
//     sqlite3_finalize(stmt);
//     return available;
// }

// Modified van availability check with random probability

// the randomizer isn't working sadly

// bool is_van_available(const char *delivery_time) {
//     //if (!db || !delivery_time) return false;
//     if (!db) return false;
//     // Seed random number generator
//     srand(time(NULL));
    
//     // 30% chance that no van is available
//     int random_chance = rand() % 100;
//     if (random_chance < 30) {
//         return false;
//     }
    
//     sqlite3_stmt *stmt;
//     const char *sql = "SELECT COUNT(*) FROM delivery_vans "
//                      "WHERE is_available = 1 AND "
//                      "(next_available IS NULL OR next_available <= datetime(?));";
    
//     if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
//         return false;
//     }
    
//     sqlite3_bind_text(stmt, 1, delivery_time, -1, SQLITE_STATIC);
//     bool available = false;
    
//     if (sqlite3_step(stmt) == SQLITE_ROW) {
//         available = sqlite3_column_int(stmt, 0) > 0;
//     }
    
//     sqlite3_finalize(stmt);
//     return available;
// }


bool is_van_available(const char *delivery_time) {
    (void)delivery_time; // Suppress unused parameter warning
    srand(time(NULL)); // Seed the random number generator

    return rand() % 1; // Return true or false randomly
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


// Function to assign a delivery van
bool assign_delivery_van(const char *delivery_time) {
    if (!db || !delivery_time) return false;
    
    sqlite3_stmt *stmt;
    // Find an available van
    const char *sql = "SELECT van_id FROM delivery_vans "
                     "WHERE is_available = 1 AND "
                     "(next_available IS NULL OR next_available <= datetime(?)) "
                     "LIMIT 1;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, delivery_time, -1, SQLITE_STATIC);
    
    int van_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        van_id = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    
    if (van_id == -1) return false;
    
    // Calculate next available time (assuming 2-hour delivery slots)
    char next_available[64];
    // Add 2 hours to delivery_time
    // This is a simplified version - Soumya might want to add proper time calculation
    sprintf(next_available, "%s+2 hours", delivery_time);
    
    return update_van_availability(van_id, false, next_available);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////

bool get_order_items( Order **orders, int *count){
    sqlite3_stmt *stmt;
    const char *sql="SELECT * FROM orders;";
    if(sqlite3_prepare_v2(db,sql,-1,&stmt,NULL)!=SQLITE_OK){
        return false;
    }
    *count=0;
    while(sqlite3_step(stmt)==SQLITE_ROW){
        (*count)++;
    }
    sqlite3_reset(stmt);
    *orders= (Order *)malloc(sizeof(Order)*(*count));
    if (!*orders){
        sqlite3_finalize(stmt);
        return false;
    }
    int i=0;
    while(sqlite3_step(stmt)==SQLITE_ROW){
        Order *order= &(*orders)[i++];
        order->order_id=sqlite3_column_int(stmt,0);
        order->user_id=sqlite3_column_int(stmt,1);
        order->total_amount=sqlite3_column_double(stmt,2);
        order->status=sqlite3_column_int(stmt,3);
        order->created_at=sqlite3_column_int(stmt,4);
    }
    sqlite3_finalize(stmt);
    return true;
    
}

bool update_payment(int order_id ,PaymentStatus status){
    sqlite3_stmt *stmt;
    const char *sql="UPDATE orders SET payment_status=? WHERE order_id=?;";
    if(sqlite3_prepare_v2(db,sql,-1,&stmt,NULL)!=SQLITE_OK){
        return false;
    }
    sqlite3_bind_int(stmt,1,status);
    sqlite3_bind_int(stmt,2,order_id);
    bool success=sqlite3_step(stmt)==SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool update_delivery(int order_id, DeliveryType delivery_type){
    sqlite3_stmt *stmt;
    const char *sql="UPDATE orders SET delivery_type=? WHERE order_id=?;";
    if(sqlite3_prepare_v2(db,sql,-1,&stmt,NULL)!=SQLITE_OK){
        return false;
    }
    sqlite3_bind_int(stmt,1,delivery_type);
    sqlite3_bind_int(stmt,2,order_id);
    bool success=sqlite3_step(stmt)==SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
bool update_stock_after_order(int user_id) {
    if (!db) {
        return false;
    }

    sqlite3_stmt *stmt, *update_stmt;
    const char *cart_query = "SELECT product_id, quantity FROM cart WHERE user_id = ?;";
    const char *update_stock_query = "UPDATE products SET stock = stock - ? WHERE product_id = ? AND stock >= ?;";

    if (sqlite3_prepare_v2(db, cart_query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing cart query: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    // Prepare the update statement
    if (sqlite3_prepare_v2(db, update_stock_query, -1, &update_stmt, NULL) != SQLITE_OK) {
        printf("Error preparing stock update query: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    bool success = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int product_id = sqlite3_column_int(stmt, 0);
        int quantity = sqlite3_column_int(stmt, 1);

        // Bind values for stock update
        sqlite3_bind_int(update_stmt, 1, quantity);
        sqlite3_bind_int(update_stmt, 2, product_id);
        sqlite3_bind_int(update_stmt, 3, quantity);

        if (sqlite3_step(update_stmt) != SQLITE_DONE) {
            printf("Error updating stock for product ID %d: %s\n", product_id, sqlite3_errmsg(db));
            success = false;
        }

        sqlite3_reset(update_stmt); // Reset for next iteration
    }

    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_finalize(update_stmt);

    return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  Function to execute a SQL query based on user input
bool run_sql_query(sqlite3 *db) {
    if (db == NULL) {
        printf("Database is not initialized.\n");
        return false;
    }

    char sql_query[1024];

    // Loop to allow multiple queries until user types 'exit'
    while (true) {
        printf("\nEnter your SQL query (or type 'exit' to quit):\n");
        fgets(sql_query, sizeof(sql_query), stdin);

        // Remove trailing newline character if present
        sql_query[strcspn(sql_query, "\n")] = 0;

        // Check if user wants to exit
        if (strcmp(sql_query, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);

       //the errors
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
            continue; // Skip to the next query input
        }

        int column_count = sqlite3_column_count(stmt);
        int step_result;

        // Fetch and print results if it's a SELECT query
        if (column_count > 0) {
            printf("\nQuery Results:\n");
            while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW) {
                for (int i = 0; i < column_count; i++) {
                    const char *column_name = sqlite3_column_name(stmt, i);
                    const char *column_value = (const char *)sqlite3_column_text(stmt, i);
                    printf("%s: %s\t", column_name, column_value ? column_value : "NULL");
                }
                printf("\n");
            }
        }

        if (step_result != SQLITE_DONE) {
            fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
        } else {
            printf("Query executed successfully.\n");
        }

        sqlite3_finalize(stmt);
    }

    return true;
}

bool validate_cart_stock(int user_id) {
    if (!db) {
        return false;
    }

    sqlite3_stmt *stmt;
    const char *sql = "SELECT c.product_id, c.quantity, p.stock, p.name "
                      "FROM cart c "
                      "JOIN products p ON c.product_id = p.product_id "
                      "WHERE c.user_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing stock validation query: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    bool is_valid = true;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int product_id = sqlite3_column_int(stmt, 0);
        int quantity = sqlite3_column_int(stmt, 1);
        int stock = sqlite3_column_int(stmt, 2);
        const unsigned char *name = sqlite3_column_text(stmt, 3);

        if (quantity > stock) {
            printf("Product '%s' (ID: %d) exceeds available stock. Available: %d, In Cart: %d\n",
                   name, product_id, stock, quantity);
            is_valid = false;
        }
    }

    sqlite3_finalize(stmt);
    return is_valid;
}

bool clear_cart_after_order(int user_id) {
    if (!db) {
        return false;
    }

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM cart WHERE user_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing clear cart query: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error clearing cart: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//The coupons functions
bool apply_coupon(int coupon_code , float *total_price){
 sqlite3_stmt *stmt;
    const char *sql = "SELECT discount_value AND min_order_value FROM coupons WHERE code  = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing coupon applied  query 1: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, coupon_code);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        printf("could not find coupon 2: \n");
        sqlite3_finalize(stmt);
        return false;
    }
    else{
        float discount = sqlite3_column_double(stmt, 0);
        float min_order_value = sqlite3_column_double(stmt, 1);
        sqlite3_finalize(stmt);
        if (*total_price >= min_order_value) {
            *total_price -= discount;
            return true;
        }
        else {

            printf("Minimum order value %.2f not met for the applied coupon.\n", min_order_value);
            return false;
        }
             
    }

}


bool asking_coupon(float *total_price){
    char choice;
    printf("Do you want to apply a coupon? (y/n): ");
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y') {

        int coupon_code;
        printf("Enter the coupon code: ");
        scanf("%d", &coupon_code);
        if (!apply_coupon(coupon_code,total_price)) {
            return false;
        }

        return true;
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool remove_address(int user_id, int address_id) {
    if (!db) {
        return false;
    }

    sqlite3_stmt *stmt; 
    const char *sql = "DELETE FROM addresses WHERE user_id = ? AND address_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing address removal query: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, address_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error removing address: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    printf("Address removed successfully.\n");
    return true;
}





bool check_if_in_cart(int product_id,int user_id){
    if(!db){
        return false;
    }
    char *sql = "SELECT * FROM cart WHERE product_id = ? AND user_id = ?";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, product_id);
    sqlite3_bind_int(stmt, 2, user_id);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (result == SQLITE_ROW) {
        return true;
    } else {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Remember to make the Latex template for the bill
int generate_bill(int user_id) {
    if (!db) {
        printf("Error: Database connection is not initialized.\n");
        return 10;
    }

    sqlite3_stmt *stmt;

    // Updated query to include is_delayed status
    const char *user_query =
        "SELECT users.username, users.email, users.phone, addresses.address_line, "
        "addresses.city, addresses.postal_code, addresses.country "
        "FROM users "
        "JOIN addresses ON users.user_id = addresses.user_id "
        "WHERE users.user_id = ?;";

    // Updated order query to include is_delayed and delivery_time
    const char *order_query =
        "SELECT orders.order_id, orders.total_amount, orders.created_at, "
        "orders.delivery_type, orders.payment_status, orders.is_delayed, "
        "orders.delivery_time, orders.address_id "
        "FROM orders "
        "WHERE orders.user_id = ? AND orders.bill_status = 0 "
        "ORDER BY orders.created_at DESC LIMIT 1;";

    const char *products_query =
        "SELECT products.name, products.price, cart.quantity, "
        "(products.price * cart.quantity) AS total_price "
        "FROM cart "
        "JOIN products ON cart.product_id = products.product_id "
        "WHERE cart.user_id = ?;";

    //  // Query to insert a new bill into the bills table
    //  const char *insert_bill_query =
    //      "INSERT INTO bills (order_id, user_id) VALUES (?, ?);";
    //  // Query to update the order's bill_status to 1 after generating the bill
    //  const char *update_order_query =
    //      "UPDATE orders SET bill_status = 1 WHERE order_id = ?;";

   // Print bill header with formatting
    printf("\n");
    printf("========================================\n");
    printf("              SALES BILL                \n");
    printf("========================================\n");

    // Fetch and print user details
    if (sqlite3_prepare_v2(db, user_query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing user query: %s\n", sqlite3_errmsg(db));
        return 20;
    }
    sqlite3_bind_int(stmt, 1, user_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *username = (const char *)sqlite3_column_text(stmt, 0);
        const char *email = (const char *)sqlite3_column_text(stmt, 1);
        const char *phone = (const char *)sqlite3_column_text(stmt, 2);
        const char *address = (const char *)sqlite3_column_text(stmt, 3);
        const char *city = (const char *)sqlite3_column_text(stmt, 4);
        const char *postal_code = (const char *)sqlite3_column_text(stmt, 5);
        const char *country = (const char *)sqlite3_column_text(stmt, 6);

        printf("\nCUSTOMER DETAILS:\n");
        printf("----------------------------------------\n");
        printf("Name   : %s\n", username);
        printf("Email  : %s\n", email);
        printf("Phone  : %s\n", phone);
        printf("Address: %s\n", address);
        printf("         %s, %s\n", city, postal_code);
        printf("         %s\n", country);
    } else {
        printf("Error: User details not found.\n");
        sqlite3_finalize(stmt);
        return 25;
    }
    sqlite3_finalize(stmt);

    // Fetch and print order details
    if (sqlite3_prepare_v2(db, order_query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing order query: %s\n", sqlite3_errmsg(db));
        return 30;
    }
    sqlite3_bind_int(stmt, 1, user_id);

    int order_id ;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        order_id = sqlite3_column_int(stmt, 0);
        double total_amount = sqlite3_column_double(stmt, 1);
        const char *created_at = (const char *)sqlite3_column_text(stmt, 2);
        int delivery_type = sqlite3_column_int(stmt, 3);
        int payment_status = sqlite3_column_int(stmt, 4);
        int is_delayed = sqlite3_column_int(stmt, 5);
        const char *delivery_time = (const char *)sqlite3_column_text(stmt, 6);

        printf("\nORDER DETAILS:\n");
        printf("----------------------------------------\n");
        printf("Order ID     : %d\n", order_id);
        printf("Order Date   : %s\n", created_at);
        printf("Delivery Type: %s\n", (delivery_type == DELIVERY_HOME) ? "Home Delivery" : "Takeaway");
        
        if (delivery_type == DELIVERY_HOME) {
            printf("Delivery Time: %s\n", delivery_time ? delivery_time : "Not specified");
            if (is_delayed) {
                printf("\n!!! IMPORTANT DELIVERY NOTICE !!!\n");
                printf("Due to high demand, your delivery may be delayed\n");
                printf("by 1-2 hours from the scheduled time slot.\n");
                printf("We apologize for any inconvenience caused.\n");
            }
        }
        
        printf("Payment Status: %s\n", (payment_status == PAYMENT_PAID) ? "Paid" : "Unpaid");
    } 
    // else {
    //     printf("Error: No pending bills found for the user.\n");
    //     sqlite3_finalize(stmt);
    //     return 40;
    // }
    sqlite3_finalize(stmt);

    // Fetch and print product details
    if (sqlite3_prepare_v2(db, products_query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing products query: %s\n", sqlite3_errmsg(db));
        return 50;
    }
    sqlite3_bind_int(stmt, 1, user_id);

    printf("\nORDERED ITEMS:\n");
    printf("----------------------------------------\n");
    printf("%-30s %-10s %-10s %-10s\n", "Product", "Price", "Quantity", "Total");
    printf("----------------------------------------\n");

    double grand_total = 0.0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *product_name = (const char *)sqlite3_column_text(stmt, 0);
        double price = sqlite3_column_double(stmt, 1);
        int quantity = sqlite3_column_int(stmt, 2);
        double total_price = sqlite3_column_double(stmt, 3);
        grand_total += total_price;

        printf("%-30s $%-9.2f %-10d $%-9.2f\n", 
               product_name, price, quantity, total_price);
    }
    sqlite3_finalize(stmt);

    // Print totals
    printf("----------------------------------------\n");
    printf("%-52s $%-9.2f\n", "Grand Total:", grand_total);
    printf("========================================\n");

    // Insert the bill record
    const char *insert_bill_query =
        "INSERT INTO bills (order_id, user_id) VALUES (?, ?);";
    
    if (sqlite3_prepare_v2(db, insert_bill_query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing insert bill query: %s\n", sqlite3_errmsg(db));
        return 60;
    }
    
    sqlite3_bind_int(stmt, 1, order_id);
    sqlite3_bind_int(stmt, 2, user_id);
    //sqlite3_bind_double(stmt, 3, grand_total);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error inserting bill: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);

    // Update order's bill_status
    const char *update_order_query =
        "UPDATE orders SET bill_status = 1 WHERE order_id = ?;";
    
    if (sqlite3_prepare_v2(db, update_order_query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing update order query: %s\n", sqlite3_errmsg(db));
        return 70;
    }
    
    sqlite3_bind_int(stmt, 1, order_id);

    

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error updating order bill_status: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);

    printf("\nThank you for your purchase!\n");
    printf("========================================\n\n");
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////

// Coupons Stuff
bool add_to_coupon(int coupon_code ,float minimum_order_value , float discount){

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO coupons (coupon_code, min_order_value, discount_value) VALUES (?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing coupon applied  query 1: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, coupon_code);
    sqlite3_bind_double(stmt, 2, minimum_order_value);
    sqlite3_bind_double(stmt, 3, discount);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}
bool apply_add_coupon(){
    printf("do you want to add any coupon ? (y/n): ");
    char choice;
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y') {
        int coupon_code;
        float minimum_order_value;
        float discount;
        printf("Enter the coupon code: ");
        scanf("%d", &coupon_code);
        printf("Enter the minimum order value: ");
        scanf("%f", &minimum_order_value);
        printf("Enter the discount value: ");
        scanf("%f", &discount);
        return add_to_coupon(coupon_code,minimum_order_value,discount);
    }
    return false;
}

bool remove_coupon(int coupon_code){

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM coupons WHERE code = ?;";    

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing coupon applied  query 1: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, coupon_code);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success; 
}

bool apply_delete_coupon(){
    printf("do you want to delete any coupon ? (y/n): ");
    char choice;
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y') {
        int coupon_code;
        printf("Enter the coupon code: ");
        scanf("%d", &coupon_code);
        return remove_coupon(coupon_code);
    }
    return false;
}

// Function to display available coupons for a given user_id
int display_available_coupons(sqlite3 *db, int user_id) {
    sqlite3_stmt *stmt;
    const char *sql = 
        "SELECT coupon_id, code, discount_type, discount_value, "
        "min_order_value, start_date, end_date, usage_limit, usage_count "
        "FROM coupons "
        "WHERE is_active = 1 "
        "AND (usage_count < usage_limit OR usage_limit IS NULL) "
        "AND (start_date IS NULL OR start_date <= CURRENT_TIMESTAMP) "
        "AND (end_date IS NULL OR end_date >= CURRENT_TIMESTAMP)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    
    printf("Available Coupons:\n");
    printf("--------------------------------\n");
    
    int coupon_count = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int coupon_id = sqlite3_column_int(stmt, 0);
        int code = sqlite3_column_int(stmt, 1);
        const char *discount_type = (const char *)sqlite3_column_text(stmt, 2);
        double discount_value = sqlite3_column_double(stmt, 3);
        double min_order_value = sqlite3_column_double(stmt, 4);
        const char *start_date = (const char *)sqlite3_column_text(stmt, 5);
        const char *end_date = (const char *)sqlite3_column_text(stmt, 6);
        int usage_limit = sqlite3_column_int(stmt, 7);
        int usage_count = sqlite3_column_int(stmt, 8);
        
        printf("Coupon ID: %d\n", coupon_id);
        printf("Code: %d\n", code);
        printf("Discount Type: %s\n", discount_type);
        printf("Discount Value: %.2f\n", discount_value);
        printf("Minimum Order Value: %.2f\n", min_order_value);
        printf("Start Date: %s\n", start_date ? start_date : "No Restriction");
        printf("End Date: %s\n", end_date ? end_date : "No Restriction");
        printf("Usage Limit: %d\n", usage_limit);
        printf("Current Usage Count: %d\n", usage_count);
        printf("--------------------------------\n");
        
        coupon_count++;
    }
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    
    if (coupon_count == 0) {
        printf("No available coupons found.\n");
    }
    
    return SQLITE_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////


