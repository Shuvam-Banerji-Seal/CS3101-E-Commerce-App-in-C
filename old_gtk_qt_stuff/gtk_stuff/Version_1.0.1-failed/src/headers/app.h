// app.h
#ifndef GTK4_ECOMMERCE_APP_H
#define GTK4_ECOMMERCE_APP_H

#include <gtk/gtk.h>
#include "auth.h"
#include "database.h"

// Main application structure
typedef struct {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *stack;
    GtkWidget *header_bar;
    User *current_user;
    
    // Login widgets
    GtkWidget *login_page;
    GtkWidget *login_username_entry;
    GtkWidget *login_password_entry;
    GtkWidget *login_message_label;
    
    // Register widgets
    GtkWidget *register_page;
    GtkWidget *register_username_entry;
    GtkWidget *register_password_entry;
    GtkWidget *register_email_entry;
    GtkWidget *register_phone_entry;
    GtkWidget *register_message_label;
    
    // Main shop widgets
    GtkWidget *shop_page;
    GtkWidget *product_list;
    GtkWidget *cart_badge;
    GtkWidget *category_combo;
    
    // Cart widgets
    GtkWidget *cart_page;
    GtkWidget *cart_list;
    GtkWidget *total_label;
    
    // Order widgets
    GtkWidget *order_page;
    GtkWidget *order_list;
    GtkWidget *address_combo;
    
    // Search widgets
    GtkWidget *search_entry;
    GtkWidget *search_results;
} EcommerceApp;

// Function declarations
void ecommerce_app_activate(GtkApplication *app, gpointer user_data);
void cleanup_app(EcommerceApp *ecom_app);

// Page creation functions
void create_login_page(EcommerceApp *app);
void create_register_page(EcommerceApp *app);
void create_shop_page(EcommerceApp *app);
void create_cart_page(EcommerceApp *app);
void create_order_page(EcommerceApp *app);
void create_profile_page(EcommerceApp *app);

// Navigation functions
void switch_to_page(EcommerceApp *app, const char *page_name);
void logout_user(EcommerceApp *app);

// Shop functions
void refresh_product_list(EcommerceApp *app);
// void add_to_cart(EcommerceApp *app, int product_id);

void add_product_to_cart(EcommerceApp *app, int product_id);  // Changed from add_to_cart
void update_cart_badge(EcommerceApp *app);

// Cart functions
void refresh_cart(EcommerceApp *app);
void checkout_cart(EcommerceApp *app);

// Order functions
void create_new_order(EcommerceApp *app);
void refresh_order_list(EcommerceApp *app);

#endif // GTK4_ECOMMERCE_APP_H

