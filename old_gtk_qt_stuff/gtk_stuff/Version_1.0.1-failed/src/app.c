// app.c
#include "headers/app.h"
#include <stdlib.h>

static void on_login_clicked(GtkButton *button, EcommerceApp *app) {
    const char *username = gtk_editable_get_text(GTK_EDITABLE(app->login_username_entry));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(app->login_password_entry));
    
    User *user = g_new0(User, 1);
    AuthResult result = authenticate_user(username, password, user);
    
    switch (result) {
        case AUTH_SUCCESS:
            app->current_user = user;
            gtk_label_set_text(GTK_LABEL(app->login_message_label), "Login successful!");
            create_shop_page(app);
            switch_to_page(app, "shop");
            update_cart_badge(app);
            break;
            
        case AUTH_ACCOUNT_LOCKED:
            gtk_label_set_text(GTK_LABEL(app->login_message_label), 
                             "Account locked. Please try again later.");
            g_free(user);
            break;
            
        case AUTH_FAILED:
            gtk_label_set_text(GTK_LABEL(app->login_message_label), 
                             "Invalid username or password.");
            g_free(user);
            break;
            
        default:
            gtk_label_set_text(GTK_LABEL(app->login_message_label), 
                             "An error occurred. Please try again.");
            g_free(user);
            break;
    }
}

static void on_register_clicked(GtkButton *button, EcommerceApp *app) {
    RegistrationData reg_data = {
        .login_id = gtk_editable_get_text(GTK_EDITABLE(app->register_username_entry)),
        .password = gtk_editable_get_text(GTK_EDITABLE(app->register_password_entry)),
        .email = gtk_editable_get_text(GTK_EDITABLE(app->register_email_entry)),
        .phone = gtk_editable_get_text(GTK_EDITABLE(app->register_phone_entry)),
        .role = ROLE_USER
    };
    
    AuthResult result = register_user(&reg_data);
    
    switch (result) {
        case AUTH_SUCCESS:
            gtk_label_set_text(GTK_LABEL(app->register_message_label), 
                             "Registration successful! Please login.");
            switch_to_page(app, "login");
            break;
            
        case AUTH_USER_EXISTS:
            gtk_label_set_text(GTK_LABEL(app->register_message_label), 
                             "Username already exists.");
            break;
            
        case AUTH_WEAK_PASSWORD:
            gtk_label_set_text(GTK_LABEL(app->register_message_label), 
                             "Password too weak. Please use at least 8 characters with "
                             "numbers, letters, and symbols.");
            break;
            
        default:
            gtk_label_set_text(GTK_LABEL(app->register_message_label), 
                             "Registration failed. Please try again.");
            break;
    }
}

void create_login_page(EcommerceApp *app) {
    app->login_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(app->login_page, 20);
    gtk_widget_set_margin_end(app->login_page, 20);
    gtk_widget_set_margin_top(app->login_page, 20);
    gtk_widget_set_margin_bottom(app->login_page, 20);
    
    GtkWidget *title = gtk_label_new("Login to Ecommerce");
    gtk_widget_add_css_class(title, "title-2");
    
    app->login_username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->login_username_entry), "Username");
    
    app->login_password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->login_password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(app->login_password_entry), FALSE);
    
    GtkWidget *login_button = gtk_button_new_with_label("Login");
    gtk_widget_add_css_class(login_button, "suggested-action");
    
    GtkWidget *register_button = gtk_button_new_with_label("Create Account");
    
    app->login_message_label = gtk_label_new("");
    gtk_widget_add_css_class(app->login_message_label, "error");
    
    gtk_box_append(GTK_BOX(app->login_page), title);
    gtk_box_append(GTK_BOX(app->login_page), app->login_username_entry);
    gtk_box_append(GTK_BOX(app->login_page), app->login_password_entry);
    gtk_box_append(GTK_BOX(app->login_page), login_button);
    gtk_box_append(GTK_BOX(app->login_page), register_button);
    gtk_box_append(GTK_BOX(app->login_page), app->login_message_label);
    
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_clicked), app);
    g_signal_connect(register_button, "clicked", G_CALLBACK(switch_to_page), "register");
    
    gtk_stack_add_named(GTK_STACK(app->stack), app->login_page, "login");
}

void create_shop_page(EcommerceApp *app) {
    app->shop_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    
    // Create header bar with search and cart
    app->header_bar = gtk_header_bar_new();
    app->search_entry = gtk_search_entry_new();
    gtk_header_bar_pack_start(GTK_HEADER_BAR(app->header_bar), app->search_entry);
    
    GtkWidget *cart_button = gtk_button_new_from_icon_name("cart-symbolic");
    app->cart_badge = gtk_label_new("0");
    gtk_widget_add_css_class(app->cart_badge, "badge");
    gtk_widget_set_parent(app->cart_badge, cart_button);
    
    gtk_header_bar_pack_end(GTK_HEADER_BAR(app->header_bar), cart_button);
    gtk_window_set_titlebar(GTK_WINDOW(app->window), app->header_bar);
    
    // Create category filter
    app->category_combo = gtk_drop_down_new_from_strings((const char *[])
        {"All Categories", "Electronics", "Clothing", "Books", NULL});
    
    // Create scrolled product list
    GtkWidget *scrolled = gtk_scrolled_window_new();
    app->product_list = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), app->product_list);
    
    gtk_box_append(GTK_BOX(app->shop_page), app->category_combo);
    gtk_box_append(GTK_BOX(app->shop_page), scrolled);
    
    gtk_stack_add_named(GTK_STACK(app->stack), app->shop_page, "shop");
    
    // Connect signals
    g_signal_connect(app->search_entry, "search-changed", G_CALLBACK(refresh_product_list), app);
    g_signal_connect(app->category_combo, "notify::selected", G_CALLBACK(refresh_product_list), app);
    g_signal_connect(cart_button, "clicked", G_CALLBACK(switch_to_page), "cart");
    
    refresh_product_list(app);
}

void refresh_product_list(EcommerceApp *app) {
    // Clear existing list
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(app->product_list)) != NULL) {
        gtk_list_box_remove(GTK_LIST_BOX(app->product_list), child);
    }
    
    // Get products based on search and category
    Product *products;
    int count;
    const char *search_term = gtk_editable_get_text(GTK_EDITABLE(app->search_entry));
    
    if (search_term && *search_term) {
        search_products(search_term, &products, &count);
    } else {
        get_all_products(&products, &count);
    }
    
    // Add products to list
    for (int i = 0; i < count; i++) {
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        
        GtkWidget *name_label = gtk_label_new(products[i].name);
        GtkWidget *price_label = gtk_label_new_printf("$%.2f", products[i].price);
        GtkWidget *add_button = gtk_button_new_from_icon_name("list-add-symbolic");
        
        gtk_box_append(GTK_BOX(row), name_label);
        gtk_box_append(GTK_BOX(row), price_label);
        gtk_box_append(GTK_BOX(row), add_button);
        
        gtk_list_box_append(GTK_LIST_BOX(app->product_list), row);
        
        g_signal_connect_swapped(add_button, "clicked", G_CALLBACK(add_to_cart), 
                               GINT_TO_POINTER(products[i].product_id));
    }
    
    free(products);
}

void ecommerce_app_activate(GtkApplication *app, gpointer user_data) {
    EcommerceApp *ecom_app = g_new0(EcommerceApp, 1);
    ecom_app->app = app;
    
    // Initialize auth and database
    if (!init_auth()) {
        g_error("Failed to initialize authentication system");
        return;
    }
    
    // Create main window
    ecom_app->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(ecom_app->window), "GTK4 Ecommerce");
    gtk_window_set_default_size(GTK_WINDOW(ecom_app->window), 800, 600);
    
    // Create stack for pages
    ecom_app->stack = gtk_stack_new();
    gtk_window_set_child(GTK_WINDOW(ecom_app->window), ecom_app->stack);
    
    // Create initial pages
    create_login_page(ecom_app);
    create_register_page(ecom_app);
    
    // Show window
    gtk_window_present(GTK_WINDOW(ecom_app->window));
}

void cleanup_app(EcommerceApp *ecom_app) {
    if (ecom_app) {
        if (ecom_app->current_user) {
            g_free(ecom_app->current_user);
        }
        cleanup_auth();
        g_free(ecom_app);
    }
}



void create_cart_page(EcommerceApp *app) {
    app->cart_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(app->cart_page, 20);
    gtk_widget_set_margin_end(app->cart_page, 20);
    gtk_widget_set_margin_top(app->cart_page, 20);
    
    GtkWidget *title = gtk_label_new("Shopping Cart");
    gtk_widget_add_css_class(title, "title-2");
    
    // Create scrolled window for cart items
    GtkWidget *scrolled = gtk_scrolled_window_new();
    app->cart_list = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), app->cart_list);
    
    // Total and checkout section
    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    app->total_label = gtk_label_new("Total: $0.00");
    GtkWidget *checkout_button = gtk_button_new_with_label("Checkout");
    gtk_widget_add_css_class(checkout_button, "suggested-action");
    
    gtk_box_append(GTK_BOX(bottom_box), app->total_label);
    gtk_box_append(GTK_BOX(bottom_box), checkout_button);
    
    gtk_box_append(GTK_BOX(app->cart_page), title);
    gtk_box_append(GTK_BOX(app->cart_page), scrolled);
    gtk_box_append(GTK_BOX(app->cart_page), bottom_box);
    
    gtk_stack_add_named(GTK_STACK(app->stack), app->cart_page, "cart");
    
    g_signal_connect(checkout_button, "clicked", G_CALLBACK(checkout_cart), app);
    
    refresh_cart(app);
}

void create_order_page(EcommerceApp *app) {
    app->order_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(app->order_page, 20);
    gtk_widget_set_margin_end(app->order_page, 20);
    gtk_widget_set_margin_top(app->order_page, 20);
    
    GtkWidget *title = gtk_label_new("My Orders");
    gtk_widget_add_css_class(title, "title-2");
    
    // Address selection
    GtkWidget *address_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *address_label = gtk_label_new("Delivery Address:");
    app->address_combo = gtk_drop_down_new_from_strings(NULL); // Will be populated
    
    gtk_box_append(GTK_BOX(address_box), address_label);
    gtk_box_append(GTK_BOX(address_box), app->address_combo);
    
    // Orders list
    GtkWidget *scrolled = gtk_scrolled_window_new();
    app->order_list = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), app->order_list);
    
    gtk_box_append(GTK_BOX(app->order_page), title);
    gtk_box_append(GTK_BOX(app->order_page), address_box);
    gtk_box_append(GTK_BOX(app->order_page), scrolled);
    
    gtk_stack_add_named(GTK_STACK(app->stack), app->order_page, "orders");
    
    refresh_order_list(app);
}

void create_profile_page(EcommerceApp *app) {
    app->profile_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(app->profile_page, 20);
    gtk_widget_set_margin_end(app->profile_page, 20);
    gtk_widget_set_margin_top(app->profile_page, 20);
    
    GtkWidget *title = gtk_label_new("My Profile");
    gtk_widget_add_css_class(title, "title-2");
    
    // User info section
    GtkWidget *info_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(info_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(info_grid), 10);
    
    // Add user information fields
    int row = 0;
    add_profile_field(info_grid, "Username:", app->current_user->login_id, row++);
    add_profile_field(info_grid, "Email:", app->current_user->email, row++);
    add_profile_field(info_grid, "Phone:", app->current_user->phone, row++);
    
    // Edit profile button
    GtkWidget *edit_button = gtk_button_new_with_label("Edit Profile");
    gtk_widget_add_css_class(edit_button, "suggested-action");
    
    // Logout button
    GtkWidget *logout_button = gtk_button_new_with_label("Logout");
    gtk_widget_add_css_class(logout_button, "destructive-action");
    
    gtk_box_append(GTK_BOX(app->profile_page), title);
    gtk_box_append(GTK_BOX(app->profile_page), info_grid);
    gtk_box_append(GTK_BOX(app->profile_page), edit_button);
    gtk_box_append(GTK_BOX(app->profile_page), logout_button);
    
    gtk_stack_add_named(GTK_STACK(app->stack), app->profile_page, "profile");
    
    g_signal_connect(logout_button, "clicked", G_CALLBACK(logout_user), app);
    g_signal_connect(edit_button, "clicked", G_CALLBACK(show_edit_profile_dialog), app);
}

// Cart management functions
void refresh_cart(EcommerceApp *app) {
    // Clear existing items
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(app->cart_list)) != NULL) {
        gtk_list_box_remove(GTK_LIST_BOX(app->cart_list), child);
    }
    
    CartItem *items;
    int count;
    double total = 0;
    
    if (get_cart_items(app->current_user->user_id, &items, &count) == DB_SUCCESS) {
        for (int i = 0; i < count; i++) {
            GtkWidget *row = create_cart_item_row(&items[i], app);
            gtk_list_box_append(GTK_LIST_BOX(app->cart_list), row);
            total += items[i].price * items[i].quantity;
        }
        
        gtk_label_set_text(GTK_LABEL(app->total_label), 
                          g_strdup_printf("Total: $%.2f", total));
        free(items);
    }
    
    update_cart_badge(app);
}

void add_to_cart(EcommerceApp *app, int product_id) {
    DbResult result = add_cart_item(app->current_user->user_id, product_id, 1);
    
    if (result == DB_SUCCESS) {
        update_cart_badge(app);
        show_message_dialog(GTK_WINDOW(app->window), "Item added to cart!");
    } else {
        show_error_dialog(GTK_WINDOW(app->window), "Failed to add item to cart");
    }
}

void update_cart_badge(EcommerceApp *app) {
    int count = get_cart_count(app->current_user->user_id);
    gtk_label_set_text(GTK_LABEL(app->cart_badge), g_strdup_printf("%d", count));
}

// Order management functions
void create_new_order(EcommerceApp *app) {
    const char *address = gtk_string_list_get_string(
        gtk_drop_down_get_model(GTK_DROP_DOWN(app->address_combo)),
        gtk_drop_down_get_selected(GTK_DROP_DOWN(app->address_combo))
    );
    
    DbResult result = create_order(app->current_user->user_id, address);
    
    if (result == DB_SUCCESS) {
        show_message_dialog(GTK_WINDOW(app->window), 
                          "Order placed successfully!");
        clear_cart(app->current_user->user_id);
        refresh_cart(app);
        refresh_order_list(app);
        switch_to_page(app, "orders");
    } else {
        show_error_dialog(GTK_WINDOW(app->window), 
                         "Failed to create order. Please try again.");
    }
}

void refresh_order_list(EcommerceApp *app) {
    // Clear existing items
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(app->order_list)) != NULL) {
        gtk_list_box_remove(GTK_LIST_BOX(app->order_list), child);
    }
    
    Order *orders;
    int count;
    
    if (get_user_orders(app->current_user->user_id, &orders, &count) == DB_SUCCESS) {
        for (int i = 0; i < count; i++) {
            GtkWidget *row = create_order_row(&orders[i]);
            gtk_list_box_append(GTK_LIST_BOX(app->order_list), row);
        }
        free(orders);
    }
}

// Cleanup functions
void cleanup_app(EcommerceApp *app) {
    if (app) {
        if (app->current_user) {
            g_free(app->current_user);
        }
        
        // Cleanup database connections and resources
        cleanup_database();
        cleanup_auth();
        
        // Free the main app structure
        g_free(app);
    }
}

// Helper functions
static GtkWidget *create_cart_item_row(CartItem *item, EcommerceApp *app) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(row, 10);
    gtk_widget_set_margin_end(row, 10);
    gtk_widget_set_margin_top(row, 5);
    gtk_widget_set_margin_bottom(row, 5);
    
    GtkWidget *name_label = gtk_label_new(item->name);
    gtk_widget_set_hexpand(name_label, TRUE);
    gtk_label_set_xalign(GTK_LABEL(name_label), 0);
    
    GtkWidget *quantity_spin = gtk_spin_button_new_with_range(1, 99, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(quantity_spin), item->quantity);
    
    GtkWidget *price_label = gtk_label_new_printf("$%.2f", item->price);
    
    GtkWidget *remove_button = gtk_button_new_from_icon_name("user-trash-symbolic");
    gtk_widget_add_css_class(remove_button, "destructive-action");
    
    gtk_box_append(GTK_BOX(row), name_label);
    gtk_box_append(GTK_BOX(row), quantity_spin);
    gtk_box_append(GTK_BOX(row), price_label);
    gtk_box_append(GTK_BOX(row), remove_button);
    
    g_signal_connect(quantity_spin, "value-changed", 
                    G_CALLBACK(update_cart_item_quantity), item);
    g_signal_connect_swapped(remove_button, "clicked", 
                            G_CALLBACK(remove_cart_item), item);
    
    return row;
}

static GtkWidget *create_order_row(Order *order) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(row, 10);
    gtk_widget_set_margin_end(row, 10);
    gtk_widget_set_margin_top(row, 5);
    gtk_widget_set_margin_bottom(row, 5);
    
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *order_id = gtk_label_new_printf("Order #%d", order->order_id);
    GtkWidget *date = gtk_label_new(order->order_date);
    GtkWidget *status = gtk_label_new(order->status);
    
    gtk_widget_add_css_class(status, order->status);
    
    gtk_box_append(GTK_BOX(header), order_id);
    gtk_box_append(GTK_BOX(header), date);
    gtk_box_append(GTK_BOX(header), status);
    
    GtkWidget *items = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    for (int i = 0; i < order->item_count; i++) {
        GtkWidget *item = gtk_label_new_printf("%s x%d - $%.2f",
                                              order->items[i].name,
                                              order->items[i].quantity,
                                              order->items[i].price);
        gtk_label_set_xalign(GTK_LABEL(item), 0);
        gtk_box_append(GTK_BOX(items), item);
    }
    
    gtk_box_append(GTK_BOX(row), header);
    gtk_box_append(GTK_BOX(row), items);
    
    return row;
}