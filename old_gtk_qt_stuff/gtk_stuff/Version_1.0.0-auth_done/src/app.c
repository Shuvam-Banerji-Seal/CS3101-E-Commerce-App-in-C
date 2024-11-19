// app.c
#include "headers/app.h"
#include "headers/ui.h"

// void auth_app_activate(GtkApplication *app, gpointer user_data) {
//     AuthApp *auth_app = g_new0(AuthApp, 1);
//     auth_app->app = app;
void auth_app_activate(GtkApplication *app, gpointer user_data G_GNUC_UNUSED) {
    AuthApp *auth_app = g_new0(AuthApp, 1);
    auth_app->app = app;

    // Create main window
    auth_app->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(auth_app->window), "GTK4 Auth App");
    gtk_window_set_default_size(GTK_WINDOW(auth_app->window), 400, 300);

    // Create stack for switching between login and register pages
    auth_app->stack = gtk_stack_new();
    gtk_window_set_child(GTK_WINDOW(auth_app->window), auth_app->stack);

    // Create pages
    create_login_page(auth_app);
    create_register_page(auth_app);

    gtk_window_present(GTK_WINDOW(auth_app->window));
}
