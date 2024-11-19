
// // ui.h
// #ifndef GTK4_AUTH_UI_H
// #define GTK4_AUTH_UI_H

// #include "app.h"

// void create_login_page(AuthApp *app);
// void create_register_page(AuthApp *app);
// void show_message_dialog(GtkWindow *parent, const char *message);

// #endif


// ui.h
// #ifndef UI_H
// #define UI_H

// #include "app.h"

// void create_login_page(AuthApp *app);
// void create_register_page(AuthApp *app);
// void show_message_dialog(GtkWindow *parent, const char *message);

// #endif // UI_




// ui.h
#ifndef GTK4_AUTH_UI_H
#define GTK4_AUTH_UI_H

#include "app.h"

/**
 * @brief Create and setup the login page
 * @param app Pointer to the main application structure
 */
void create_login_page(AuthApp *app);

/**
 * @brief Create and setup the registration page
 * @param app Pointer to the main application structure
 */
void create_register_page(AuthApp *app);

/**
 * @brief Display a message dialog
 * @param parent Parent window for the dialog
 * @param message Message to display
 */
void show_message_dialog(GtkWindow *parent, const char *message);

#endif // GTK4_AUTH_UI_H
