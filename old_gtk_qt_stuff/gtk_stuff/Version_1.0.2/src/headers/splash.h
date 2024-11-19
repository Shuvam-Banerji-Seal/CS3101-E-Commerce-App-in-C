// splash.h
#ifndef MASTER_SPLASH_H
#define MASTER_SPLASH_H

#include <gtk/gtk.h>

void show_splash_screen(GtkApplication *app, void (*callback)(GtkApplication*));

#endif