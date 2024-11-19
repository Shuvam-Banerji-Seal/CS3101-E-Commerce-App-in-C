#include "headers/splash.h"

static void on_splash_timeout(GtkWidget *window) {
    gtk_window_destroy(GTK_WINDOW(window));
}

static const char *logo_svg = 
"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'>"
"  <circle cx='50' cy='50' r='45' fill='none' stroke='#4a90e2' stroke-width='2'>"
"    <animate attributeName='stroke-dasharray' from='0 283' to='283 283'"
"             dur='1.5s' fill='freeze'/>"
"  </circle>"
"  <text x='50' y='55' text-anchor='middle' font-size='24' fill='#4a90e2'"
"        opacity='0'>"
"    <tween attributeName='opacity' from='0' to='1' dur='1s'"
"           begin='0.5s' fill='freeze'/>"
"    M"
"  </text>"
"</svg>";

void show_splash_screen(GtkApplication *app, void (*callback)(GtkApplication*)) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Master");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    
    // Create SVG widget
    GtkWidget *picture = gtk_picture_new();
    GBytes *bytes = g_bytes_new_static(logo_svg, strlen(logo_svg));
    GdkPaintable *paintable = gdk_paintable_new_from_bytes(bytes, NULL);
    gtk_picture_set_paintable(GTK_PICTURE(picture), paintable);
    
    gtk_window_set_child(GTK_WINDOW(window), picture);
    
    // Set up fade animation
    gtk_widget_set_opacity(window, 0.0);
    gtk_widget_add_css_class(window, "splash-window");
    
    gtk_widget_show(window);
    
    // Fade in
    GtkPropertyAnimation *anim = gtk_property_animation_new("opacity", window, 
        gtk_widget_get_opacity(window), 1.0, 750);
    gtk_widget_add_tick_callback(window, (GtkTickCallback)gtk_widget_queue_draw, NULL, NULL);
    gtk_animation_play(GTK_ANIMATION(anim));
    
    // Schedule fade out and destruction
    g_timeout_add(2500, (GSourceFunc)on_splash_timeout, window);
    g_timeout_add(2600, (GSourceFunc)callback, app);
}
