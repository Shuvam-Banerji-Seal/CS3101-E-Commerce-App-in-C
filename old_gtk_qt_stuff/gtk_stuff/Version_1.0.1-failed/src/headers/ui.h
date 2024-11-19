#ifndef GTK4_ECOMMERCE_UI_H
#define GTK4_ECOMMERCE_UI_H

#include <gtk/gtk.h>
#include "app.h"

// Main UI structure
typedef struct {
    // Main containers
    GtkWidget *login_box;
    GtkWidget *register_box;
    GtkWidget *stack;
    
    // Login page widgets
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *login_button;
    GtkWidget *login_message_label;
    GtkWidget *remember_check;
    
    // Register page widgets
    GtkWidget *register_username_entry;
    GtkWidget *register_password_entry;
    GtkWidget *register_email_entry;
    GtkWidget *register_message_label;
    GtkWidget *register_button;
} AuthApp;

// Page creation functions
void create_login_page(AuthApp *app);
void create_register_page(AuthApp *app);

// UI helper functions
void show_message_dialog(GtkWindow *parent, const char *message);
void show_field_error(AuthApp *app, const char *field_id, const char *message);
void switch_to_page(GtkWidget *widget, const char *page_name);

// Internal UI utility functions
GtkWidget *create_input_with_icon(const char *icon_name, const char *label_text);
GtkWidget *create_social_login_buttons(void);
gboolean validate_registration_fields(AuthApp *app);

#endif // GTK4_AUTH_UI_H