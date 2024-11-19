// ui.c
#include "headers/ui.h"
#include "headers/database.h"
#include "headers/auth.h"
#include <adwaita.h>

static void on_login_clicked(GtkButton *button G_GNUC_UNUSED, AuthApp *app) {
    const char *username = gtk_editable_get_text(GTK_EDITABLE(app->username_entry));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(app->password_entry));

    AuthResult result = authenticate_user(username, password);
    
    switch (result) {
        case AUTH_SUCCESS:
            show_message_dialog(GTK_WINDOW(app->window), "Login successful!");
            break;
        case AUTH_ACCOUNT_LOCKED:
            show_message_dialog(GTK_WINDOW(app->window), "Account is locked. Please try again later.");
            break;
        default:
            show_message_dialog(GTK_WINDOW(app->window), "Invalid username or password!");
            break;
    }
}

static void on_register_clicked(GtkButton *button G_GNUC_UNUSED, AuthApp *app) {
    const char *username = gtk_editable_get_text(GTK_EDITABLE(app->register_username_entry));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(app->register_password_entry));

    // Additional validation to ensure fields aren't empty
    if (strlen(username) == 0 || strlen(password) == 0) {
        show_message_dialog(GTK_WINDOW(app->window), "Username and password cannot be empty!");
        return;
    }

    AuthResult result = register_user(username, password);
    
    switch (result) {
        case AUTH_SUCCESS:
            show_message_dialog(GTK_WINDOW(app->window), "Registration successful!");
            // Clear the input fields after successful registration
            gtk_editable_set_text(GTK_EDITABLE(app->register_username_entry), "");
            gtk_editable_set_text(GTK_EDITABLE(app->register_password_entry), "");
            // Switch back to login page after successful registration
            gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "login");
            break;
        case AUTH_USER_EXISTS:
            show_message_dialog(GTK_WINDOW(app->window), "Username already exists!");
            break;
        case AUTH_WEAK_PASSWORD:
            show_message_dialog(GTK_WINDOW(app->window), 
                "Password must be at least 8 characters and contain uppercase, lowercase, numbers, and special characters.");
            break;
        case AUTH_ERROR:
            show_message_dialog(GTK_WINDOW(app->window), "Registration failed due to a system error!");
            break;
        default:
            show_message_dialog(GTK_WINDOW(app->window), "Registration failed!");
            break;
    }
}

static void switch_to_page(GtkButton *button, gpointer user_data) {
    AuthApp *app = (AuthApp *)user_data;
    const char *page_name = g_object_get_data(G_OBJECT(button), "target-page");
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), page_name);
}

void create_login_page(AuthApp *app) {
    // Create login box
    app->login_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(app->login_box, 20);
    gtk_widget_set_margin_end(app->login_box, 20);
    gtk_widget_set_margin_top(app->login_box, 20);
    gtk_widget_set_margin_bottom(app->login_box, 20);

    // Add title
    GtkWidget *title = gtk_label_new("Login");
    gtk_widget_add_css_class(title, "title-2");
    gtk_box_append(GTK_BOX(app->login_box), title);

    // Username entry
    app->username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->username_entry), "Username");
    gtk_box_append(GTK_BOX(app->login_box), app->username_entry);

    // Password entry
    app->password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(app->password_entry), FALSE);
    gtk_box_append(GTK_BOX(app->login_box), app->password_entry);

    // Login button
    GtkWidget *login_button = gtk_button_new_with_label("Login");
    gtk_widget_add_css_class(login_button, "suggested-action");
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_clicked), app);
    gtk_box_append(GTK_BOX(app->login_box), login_button);

    // Switch to register page button
    GtkWidget *switch_button = gtk_button_new_with_label("Don't have an account? Register");
    g_object_set_data(G_OBJECT(switch_button), "target-page", "register");
    g_signal_connect(switch_button, "clicked", G_CALLBACK(switch_to_page), app);
    gtk_box_append(GTK_BOX(app->login_box), switch_button);

    gtk_stack_add_named(GTK_STACK(app->stack), app->login_box, "login");
}

void create_register_page(AuthApp *app) {
    // Create register box
    app->register_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(app->register_box, 20);
    gtk_widget_set_margin_end(app->register_box, 20);
    gtk_widget_set_margin_top(app->register_box, 20);
    gtk_widget_set_margin_bottom(app->register_box, 20);

    // Add title
    GtkWidget *title = gtk_label_new("Register");
    gtk_widget_add_css_class(title, "title-2");
    gtk_box_append(GTK_BOX(app->register_box), title);

    // Username entry
    app->register_username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->register_username_entry), "Username");
    gtk_box_append(GTK_BOX(app->register_box), app->register_username_entry);

    // Password entry
    app->register_password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->register_password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(app->register_password_entry), FALSE);
    gtk_box_append(GTK_BOX(app->register_box), app->register_password_entry);

    // Register button
    GtkWidget *register_button = gtk_button_new_with_label("Register");
    gtk_widget_add_css_class(register_button, "suggested-action");
    g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_clicked), app);
    gtk_box_append(GTK_BOX(app->register_box), register_button);

    // Switch to login page button
    GtkWidget *switch_button = gtk_button_new_with_label("Already have an account? Login");
    g_object_set_data(G_OBJECT(switch_button), "target-page", "login");
    g_signal_connect(switch_button, "clicked", G_CALLBACK(switch_to_page), app);
    gtk_box_append(GTK_BOX(app->register_box), switch_button);

    gtk_stack_add_named(GTK_STACK(app->stack), app->register_box, "register");
}
// void create_login_page(AuthApp *app) {
//     // Create login box
//     app->login_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
//     gtk_widget_set_margin_start(app->login_box, 20);
//     gtk_widget_set_margin_end(app->login_box, 20);
//     gtk_widget_set_margin_top(app->login_box, 20);
//     gtk_widget_set_margin_bottom(app->login_box, 20);

//     // Add title
//     GtkWidget *title = gtk_label_new("Login");
//     gtk_widget_add_css_class(title, "title-2");
//     gtk_box_append(GTK_BOX(app->login_box), title);

//     // Username entry
//     app->username_entry = gtk_entry_new();
//     gtk_entry_set_placeholder_text(GTK_ENTRY(app->username_entry), "Username");
//     gtk_box_append(GTK_BOX(app->login_box), app->username_entry);

//     // Password entry
//     app->password_entry = gtk_entry_new();
//     gtk_entry_set_placeholder_text(GTK_ENTRY(app->password_entry), "Password");
//     gtk_entry_set_visibility(GTK_ENTRY(app->password_entry), FALSE);
//     gtk_box_append(GTK_BOX(app->login_box), app->password_entry);

//     // Login button
//     GtkWidget *login_button = gtk_button_new_with_label("Login");
//     gtk_widget_add_css_class(login_button, "suggested-action");
//     g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_clicked), app);
//     gtk_box_append(GTK_BOX(app->login_box), login_button);

//     // Switch to register page button
//     GtkWidget *switch_button = gtk_button_new_with_label("Don't have an account? Register");
//     g_signal_connect_swapped(switch_button, "clicked", 
//         G_CALLBACK(gtk_stack_set_visible_child_name), 
//         g_variant_new("s", "register"));
//     gtk_box_append(GTK_BOX(app->login_box), switch_button);

//     gtk_stack_add_named(GTK_STACK(app->stack), app->login_box, "login");
// }

// void create_register_page(AuthApp *app) {
//     // Create register box
//     app->register_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
//     gtk_widget_set_margin_start(app->register_box, 20);
//     gtk_widget_set_margin_end(app->register_box, 20);
//     gtk_widget_set_margin_top(app->register_box, 20);
//     gtk_widget_set_margin_bottom(app->register_box, 20);

//     // Add title
//     GtkWidget *title = gtk_label_new("Register");
//     gtk_widget_add_css_class(title, "title-2");
//     gtk_box_append(GTK_BOX(app->register_box), title);

//     // Username entry
//     app->register_username_entry = gtk_entry_new();
//     gtk_entry_set_placeholder_text(GTK_ENTRY(app->register_username_entry), "Username");
//     gtk_box_append(GTK_BOX(app->register_box), app->register_username_entry);

//     // Password entry
//     app->register_password_entry = gtk_entry_new();
//     gtk_entry_set_placeholder_text(GTK_ENTRY(app->register_password_entry), "Password");
//     gtk_entry_set_visibility(GTK_ENTRY(app->register_password_entry), FALSE);
//     gtk_box_append(GTK_BOX(app->register_box), app->register_password_entry);

//     // Register button
//     GtkWidget *register_button = gtk_button_new_with_label("Register");
//     gtk_widget_add_css_class(register_button, "suggested-action");
//     g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_clicked), app);
//     gtk_box_append(GTK_BOX(app->register_box), register_button);

//     // Switch to login page button
//     GtkWidget *switch_button = gtk_button_new_with_label("Already have an account? Login");
//     g_signal_connect_swapped(switch_button, "clicked",
//         G_CALLBACK(gtk_stack_set_visible_child_name), 
//         g_variant_new("s", "login"));
//     gtk_box_append(GTK_BOX(app->register_box), switch_button);

//     gtk_stack_add_named(GTK_STACK(app->stack), app->register_box, "register");
// }

void show_message_dialog(GtkWindow *parent, const char *message) {
    AdwMessageDialog *dialog = ADW_MESSAGE_DIALOG(
        adw_message_dialog_new(parent, "Message", message));
    
    adw_message_dialog_add_response(dialog, "ok", "OK");
    adw_message_dialog_set_default_response(dialog, "ok");
    
    gtk_window_present(GTK_WINDOW(dialog));
}