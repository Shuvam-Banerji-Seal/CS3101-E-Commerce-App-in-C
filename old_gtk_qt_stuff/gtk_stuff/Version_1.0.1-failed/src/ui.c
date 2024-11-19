// ui.c
#include "headers/ui.h"
#include "headers/database.h"
#include "headers/auth.h"
#include <adwaita.h>

// UI Animation duration in milliseconds
#define ANIMATION_DURATION 200

static void apply_entry_animation(GtkWidget *widget) {
    gtk_widget_add_css_class(widget, "animate-entry");
    g_timeout_add(ANIMATION_DURATION, (GSourceFunc)gtk_widget_remove_css_class, 
                 g_object_ref(widget));
}

static void on_entry_activated(GtkEntry *entry, AuthApp *app) {
    // Auto-focus next entry or trigger action when Enter is pressed
    if (entry == GTK_ENTRY(app->username_entry)) {
        gtk_widget_grab_focus(app->password_entry);
    } else if (entry == GTK_ENTRY(app->password_entry)) {
        on_login_clicked(NULL, app);
    }
}

static void on_login_clicked(GtkButton *button G_GNUC_UNUSED, AuthApp *app) {
    const char *username = gtk_editable_get_text(GTK_EDITABLE(app->username_entry));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(app->password_entry));
    
    // Add loading state
    gtk_widget_set_sensitive(app->login_box, FALSE);
    gtk_widget_add_css_class(app->login_button, "loading");
    
    AuthResult result = authenticate_user(username, password);
    
    // Reset loading state
    gtk_widget_set_sensitive(app->login_box, TRUE);
    gtk_widget_remove_css_class(app->login_button, "loading");
    
    switch (result) {
        case AUTH_SUCCESS:
            gtk_widget_add_css_class(app->login_message_label, "success");
            gtk_label_set_text(GTK_LABEL(app->login_message_label), "Login successful!");
            // Animate transition to main app
            gtk_widget_add_css_class(app->login_box, "fade-out");
            g_timeout_add(ANIMATION_DURATION, (GSourceFunc)switch_to_main_view, app);
            break;
            
        case AUTH_ACCOUNT_LOCKED:
            gtk_widget_add_css_class(app->login_message_label, "warning");
            gtk_label_set_text(GTK_LABEL(app->login_message_label), 
                             "Account is temporarily locked. Please try again later.");
            apply_entry_animation(app->login_message_label);
            break;
            
        default:
            gtk_widget_add_css_class(app->login_message_label, "error");
            gtk_label_set_text(GTK_LABEL(app->login_message_label), 
                             "Invalid username or password!");
            apply_entry_animation(app->login_message_label);
            gtk_widget_add_css_class(app->password_entry, "error");
            break;
    }
}

static void on_register_clicked(GtkButton *button G_GNUC_UNUSED, AuthApp *app) {
    const char *username = gtk_editable_get_text(GTK_EDITABLE(app->register_username_entry));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(app->register_password_entry));
    const char *email = gtk_editable_get_text(GTK_EDITABLE(app->register_email_entry));
    
    if (!validate_registration_fields(app)) {
        return;
    }
    
    // Add loading state
    gtk_widget_set_sensitive(app->register_box, FALSE);
    gtk_widget_add_css_class(app->register_button, "loading");
    
    RegistrationData reg_data = {
        .username = username,
        .password = password,
        .email = email,
        .role = ROLE_USER
    };
    
    AuthResult result = register_user(&reg_data);
    
    // Reset loading state
    gtk_widget_set_sensitive(app->register_box, TRUE);
    gtk_widget_remove_css_class(app->register_button, "loading");
    
    handle_registration_result(app, result);
}

static gboolean validate_registration_fields(AuthApp *app) {
    gboolean valid = TRUE;
    
    // Clear previous error states
    gtk_widget_remove_css_class(app->register_username_entry, "error");
    gtk_widget_remove_css_class(app->register_password_entry, "error");
    gtk_widget_remove_css_class(app->register_email_entry, "error");
    
    const char *username = gtk_editable_get_text(GTK_EDITABLE(app->register_username_entry));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(app->register_password_entry));
    const char *email = gtk_editable_get_text(GTK_EDITABLE(app->register_email_entry));
    
    if (strlen(username) < 3) {
        gtk_widget_add_css_class(app->register_username_entry, "error");
        show_field_error(app, "username-error", "Username must be at least 3 characters");
        valid = FALSE;
    }
    
    if (!is_valid_password(password)) {
        gtk_widget_add_css_class(app->register_password_entry, "error");
        show_field_error(app, "password-error", 
                        "Password must be at least 8 characters with numbers and symbols");
        valid = FALSE;
    }
    
    if (!is_valid_email(email)) {
        gtk_widget_add_css_class(app->register_email_entry, "error");
        show_field_error(app, "email-error", "Please enter a valid email address");
        valid = FALSE;
    }
    
    return valid;
}

void create_login_page(AuthApp *app) {
    // Create responsive container
    app->login_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_add_css_class(app->login_box, "auth-container");
    
    // Create header with logo and title
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *logo = gtk_image_new_from_resource("/com/example/auth/logo.png");
    GtkWidget *title = gtk_label_new("Welcome Back");
    gtk_widget_add_css_class(title, "auth-title");
    
    gtk_box_append(GTK_BOX(header_box), logo);
    gtk_box_append(GTK_BOX(header_box), title);
    
    // Create form container
    GtkWidget *form_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_add_css_class(form_box, "auth-form");
    
    // Username field with icon
    GtkWidget *username_box = create_input_with_icon("user-symbolic", "Username");
    app->username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->username_entry), "Enter your username");
    gtk_box_append(GTK_BOX(username_box), app->username_entry);
    
    // Password field with icon and visibility toggle
    GtkWidget *password_box = create_input_with_icon("lock-symbolic", "Password");
    app->password_entry = gtk_password_entry_new();
    gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(app->password_entry), TRUE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->password_entry), "Enter your password");
    gtk_box_append(GTK_BOX(password_box), app->password_entry);
    
    // Remember me and forgot password row
    GtkWidget *options_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(options_box, "login-options");
    
    app->remember_check = gtk_check_button_new_with_label("Remember me");
    GtkWidget *forgot_link = gtk_link_button_new_with_label("", "Forgot password?");
    
    gtk_box_append(GTK_BOX(options_box), app->remember_check);
    gtk_box_append(GTK_BOX(options_box), forgot_link);
    
    // Login button
    app->login_button = gtk_button_new_with_label("Login");
    gtk_widget_add_css_class(app->login_button, "suggested-action");
    gtk_widget_add_css_class(app->login_button, "auth-button");
    
    // Message label for errors/success
    app->login_message_label = gtk_label_new("");
    gtk_widget_add_css_class(app->login_message_label, "auth-message");
    
    // Social login options
    GtkWidget *social_box = create_social_login_buttons();
    
    // Register link
    GtkWidget *register_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_add_css_class(register_box, "register-prompt");
    
    GtkWidget *register_label = gtk_label_new("Don't have an account?");
    GtkWidget *register_button = gtk_button_new_with_label("Sign up");
    gtk_widget_add_css_class(register_button, "link-button");
    
    gtk_box_append(GTK_BOX(register_box), register_label);
    gtk_box_append(GTK_BOX(register_box), register_button);
    
    // Add all elements to form
    gtk_box_append(GTK_BOX(form_box), username_box);
    gtk_box_append(GTK_BOX(form_box), password_box);
    gtk_box_append(GTK_BOX(form_box), options_box);
    gtk_box_append(GTK_BOX(form_box), app->login_button);
    gtk_box_append(GTK_BOX(form_box), app->login_message_label);
    gtk_box_append(GTK_BOX(form_box), social_box);
    gtk_box_append(GTK_BOX(form_box), register_box);
    
    // Add everything to main container
    gtk_box_append(GTK_BOX(app->login_box), header_box);
    gtk_box_append(GTK_BOX(app->login_box), form_box);
    
    // Connect signals
    g_signal_connect(app->username_entry, "activate", G_CALLBACK(on_entry_activated), app);
    g_signal_connect(app->password_entry, "activate", G_CALLBACK(on_entry_activated), app);
    g_signal_connect(app->login_button, "clicked", G_CALLBACK(on_login_clicked), app);
    g_signal_connect(register_button, "clicked", G_CALLBACK(switch_to_page), "register");
    
    gtk_stack_add_named(GTK_STACK(app->stack), app->login_box, "login");
}

static GtkWidget *create_input_with_icon(const char *icon_name, const char *label_text) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(box, "input-container");
    
    GtkWidget *icon = gtk_image_new_from_icon_name(icon_name);
    gtk_widget_add_css_class(icon, "input-icon");
    
    GtkWidget *label = gtk_label_new(label_text);
    gtk_widget_add_css_class(label, "input-label");
    
    gtk_box_append(GTK_BOX(box), icon);
    gtk_box_append(GTK_BOX(box), label);
    
    return box;
}

static GtkWidget *create_social_login_buttons(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_add_css_class(box, "social-login");
    
    GtkWidget *divider = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(divider, "divider");
    
    GtkWidget *left_line = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *or_label = gtk_label_new("or continue with");
    GtkWidget *right_line = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    
    gtk_box_append(GTK_BOX(divider), left_line);
    gtk_box_append(GTK_BOX(divider), or_label);
    gtk_box_append(GTK_BOX(divider), right_line);
    
    GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(buttons_box, "social-buttons");
    
    const char *providers[] = {"google", "github", "apple", NULL};
    for (int i = 0; providers[i]; i++) {
        GtkWidget *button = gtk_button_new();
        gtk_button_set_icon_name(GTK_BUTTON(button), providers[i]);
        gtk_widget_add_css_class(button, "social-button");
        gtk_box_append(GTK_BOX(buttons_box), button);
    }
    
    gtk_box_append(GTK_BOX(box), divider);
    gtk_box_append(GTK_BOX(box), buttons_box);
    
    return box;
}