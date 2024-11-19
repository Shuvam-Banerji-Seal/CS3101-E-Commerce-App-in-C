// database.h

#ifndef GTK4_ECOMMERCE_DATABASE_H
#define GTK4_ECOMMERCE_DATABASE_H
#include <sqlite3.h>
#include <stdbool.h>
#include <time.h>

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
    char *login_id;
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
    char *address_line;
    char *city;
    char *postal_code;
    char *country;
} Address;

typedef struct {
    int product_id;
    char *name;
    char *description;
    double price;
    int stock;
    int category_id;
    time_t created_at;
    time_t updated_at;
    bool is_active;
} Product;

typedef struct {
    int category_id;
    char *name;
    char *description;
    int parent_id;
} Category;

typedef struct {
    int cart_id;
    int user_id;
    int product_id;
    int quantity;
    time_t added_at;
} Cart;

typedef struct {
    int order_id;
    int user_id;
    OrderStatus status;
    PaymentStatus payment_status;
    DeliveryType delivery_type;
    int address_id;
    char *delivery_time;
    double total_amount;
    time_t created_at;
    time_t updated_at;
} Order;

typedef struct {
    int payment_id;
    int order_id;
    double amount;
    PaymentMethod method;
    char *status;
    char *transaction_id;
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
bool init_database(void);
void cleanup_database(void);

// User management
bool create_user(const User *user);
bool verify_user(const char *login_id, const char *password);
bool get_user(const char *login_id, User *user);
bool update_last_login(int user_id);

// Address management
bool add_address(const Address *address);
bool get_user_addresses(int user_id, Address **addresses, int *count);
bool set_default_address(int user_id, int address_id);

// Product management
bool add_product(const Product *product);
bool update_product(const Product *product);
bool delete_product(int product_id);
bool get_product(int product_id, Product *product);
bool get_all_products(Product **products, int *count);

// Category management
bool add_category(const Category *category);
bool get_category_products(int category_id, Product **products, int *count);

// Cart management
bool add_to_cart(const Cart *cart);
bool update_cart_quantity(int cart_id, int quantity);
bool remove_from_cart(int cart_id);
bool get_user_cart(int user_id, Cart **cart_items, int *count);

// Order management
bool create_order(const Order *order);
bool update_order_status(int order_id, OrderStatus status);
bool add_order_item(int order_id, int product_id, int quantity, double price);
bool get_order_items(int order_id, struct OrderItem **items, int *count);

// Payment management
bool create_payment(const Payment *payment);
bool update_payment_status(int payment_id, const char *status);

// Delivery van management
bool is_van_available(const char *delivery_time);
bool update_van_availability(int van_id, bool is_available, const char *next_available);

// Search functionality
bool log_product_search(int user_id, const char *search_term);
bool search_products(const char *term, Product **results, int *count);

//Close the database
void close_database(void);
#endif // GTK4_AUTH_DATABASE_H






