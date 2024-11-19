#ifndef ECOMMERCE_DATABASE_H
#define ECOMMERCE_DATABASE_H
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <crypt.h>
#include <unistd.h>
#include <ctype.h>
#include "tokenizer.c"


#define _XOPEN_SOURCE
//#define _DEFAULT_SOURCE


// Enums for database fields
typedef enum {
    ROLE_ADMIN,
    ROLE_USER
} UserRole;

typedef enum {
    STATUS_PENDING,
    STATUS_SHIPPED,
    STATUS_DELIVERED,
    STATUS_CANCELLED
} OrderStatus;

typedef enum {
    PAYMENT_PAID,
    PAYMENT_UNPAID,
    PAYMENT_REFUNDED
} PaymentStatus;

typedef enum {
    DELIVERY_TAKEAWAY,
    DELIVERY_HOME
} DeliveryType;

typedef enum {
    PAYMENT_CREDIT_CARD,
    PAYMENT_DEBIT_CARD,
    PAYMENT_PAYPAL
} PaymentMethod;

// Structures matching database tables
typedef struct {
    int user_id;
    char *username;
    char *password_hash;
    UserRole role;
    char *email;
    char *phone;
    int address_default;
    time_t created_at;
    time_t last_login;
} User;

typedef struct {
    int address_id;
    int user_id;
    char address_line[300];
    char city[300];
    char  postal_code[300];
    char country[300];
} Address;

// Delivery time slots structure (keep as before)
typedef struct {
    char time_slot[20];
    bool available;
} TimeSlot;

// Available time slots (keep as before)
static const TimeSlot DEFAULT_TIME_SLOTS[] = {
    {"09:00-11:00", true},
    {"11:00-13:00", true},
    {"14:00-16:00", true},
    {"16:00-18:00", true},
    {"18:00-20:00", true}
};
bool is_delivery_delayed = false;
// typedef struct {
//     int product_id;
//     char name[256];
//     char description[255];
//     double price;
//     int stock;
//     int category_id;
//     int64_t created_at;
//     int64_t updated_at;
//     bool is_active;
// } Product;

// typedef struct {
//     int product_id;
//     char name[256];        // Fixed buffer instead of char*
//     char description[1024]; // Fixed buffer instead of char*
//     double price;
//     int stock;
//     char sku[64];          // Added SKU field
//     double discount;       // Added discount field
//     double rating;         // Added rating field
//     int category_id;
//     int64_t created_at;
//     int64_t updated_at;
//     int is_active;
// } Product;

typedef struct {
    int product_id;
    char name[256];        // Fixed buffer instead of char*
    char description[1024]; // Fixed buffer instead of char*
    double price;
    int stock;
    char sku[64];          // Added SKU field
    double discount;       // Added discount field
    double rating;         // Added rating field
    int category_id;
    time_t created_at;
    time_t updated_at;
    int is_active;
} Product;


typedef struct {
    int category_id;
    char name[50];
    char description[200];
    int parent_id;
} Category;

typedef struct {
    int cart_id;//shouldn't be present
    int user_id;
    int product_id;
    int quantity;
    time_t added_at;
} Cart;

typedef struct {
    int order_id;
    int user_id;
    int status;
   PaymentStatus payment_status;
   DeliveryType delivery_type;
   OrderStatus order_status;
    int address_id;
    //time_t delivery_time;
    char* delivery_time;

    double total_amount;
    time_t created_at;
    time_t updated_at;
    bool is_delayed;

} Order;

typedef struct {
    int payment_id;
    int order_id;
    int user_id;
    double amount;
    PaymentMethod method;
    PaymentStatus status;
    char transaction_id[20];
    time_t payment_date;
} Payment;

typedef struct {
    int order_item_id;
    int order_id;
    int product_id;
    int quantity;
    double price;
} OrderItem;

// Database management
bool init_database(void);//done
void cleanup_database(void);

// User management
bool create_user(const char *username, const char *password);//done//for user
//bool verify_user(const char *username, const char *password);//done//for user
bool user_signup(const char *username, const char *hashed_password, const char *email, const char *phone);
bool get_user(const char *username, User *user);
bool update_last_login(int user_id);//dpne//for user
bool verify_user(const char *username, const char *password);//done//for user   
bool update_last_login(int user_id);//done//for user


//admin functions
bool verify_admin(const char *username, const char *password);//done//for admin

// Address management
bool add_address(const Address *address);//done//for user
bool get_user_addresses(int user_id, Address **addresses, int *count);
bool set_default_address(int user_id, int address_id);//done//for user
bool apply_coupon(int coupon_code , float *discount);
// Product management
bool add_product(const Product *product);//done
bool update_product(const Product *product);//done
bool delete_product(int product_id);//done
bool get_product(int product_id, Product *product);//done
bool get_all_products(Product **products, int *count);//done
float total_price(int product_id);//done//for user


// Category management
bool add_category(const Category *category);
bool get_all_categorys(Category **categories, int *count);
bool get_category_products(int category_id, Product **products, int *count);//done//for user
bool update_category(const Category *category,int category_id);//done
bool check_if_in_cart(int product_id,int user_id);

// Cart management
bool add_to_cart(const Cart *cart);//recently defined //for user
bool update_cart_quantity(int product_id, int quantity);//only function that was edited in database.c//for user
bool remove_from_cart(int product_id, int user_id);//needed to compare user_id for verification
bool get_user_cart(int user_id, Cart **cart_items, int *count);//recently added
bool remove_item_from_cart(int product_id, int user_id);//for user



//needed a function to show cart Item to user
//needed  a function to delete a particular element from cart 

// Order management
bool create_order(const Order *order);//for user
bool cart_to_order( Order *order, int user_id);//this function calculates the total amount needed to be paid//for user
bool update_order_status(int order_id, OrderStatus status);
//bool add_order_item(int order_id, int product_id, int quantity, double price);
bool get_order_items( Order **orders, int *count);//only function pending to be created 
bool edit_cart_delete_cart(int quantity ,int user_id ,int product_id);//shall leater find bst place to place the function inside another function 
bool update_payment(int order_id ,PaymentStatus status);//for admin
bool update_delivery(int order_id ,DeliveryType delivery_type);//for admin



// Payment management
bool create_payment(const Payment *payment);//for user
bool update_payment_status(int payment_id, PaymentStatus status);

// Delivery van management
bool is_van_available(const char *delivery_time);
bool update_van_availability(int van_id, bool is_available, const char *next_available);

// Search functionality
bool log_product_search(int user_id, const char *search_term);
// bool search_products(BKNode * root , char *term, Product **results, int *count);

//to be categorized

bool check_admin_exists(sqlite3 *, const char *);
// bool search_products(sqlite3 *db, BKNode *root, const char *search_term, Product **results, int *count);
// bool search_products(sqlite3 *db, const char *search_term, ProductSearchResult **results, int *result_count);
void place_order(int user_id1);


void close_database(void);

#endif 
