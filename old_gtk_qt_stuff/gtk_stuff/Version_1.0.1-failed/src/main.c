


// #include "headers/app.h"
// #include "headers/database.h"
// #include "headers/auth.h"
// #include <gtk/gtk.h>
// #include <adwaita.h>

// static void load_css(void) {
//     GtkCssProvider *provider = gtk_css_provider_new();
//     gtk_css_provider_load_from_resource(provider, "/com/example/gtk4auth/style.css");
//     gtk_style_context_add_provider_for_display(gdk_display_get_default(),
//         GTK_STYLE_PROVIDER(provider),
//         GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
//     g_object_unref(provider);
// }

// // Either remove setup_animations if not used, or use it where needed
// // // If you want to keep it, add it to your widget setup:
// // static void apply_animations(AuthApp *app) {
// //     setup_animations(app->login_box);
// //     setup_animations(app->register_box);
// //     setup_animations(app->username_entry);
// //     setup_animations(app->password_entry);
// // }

   

// int main(int argc, char *argv[]) {
//     GtkApplication *app;
//     int status;

//     // Initialize libadwaita
//     adw_init();
//      if (!init_database()) {
//         g_print("Failed to initialize database\n");
//         return 1;
//     }

//     if (!init_auth()) {
//         cleanup_database();
//         g_print("Failed to initialize auth system\n");
//         return 1;
//     }


//     // Set up database and encryption
//     if (!init_database() || !init_auth()) {
//         g_printerr("Failed to initialize application components\n");
//         return 1;
//     }

//     // // Create application
//     // app = gtk_application_new(APP_ID, G_APPLICATION_FLAGS_NONE);
//     // g_signal_connect(app, "activate", G_CALLBACK(auth_app_activate), NULL);

//      // Create application - Using G_APPLICATION_DEFAULT_FLAGS instead of G_APPLICATION_FLAGS_NONE
//     app = gtk_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
//     g_signal_connect(app, "activate", G_CALLBACK(auth_app_activate), NULL);


//     // Load CSS before running the application
//     load_css();

//     // Run application
//     status = g_application_run(G_APPLICATION(app), argc, argv);

//     // Cleanup
//     g_object_unref(app);
//     cleanup_database();
//     cleanup_auth();

//     return status;
// }

#include "headers/app.h"
#include "headers/database.h"
#include "headers/auth.h"
#include <gtk/gtk.h>
#include <adwaita.h>

// Add this at the top of main.c after the includes
#define APP_ID "com.example.gtk4ecommerce"

static void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/com/example/gtk4auth/style.css");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

int main(int argc, char *argv[]) {
    GtkApplication *app;
    int status;

    // Initialize libadwaita
    adw_init();
    
    if (!init_database()) {
        g_print("Failed to initialize database\n");
        return 1;
    }

    if (!init_auth()) {
        cleanup_database();
        g_print("Failed to initialize auth system\n");
        return 1;
    }

    // Create application
    app = gtk_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
    // Changed auth_app_activate to ecommerce_app_activate to match app.h declaration
    g_signal_connect(app, "activate", G_CALLBACK(ecommerce_app_activate), NULL);

    // Load CSS before running the application
    load_css();

    // Run application
    status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup
    g_object_unref(app);
    cleanup_database();
    cleanup_auth();

    return status;
}