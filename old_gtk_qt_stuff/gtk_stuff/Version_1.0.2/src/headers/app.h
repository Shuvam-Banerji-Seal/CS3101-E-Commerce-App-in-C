// // app.h
// #ifndef GTK4_AUTH_APP_H
// #define GTK4_AUTH_APP_H

// #include <gtk/gtk.h>

// #define APP_ID "com.example.gtk4auth"

// typedef struct {
//     GtkApplication *app;
//     GtkWidget *window;
//     GtkWidget *stack;
//     GtkWidget *login_box;
//     GtkWidget *register_box;
//     GtkWidget *username_entry;
//     GtkWidget *password_entry;
// } AuthApp;

// void auth_app_activate(GtkApplication *app, gpointer user_data);

// #endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// app.h
// #ifndef APP_H
// #define APP_H

// #include <gtk/gtk.h>
// #include <adwaita.h>

// #define APP_ID "com.example.gtk4auth"

// typedef struct {
//     GtkApplication *app;
//     GtkWidget *window;
//     GtkWidget *stack;
//     GtkWidget *login_box;
//     GtkWidget *register_box;
//     GtkWidget *username_entry;
//     GtkWidget *password_entry;
// } AuthApp;

// void auth_app_activate(GtkApplication *app, gpointer user_data);

// #endif // APP_H


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// app.h
#ifndef GTK4_AUTH_APP_H
#define GTK4_AUTH_APP_H

#include <gtk/gtk.h>
#include <adwaita.h>

/**
 * @brief Application ID used for GTK registration
 */
#define APP_ID "com.example.gtk4auth"

/**
 * @brief Main application structure holding UI components
 */
typedef struct {
    GtkApplication *app;        /**< Main GTK application instance */
    GtkWidget *window;          /**< Main application window */
    GtkWidget *stack;           /**< Stack for switching between pages */
    GtkWidget *login_box;       /**< Container for login page */
    GtkWidget *register_box;    /**< Container for registration page */
    GtkWidget *username_entry;  /**< Username input field */
    GtkWidget *password_entry;  /**< Password input field */
    GtkWidget *register_username_entry;
    GtkWidget *register_password_entry;
} AuthApp;

/**
 * @brief GTK application activation callback
 * @param app The GTK application instance
 * @param user_data Additional user data (unused)
 */
void auth_app_activate(GtkApplication *app, gpointer user_data);

#endif // GTK4_AUTH_APP_H


