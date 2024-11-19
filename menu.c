
// #define MAX_INPUT_SIZE 256
#define CLEAR_SCREEN() printf("\033[H\033[J")
#define sql_query_length 2048
#define PRESS_ENTER() do { printf("\nPress Enter to continue..."); while (getchar() != '\n'); } while(0)
// Function prototypes for the menu system
//void clear_input_buffer();
void home_page();
void login_page();
void signup_page();
void user_menu(const char* username);
void admin_menu(const char* username);
void display_products();
void shopping_cart_menu(int user_id);
void checkout_process(int user_id);
void manage_products_menu();
void search_products_menu(bool is_admin);
void manage_address_menu(int user_id);
void view_orders_menu(int user_id, bool is_admin);
void manage_categories_menu();
void manage_order_menu();
void print_products();
void debug();



// // function to clear input buffer
// void clear_input_buffer() {
//     int c;
//     while ((c = getchar()) != '\n' && c != EOF);
// }


// showing the welcome page
void home_page() {
    int r;
    while (1) {
        CLEAR_SCREEN();
        printf("\n=== Welcome to E-Commerce System ===\n");
        //
        printf("1. Login\n");
        printf("2. Sign Up (New User)\n");
        printf("3. Exit\n");
        printf("4. For Debugging \n");
        printf("5. If you are want to know more functions and about the project");
        printf("\nEnter your choice (1-5): ");
        
        int choice;
        scanf("%d", &choice);
        
        
        switch (choice) {
            case 1:
                login_page();
                break;
            case 2:
                signup_page();
                break;

            case 3:
                printf("\nThank you for using our system!\n");
                exit(0);
            case 4:
                debug();
                exit(0);
            
            case 5:
                
                system("./bm25-gtk &!");
                break;
                
            default:
                printf("\nInvalid choice. Please try again.\n");
                PRESS_ENTER();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// login page for user
void login_page() {
    CLEAR_SCREEN();
    char username[MAX_INPUT_SIZE];
    char password[MAX_INPUT_SIZE];
    
    printf("\n=== Login Page ===\n");
    clear_input_buffer();
    
    printf("Username: ");
    fgets(username, MAX_INPUT_SIZE, stdin);
    username[strcspn(username, "\n")] = '\0';
    
    printf("Password: ");
    fgets(password, MAX_INPUT_SIZE, stdin);
    password[strcspn(password, "\n")] = '\0';
    
    if (verify_admin(username, password)) {
        admin_menu(username);
        printf("\nAdmin login successful!\n");
    } else if (verify_user(username, password)) {
        User user;
        if (get_user(username, &user)) {
            update_last_login(user.user_id);
            user_menu(username);
        }
    } else {
        printf("\nInvalid credentials. Please try again.\n");
        PRESS_ENTER();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////

//if the account don't exist then sign up


void signup_page() {
    char username[MAX_INPUT_SIZE];
    char password[MAX_INPUT_SIZE];
    char email[MAX_INPUT_SIZE];
    char phone[MAX_INPUT_SIZE];
    
    printf("Welcome to the Signup Page!\n");
    
    
    while (getchar() != '\n');
    
    
    printf("Enter a username: ");
    if (fgets(username, MAX_INPUT_SIZE, stdin) != NULL) {
        username[strcspn(username, "\n")] = '\0';
    }
    
    
    printf("Enter a password: ");
    if (fgets(password, MAX_INPUT_SIZE, stdin) != NULL) {
        password[strcspn(password, "\n")] = '\0';
    }
    
    
    printf("Enter your email: ");
    if (fgets(email, MAX_INPUT_SIZE, stdin) != NULL) {
        email[strcspn(email, "\n")] = '\0';
    }
    
    
    printf("Enter your phone number: ");
    if (fgets(phone, MAX_INPUT_SIZE, stdin) != NULL) {
        phone[strcspn(phone, "\n")] = '\0';
    }

    //
    printf("DEBUG: About to sign up with:\n");
    printf("Username: '%s'\n", username);
    printf("Password: '%s'\n", password);
    printf("Email: '%s'\n", email);
    printf("Phone: '%s'\n", phone);

    // Attempt to create the user
    if (user_signup(username, password, email, phone)) {
        printf("\nSignup successful! Welcome, %s!\n", username);
    } else {
        printf("\nSignup failed. Please try again with a different username.\n");
    }

    // Redirect to the home page
    home_page();
}
//////////////////////////////////////////////////////////////////////////////////////////////////

void user_menu(const char* username) {
    User user;
    if (!get_user(username, &user)) {
        printf("Error retrieving user information.\n");     
        return;
    }

    while (1) {
        CLEAR_SCREEN();
        printf("\n=== User Menu (%s) ===\n", username);
        printf("1. View Products\n");
        printf("2. Search Products\n");
        printf("3. Shopping Cart\n");
        printf("4. Manage Delivery Addresses\n");
        printf("5. View My Orders\n");
        printf("6. Logout\n");
        
        printf("\nEnter your choice (1-6): ");
        int choice;
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                //display_products();
                print_products();
                break;
            case 2:
                search_products_menu(false);
                break;
            case 3:
                shopping_cart_menu(user.user_id);
                break;
            case 4:
                manage_address_menu(user.user_id);
                break;
            case 5:
                view_orders_menu(user.user_id, false);
                break;
            case 6:
                return;
            default:
                printf("\nInvalid choice. Please try again.\n");
                PRESS_ENTER();
        }
    }
}


void debug() {
    run_sql_query(db);
}

void admin_menu(const char* username) {
    while (1) {
        CLEAR_SCREEN();
        printf("\n=== Admin Menu (%s) ===\n", username);
        printf("1. Manage Products\n");
        printf("2. View All Orders\n");
        printf("3. Search Products\n");
        printf("4. Manage Categories\n");
        printf("5. Logout\n");
        
        printf("\nEnter your choice (1-5): ");
        int choice;
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                manage_products_menu();
                break;
            case 2:
                view_orders_menu(0, true);  // 0 for admin, as they can see all orders
                break;
            case 3:
                search_products_menu(true);
                break;
            case 4:
                manage_categories_menu();
                break;
            case 5:
                return;
            default:
                printf("\nInvalid choice. Please try again.\n");
                PRESS_ENTER();
        }
    }
}


void print_products(void) {
    Product *products = NULL;
    int count = 0;
    
    if (!get_all_products(&products, &count)) {
        fprintf(stderr, "Failed to retrieve products\n");
        return;
    }
    
    if (count == 0) {
        printf("No active products found in the database.\n");
        return;
    }
    
    // Print header
    printf("\n%-6s | %-30s | %-50s | %-10s | %-8s | %-12s | %-20s | %-20s\n",
           "ID", "Name", "Description", "Price", "Stock", "Category", "Created", "Updated");
    printf("%.*s\n", 170, "--------------------------------------------------------------------------------"
           "--------------------------------------------------------------------------------"
           "--------------------------------------------------------------------------------");
    
    // Print each product
    for (int i = 0; i < count; i++) {
        Product *prod = &products[i];
        
        // Convert timestamps to readable format
        char created_str[20] = {0};
        char updated_str[20] = {0};
        strftime(created_str, sizeof(created_str), "%Y-%m-%d %H:%M", localtime(&prod->created_at));
        strftime(updated_str, sizeof(updated_str), "%Y-%m-%d %H:%M", localtime(&prod->updated_at));
        
        // Truncate description if too long
        char desc_truncated[51] = {0};
        strncpy(desc_truncated, prod->description, 47);
        if (strlen(prod->description) > 47) {
            strcat(desc_truncated, "...");
        }
        
        // Print product information
        printf("%-6d | %-30.30s | %-50s | $%-9.2f | %-8d | %-12d | %-20s | %-20s\n",
               prod->product_id,
               prod->name,
               desc_truncated,
               prod->price,
               prod->stock,
               prod->category_id,
               created_str,
               updated_str);
    }
    
    // Print footer with total count
    printf("\nTotal active products: %d\n\n", count);
    
    // Free the products array
    free(products);
}
void display_products() {
    CLEAR_SCREEN();
    printf("\n=== Product Catalog ===\n");
    
    Product* products;
    int count;
    
    if (get_all_products(&products, &count)) {
        printf("\nID  | Name                 | Price    | Stock\n");
        printf("----+----------------------+----------+-------\n");
        
        for (int i = 0; i < count; i++) {
            printf("%-4d| %-20s | $%-8.2f| %d\n",
                   products[i].product_id,
                   products[i].name,
                   products[i].price,
                   products[i].stock);
        }
        free(products);
    } else {
        printf("Error retrieving products.\n");
    }
    PRESS_ENTER();
}

void shopping_cart_menu(int user_id) {
    while (1) {
        CLEAR_SCREEN();
        print_products();
        printf("\n=== Shopping Cart ===\n");
        
        Cart* cart_items;
        int count;
        if (get_user_cart(user_id, &cart_items, &count)) {
            if (count == 0) {
                printf("\nYour cart is empty.\n");
            } else {
                double total = 0;
                printf("\nID  | Product              | Quantity | Price\n");
                printf("----+----------------------+----------+-------\n");
                
                for (int i = 0; i < count; i++) {
                    Product product;
                    if (get_product(cart_items[i].product_id, &product)) {
                        double item_total = product.price * cart_items[i].quantity;
                        total += item_total;
                        printf("%-4d| %-20s | %-8d | $%.2f\n",
                               product.product_id,
                               product.name,
                               cart_items[i].quantity,
                               item_total);
                    }
                }
                printf("\nTotal: $%.2f\n", total);
            }
            free(cart_items);
        }
        
        printf("\n1. Add Item to Cart\n");
        printf("2. Remove Item from Cart\n");
        printf("3. Update Item Quantity\n");
        printf("4. Proceed to Checkout\n");
        printf("5. Back to Main Menu\n");
        
        printf("\nEnter your choice (1-5): ");
        int choice;
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                printf("\nEnter Product ID: ");
                int product_id;
                scanf("%d", &product_id);
                printf("Enter Quantity: ");
                int quantity;
                scanf("%d", &quantity);
                
                if (quantity <= 0) {
                    printf("\nQuantity must be greater than 0.\n");
                    PRESS_ENTER();
                    break;
                }
                if (check_if_in_cart(product_id,user_id)){
                    printf("\nItem already in cart.\n");
                    printf("\nWould you like to update the quantity? (y/n): ");
                    char choice;
                    scanf(" %c", &choice);
                    if (tolower(choice) == 'y') {
                        printf("Enter New Quantity: ");
                        scanf("%d", &quantity);
                        if (update_cart_quantity(product_id, quantity)) {
                            printf("\nItem quantity updated successfully!\n");
                        } else {
                            printf("\nFailed to update item quantity.\n");
                        }
                    }
                    PRESS_ENTER();
                    break;
                }

                Cart cart = {
                    .user_id = user_id,
                    .product_id = product_id,
                    .quantity = quantity
                };
                
                if (add_to_cart(&cart)) {
                    printf("\nItem added to cart successfully!\n");
                } else {
                    printf("\nFailed to add item to cart.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 2: {
                printf("\nEnter Product ID to remove: ");
                int product_id;
                scanf("%d", &product_id);
                
                if (remove_item_from_cart(product_id, user_id)) {
                    printf("\nItem removed from cart successfully!\n");
                } else {
                    printf("\nFailed to remove item from cart.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 3: {
                printf("\nEnter Product ID: ");
                int product_id;
                scanf("%d", &product_id);
                printf("Enter New Quantity: ");
                int quantity;
                scanf("%d", &quantity);
                
                if (update_cart_quantity(product_id, quantity)) {
                    printf("\nQuantity updated successfully!\n");
                } else {
                    printf("\nFailed to update quantity.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 4:
                //checkout_process(user_id);
                place_order(user_id);
                //generate_bill(user_id);
                // Step 9: Clear Cart
        printf("\nClearing your cart...\n");
        if (generate_bill(user_id)==0) {
            if (clear_cart_after_order(user_id)) {
                printf("Cart cleared successfully.\n");
            }
            
        } else {
            printf("Warning: Failed to clear your cart. Please check manually.\n");
        }
                break;
            case 5:
                return;
            default:
                printf("\nInvalid choice. Please try again.\n");
                PRESS_ENTER();
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////


void place_order(int user_id) {
    Order order = {0};
    Payment payment = {0};
    int status;
    char selected_time[20] = {0};
    int selected_address_id = 0;

    printf("\n--- Placing Your Order ---\n");

    // Step 1: Validate Cart and Check Stock Availability
    if (!validate_cart_stock(user_id)) {
        printf("Error: Some products in your cart exceed available stock. Please adjust your cart.\n");
        return;
    }

    // Step 2: Convert Cart to Order
    // status = cart_to_order(&order, user_id);
    // printf("Cart to Order Status: %s\n", (status ? "Success" : "Failed"));
    // if (!status) {
    //     printf("Error: Unable to process your cart. Please try again.\n");
    //     return;
    // }
    // order.user_id = user_id;

    // Step 3: Choose Payment Method
    printf("\nChoose Your Payment Method:\n");
    printf("0 - Credit Card\n1 - Debit Card\n2 - PayPal\n");
    int payment_choice;
    while (true) {
        printf("Enter your choice: ");
        scanf("%d", &payment_choice);
        if (payment_choice == PAYMENT_CREDIT_CARD ||
            payment_choice == PAYMENT_DEBIT_CARD ||
            payment_choice == PAYMENT_PAYPAL) {
            payment.method = payment_choice;
            break;
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    // Step 4: Enter Transaction ID
    printf("Enter Transaction ID: ");
    scanf("%s", payment.transaction_id);

    // Set Payment Details
    time(&payment.payment_date);
    payment.amount = order.total_amount;
    payment.user_id = user_id;

    // Create Payment
    status = create_payment(&payment);
    printf("Payment Status: %s\n", (status ? "Success" : "Failed"));
    if (!status) {
        printf("Error: Payment failed. Please try again.\n");
        return;
    }

    // Step 5: Enhanced Delivery Selection
    printf("\n=== Delivery Options ===\n");
    printf("1. Take-away/Collect from store\n");
    printf("2. Home delivery\n");
    printf("\nEnter your choice (1-2): ");
    
    int delivery_choice;
    scanf("%d", &delivery_choice);
    
    if (delivery_choice == 1) {
        order.delivery_type = DELIVERY_TAKEAWAY;
    } else if (delivery_choice == 2) {  // Home delivery
        order.delivery_type = DELIVERY_HOME;
        
        // Get user addresses
        Address* addresses;
        int count;
        if (!get_user_addresses(user_id, &addresses, &count)) {
            printf("\nError: Unable to fetch addresses. Please add an address first.\n");
            return;
        }
        
        if (count == 0) {
            printf("\nNo delivery addresses found. Please add an address first.\n");
            manage_address_menu(user_id);

        get_user_addresses(user_id, &addresses, &count);
        }
        
        // Display addresses
        printf("\nAvailable Delivery Addresses:\n");
        printf("ID | Address Line | City | Postal Code | Country\n");
        printf("----+---------------------------------+--------------+-------------+---------\n");
        for (int i = 0; i < count; i++) {
            printf("%-4d| %-32s | %-12s | %-11s | %s\n",
                   addresses[i].address_id,
                   addresses[i].address_line,
                   addresses[i].city,
                   addresses[i].postal_code,
                   addresses[i].country);
        }
        
        // Select address
        printf("\nEnter Address ID for delivery: ");
        scanf("%d", &selected_address_id);
        order.address_id = selected_address_id;
        
   
    // }
        // Select time slot
        int slot_choice;
        while (1) {
            // printf("\nEnter your preferred time slot (1-%d): ", 
            //        (int)(sizeof(DEFAULT_TIME_SLOTS)/sizeof(TimeSlot)));
            // scanf("%d", &slot_choice);
            
            // if (slot_choice < 1 || slot_choice > sizeof(DEFAULT_TIME_SLOTS)/sizeof(TimeSlot)) {
            //     printf("\nInvalid time slot selection. Please try again.\n");
            //     continue;
            // }
            // printf("Dealivery Time Slot: %s\n", DEFAULT_TIME_SLOTS[slot_choice - 1].time_slot);
            // // Now check van availability for the selected time slot
            // if (!is_van_available(DEFAULT_TIME_SLOTS[slot_choice - 1].time_slot)) {
            //     printf("\nSorry, no delivery vans are available for this time slot.\n");
            //     printf("Would you like to:\n");
            //     printf("1. Accept a 1-2 hour delay\n");
            //     printf("2. Choose another time slot\n");
            //     printf("3. Cancel order\n");
            // Print available time slots

            printf("\nAvailable Time Slots:\n");
            for (int i = 0; i < sizeof(DEFAULT_TIME_SLOTS) / sizeof(TimeSlot); i++) {
                printf("%d. %s\n", i + 1, DEFAULT_TIME_SLOTS[i].time_slot);
            }

            printf("\nEnter your preferred time slot (1-%d): ", 
                (int)(sizeof(DEFAULT_TIME_SLOTS) / sizeof(TimeSlot)));
            scanf("%d", &slot_choice);

            if (slot_choice < 1 || slot_choice > sizeof(DEFAULT_TIME_SLOTS) / sizeof(TimeSlot)) {
                printf("\nInvalid time slot selection. Please try again.\n");
                continue;
            }

            printf("Chosen Delivery Time Slot: %s\n", DEFAULT_TIME_SLOTS[slot_choice - 1].time_slot);

            // Now check van availability for the selected time slot
            if (!is_van_available(DEFAULT_TIME_SLOTS[slot_choice - 1].time_slot)) {
                printf("\nSorry, no delivery vans are available for this time slot.\n");
                printf("Would you like to:\n");
                printf("1. Accept a 1-2 hour delay\n");
                printf("2. Choose another time slot\n");
                printf("3. Cancel order\n");
            }               
                int delay_choice;
                printf("Enter your choice (1-3): ");
                scanf("%d", &delay_choice);
                
                switch (delay_choice) {
                    case 1:
                        is_delivery_delayed = true;
                        strcpy(selected_time, DEFAULT_TIME_SLOTS[slot_choice - 1].time_slot);
                        order.delivery_time = strdup(selected_time);
                        order.is_delayed = true;
                        printf("\nOrder scheduled with potential delay.\n");
                        break;
                    case 2:
                        continue;  // Go back to time slot selection
                    case 3:
                        printf("\nOrder cancelled. Please try again later.\n");
                        shopping_cart_menu(user_id);
                        break;
                    default:
                        printf("\nInvalid choice. Please try again.\n");
                        continue;
                }
                
                if (delay_choice == 1) break;  // Proceed with delayed delivery
            else {
                // Van is available, proceed normally
                strcpy(selected_time, DEFAULT_TIME_SLOTS[slot_choice - 1].time_slot);
                order.delivery_time = strdup(selected_time);
                order.is_delayed = false;
                printf("\nDelivery van successfully assigned for your time slot.\n");
                break;
            }
        }
        
    } else {
        printf("\nInvalid choice. Please try again.\n");
        return;
    }
    // Finalize Order Details
    time(&order.created_at);

    // Step 6: Create Order
    status = create_order(&order);
    printf("Order Creation Status: %s\n", (status ? "Success" : "Failed"));
    if (status) {
        printf("\nOrder placed successfully! Thank you for shopping with us.\n");
        
        // Step 7: Update Stock of Products
        printf("\nUpdating product stock...\n");
        if (update_stock_after_order(user_id)) {
            printf("Product stock updated successfully.\n");
            
            // Step 8: Update Product Active Status
            printf("Checking for inactive products...\n");
            if (update_product_active_status()) {
                printf("Inactive products updated successfully.\n");
            } else {
                printf("Warning: Some products could not be updated. Please check manually.\n");
            }
        } else {
            printf("Warning: Failed to update product stock. Please check manually.\n");
        }
    } else {
        printf("\nError: Order placement failed. Please contact support.\n");
    }
}

/*



*/












////////////////////////////////////////////////////////////////////////////////////////////////////

void checkout_process(int user_id) {
    CLEAR_SCREEN();
    printf("\n=== Checkout Process ===\n");
    
    // Get delivery preference
    printf("\nDelivery Type:\n");
    printf("1. Take Away\n");
    printf("2. Home Delivery\n");
    printf("\nEnter your choice (1-2): ");
    
    int delivery_choice;
    scanf("%d", &delivery_choice);
    DeliveryType delivery_type = (delivery_choice == 1) ? DELIVERY_TAKEAWAY : DELIVERY_HOME;
    
    int address_id = 0;
    if (delivery_type == DELIVERY_HOME) {
        // Show addresses and let user select one
        Address* addresses;
        int count;
        if (get_user_addresses(user_id, &addresses, &count)) {
            printf("\nSelect Delivery Address:\n");
            for (int i = 0; i < count; i++) {
                printf("%d. %s, %s, %s\n", 
                       addresses[i].address_id,
                       addresses[i].address_line,
                       addresses[i].city,
                       addresses[i].country);
            }
            printf("\nEnter Address ID: ");
            scanf("%d", &address_id);
            free(addresses);
        }
        
        // Check delivery van availability
        printf("\nPreferred Delivery Time (YYYY-MM-DD HH:MM): ");
        char delivery_time[20];
        scanf("%19s", delivery_time);
        
        if (!is_van_available(delivery_time)) {
            printf("\nWarning: Delivery may be delayed due to high demand.\n");
            PRESS_ENTER();
        }
    }
    
    // Create order
    Order order = {
        .user_id = user_id,
        .delivery_type = delivery_type,
        .address_id = address_id,
        .order_status = STATUS_PENDING,
        .payment_status = PAYMENT_UNPAID
    };
    
    if (cart_to_order(&order, user_id)) {
        printf("\nOrder created successfully!\n");
        
        // Handle payment
        printf("\nSelect Payment Method:\n");
        printf("1. Credit Card\n");
        printf("2. Debit Card\n");
        printf("3. PayPal\n");
        
        int payment_choice;
        scanf("%d", &payment_choice);
        
        Payment payment = {
            .order_id = order.order_id,
            .amount = order.total_amount,
            .method = payment_choice - 1,
            .status = PAYMENT_PAID
        };
        
        if (create_payment(&payment)) {
            printf("\nPayment processed successfully!\n");
            printf("\nOrder Summary:\n");
            printf("Order ID: %d\n", order.order_id);
            printf("Total Amount: $%.2f\n", order.total_amount);
            printf("Delivery Type: %s\n", delivery_type == DELIVERY_TAKEAWAY ? "Take Away" : "Home Delivery");
            if (delivery_type == DELIVERY_HOME) {
                printf("Delivery Address ID: %d\n", address_id);
            }
        } else {
            printf("\nPayment processing failed.\n");
        }
    } else {
        printf("\nFailed to create order.\n");
    }
    
    PRESS_ENTER();
}

void manage_order_menu() {
    while (1) {
        CLEAR_SCREEN();
        printf("\n=== Manage Orders ===\n");
        printf("1. View All Orders\n");
        printf("2. Update Order Status\n");
        printf("3. Update Payment Status\n");
        printf("4. Update Delivery Type\n");
        printf("5. Back to Admin Menu\n");
        
        printf("\nEnter your choice (1-5): ");
        int choice;
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                Order* orders;
                int count;
                if (get_order_items(&orders, &count)) {
                    printf("\nOrder ID | User ID | Status | Payment | Delivery | Amount\n");
                    printf("---------|----------|---------|----------|----------|--------\n");
                    
                    for (int i = 0; i < count; i++) {
                        const char* status[] = {"Pending", "Shipped", "Delivered", "Cancelled"};
                        const char* payment[] = {"Paid", "Unpaid", "Refunded"};
                        const char* delivery[] = {"Take Away", "Home Delivery"};
                        
                        printf("%-9d| %-9d| %-8s| %-9s| %-9s| $%.2f\n",
                               orders[i].order_id,
                               orders[i].user_id,
                               status[orders[i].order_status],
                               payment[orders[i].payment_status],
                               delivery[orders[i].delivery_type],
                               orders[i].total_amount);
                    }
                    free(orders);
                } else {
                    printf("\nNo orders found.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 2: {
                printf("\nEnter Order ID: ");
                int order_id;
                scanf("%d", &order_id);
                
                printf("\nSelect New Status:\n");
                printf("1. Pending\n");
                printf("2. Shipped\n");
                printf("3. Delivered\n");
                printf("4. Cancelled\n");
                
                int status;
                scanf("%d", &status);
                status--; // Adjust to match enum values
                
                if (update_order_status(order_id, status)) {
                    printf("\nOrder status updated successfully!\n");
                } else {
                    printf("\nFailed to update order status.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 3: {
                printf("\nEnter Order ID: ");
                int order_id;
                scanf("%d", &order_id);
                
                printf("\nSelect Payment Status:\n");
                printf("1. Paid\n");
                printf("2. Unpaid\n");
                printf("3. Refunded\n");
                
                int status;
                scanf("%d", &status);
                status--; // Adjust to match enum values
                
                if (update_payment(order_id, status)) {
                    printf("\nPayment status updated successfully!\n");
                } else {
                    printf("\nFailed to update payment status.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 4: {
                printf("\nEnter Order ID: ");
                int order_id;
                scanf("%d", &order_id);
                
                printf("\nSelect Delivery Type:\n");
                printf("1. Take Away\n");
                printf("2. Home Delivery\n");
                
                int type;
                scanf("%d", &type);
                type--; // Adjust to match enum values
                
                if (update_delivery(order_id, type)) {
                    printf("\nDelivery type updated successfully!\n");
                } else {
                    printf("\nFailed to update delivery type.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 5:
                return;
            default:
                printf("\nInvalid choice. Please try again.\n");
                PRESS_ENTER();
        }
    }
}


// Function to check if file exists and is readable
bool file_exists_and_readable(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}


void search_products_menu(bool is_admin) {
    char username[100];

    
    while (1) {
        printf("\n=== Search Products ===\n");
        printf("1. Search\n");
        printf("2. Back to Main Menu\n");
        
        int choice;
        scanf("%d", &choice);
        clear_input_buffer();
        
        switch (choice) {
            case 1: {
             
                Product **newpro;
                newpro = (Product**)malloc(100*sizeof(Product*));
                int count;
                 search_products(newpro, &count);
               
                // Handle cart addition for non-admin users
                if (!is_admin) {
                    printf("\nWould you like to add an item to cart? (y/n): ");
                    char cart_choice;
                    scanf(" %c", &cart_choice);
                    clear_input_buffer();
                    
                    if (tolower(cart_choice) == 'y') {
                        printf("Enter Product ID: ");
                        int product_id;
                        scanf("%d", &product_id);
                        
                        printf("Enter Quantity: ");
                        int quantity;
                        scanf("%d", &quantity);
                        clear_input_buffer();
                        
                        User user;
                        if (get_user(username, &user)) {
                            Cart cart = {
                                .user_id = user.user_id,
                                .product_id = product_id,
                                .quantity = quantity
                            };
                            
                            if (add_to_cart(&cart)) {
                                printf("\nItem added to cart successfully!\n");
                            } else {
                                printf("\nFailed to add item to cart.\n");
                            }
                                }
                            }
                        }
                        
                
                free(newpro);
                printf("\nPress Enter to continue...");
                getchar();
                break;
            }
            
            case 2:
                // free_bktree(root);
                
                return;
                
            default:
                printf("\nInvalid choice. Please try again.\n");
                printf("\nPress Enter to continue...");
                getchar();
        }
    }
}






void manage_address_menu(int user_id) {
    while (1) {
        CLEAR_SCREEN();
        printf("\n=== Manage Delivery Addresses ===\n");
        
        Address* addresses;
        int count;
        if (get_user_addresses(user_id, &addresses, &count)) {
            if (count > 0) {
                printf("\nYour Addresses:\n");
                printf("ID  | Address Line                    | City          | Postal Code | Country\n");
                printf("----+---------------------------------+--------------+-------------+---------\n");
                
                for (int i = 0; i < count; i++) {
                    printf("%-4d| %-32s | %-12s | %-11s | %s\n",
                           addresses[i].address_id,
                           addresses[i].address_line,
                           addresses[i].city,
                           addresses[i].postal_code,
                           addresses[i].country);
                }
                free(addresses);
            } else {
                printf("\nNo addresses found.\n");
            }
        }
        
        printf("\n1. Add New Address\n");
        printf("2. Set Default Address\n");
        printf("3. Back to Main Menu\n");
        printf("4. remove your address\n");
        
        printf("\nEnter your choice (1-3): ");
        int choice;
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                Address address;
                address.user_id = user_id;
                
                printf("\nEnter Address Details:\n");
                printf("Address Line: ");
                clear_input_buffer();
                fgets(address.address_line, 256, stdin);
                address.address_line[strcspn(address.address_line, "\n")] = '\0';
                
                printf("City: ");
                fgets(address.city, 100, stdin);
                address.city[strcspn(address.city, "\n")] = '\0';
                
                printf("Postal Code: ");
                fgets(address.postal_code, 20, stdin);
                address.postal_code[strcspn(address.postal_code, "\n")] = '\0';
                
                printf("Country: ");
                fgets(address.country, 100, stdin);
                address.country[strcspn(address.country, "\n")] = '\0';
                
                if (add_address(&address)) {
                    printf("\nAddress added successfully!\n");
                } else {
                    printf("\nFailed to add address.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 2: {
                printf("\nEnter Address ID to set as default: ");
                int address_id;
                scanf("%d", &address_id);
                
                if (set_default_address(user_id, address_id)) {
                    printf("\nDefault address updated successfully!\n");
                } else {
                    printf("\nFailed to update default address.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 3:
                return;


            case 4: {
                printf("\nEnter Address ID to remove: ");
                int address_id;
                scanf("%d", &address_id);
                
                if (remove_address(user_id, address_id)) {
                    printf("\nAddress removed successfully!\n");
                } else {
                    printf("\nFailed to remove address.\n");
                }
                PRESS_ENTER();
                break;
            }
            default:
                printf("\nInvalid choice. Please try again.\n");
                PRESS_ENTER();
        }
    }
}

void view_orders_menu(int user_id, bool is_admin) {
    CLEAR_SCREEN();
    printf("\n=== %s Orders ===\n", is_admin ? "All" : "My");
    
    Order* orders;
    int count;
    if (get_order_items(&orders, &count)) {
        printf("\nOrder ID | Date       | Status    | Total\n");
        printf("---------+------------+-----------+-------\n");
        
        for (int i = 0; i < count; i++) {
            if (is_admin || orders[i].user_id == user_id) {
                char date_str[20];
                strftime(date_str, sizeof(date_str), "%Y-%m-%d", 
                        localtime(&orders[i].created_at));
                
                const char* status[] = {"Pending", "Shipped", "Delivered", "Cancelled"};
                
                printf("%-9d| %-10s | %-9s | $%.2f\n",
                       orders[i].order_id,
                       date_str,
                       status[orders[i].order_status],
                       orders[i].total_amount);
            }
        }
        free(orders);
    } else {
        printf("\nNo orders found.\n");
    }
    PRESS_ENTER();
}

void manage_categories_menu() {
    while (1) {
        CLEAR_SCREEN();
        printf("\n=== Manage Categories ===\n");
        
        Category* categories;
        int count;
        if (get_all_categorys(&categories, &count)) {
            printf("\nExisting Categories:\n");
            printf("ID  | Name                 | Description\n");
            printf("----+----------------------+-------------\n");
            
            for (int i = 0; i < count; i++) {
                printf("%-4d| %-20s | %s\n",
                       categories[i].category_id,
                       categories[i].name,
                       categories[i].description);
            }
            free(categories);
        }
        
        printf("\n1. Add New Category\n");
        printf("2. Update Category\n");
        printf("3. Back to Admin Menu\n");
        
        printf("\nEnter your choice (1-3): ");
        int choice;
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                Category category;
                printf("\nEnter Category Details:\n");
                printf("Name: ");
                clear_input_buffer();
                fgets(category.name, 50, stdin);
                category.name[strcspn(category.name, "\n")] = '\0';
                
                printf("Description: ");
                fgets(category.description, 200, stdin);
                category.description[strcspn(category.description, "\n")] = '\0';
                
                printf("Parent Category ID (0 for none): ");
                scanf("%d", &category.parent_id);
                
                if (add_category(&category)) {
                    printf("\nCategory added successfully!\n");
                } else {
                    printf("\nFailed to add category.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 2: {
                printf("\nEnter Category ID to update: ");
                int category_id;
                scanf("%d", &category_id);
                
                Category category;
                printf("\nEnter New Details:\n");
                printf("Name: ");
                clear_input_buffer();
                fgets(category.name, 50, stdin);
                category.name[strcspn(category.name, "\n")] = '\0';
                
                printf("Description: ");
                fgets(category.description, 200, stdin);
                category.description[strcspn(category.description, "\n")] = '\0';
                
                printf("Parent Category ID (0 for none): ");
                scanf("%d", &category.parent_id);
                
                if (update_category(&category, category_id)) {
                    printf("\nCategory updated successfully!\n");
                } else {
                    printf("\nFailed to update category.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 3:
                return;
            default:
                printf("\nInvalid choice. Please try again.\n");
                PRESS_ENTER();
        }
    }
}


void manage_products_menu() {
    while (1) {
        CLEAR_SCREEN();
        printf("\n=== Manage Products ===\n");
        printf("1. Add New Product\n");
        printf("2. Update Product\n");
        printf("3. Delete Product\n");
        printf("4. View All Products\n");
        printf("5. View Products by Category\n");
        printf("6. Back to Admin Menu\n");
        
        printf("\nEnter your choice (1-6): ");
        int choice;
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                Product product;
                printf("\nEnter Product Details:\n");
                
                printf("Name: ");
                clear_input_buffer();
                fgets(product.name, sizeof(product.name), stdin);
                product.name[strcspn(product.name, "\n")] = '\0';
                
                printf("Description: ");
                fgets(product.description, sizeof(product.description), stdin);
                product.description[strcspn(product.description, "\n")] = '\0';
                
                printf("Price: ");
                scanf("%lf", &product.price);
                
                printf("Stock: ");
                scanf("%d", &product.stock);
                
                // Display available categories
                Category* categories;
                int count;
                if (get_all_categorys(&categories, &count)) {
                    printf("\nAvailable Categories:\n");
                    for (int i = 0; i < count; i++) {
                        printf("%d. %s\n", categories[i].category_id, categories[i].name);
                    }
                    free(categories);
                }
                
                printf("Category ID: ");
                scanf("%d", &product.category_id);
                
                product.created_at = time(NULL);
                product.updated_at = time(NULL);
                product.is_active = true;
                
                if (add_product(&product)) {
                    printf("\n Product Name : %s",product.name);
                    printf("\n Product Description : %s",product.description);
                    printf("\n Product Price : %.2f",product.price);
                    printf("\n Product Stock : %d",product.stock);
                    
                    printf("\n Product Created At : %ld",product.created_at);
                    printf("\n Product Updated At : %ld",product.updated_at);
                    
                    printf("\nProduct added successfully!\n");
                } else {
                    printf("\nFailed to add product.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 2: {
                printf("\nEnter Product ID to update: ");
                int product_id;
                scanf("%d", &product_id);
                
                Product product;
                if (get_product(product_id, &product)) {
                    printf("\nCurrent Product Details:\n");
                    printf("Name: %s\n", product.name);
                    printf("Description: %s\n", product.description);
                    printf("Price: %.2f\n", product.price);
                    printf("Stock: %d\n", product.stock);
                    printf("Category ID: %d\n", product.category_id);
                    
                    printf("\nEnter new details (press Enter to keep current):\n");
                    
                    printf("New Name (current: %s): ", product.name);
                    clear_input_buffer();
                    char input[256];
                    fgets(input, sizeof(input), stdin);
                    if (input[0] != '\n') {
                        input[strcspn(input, "\n")] = '\0';
                        strncpy(product.name, input, sizeof(product.name) - 1);
                    }
                    
                    printf("New Description (current: %s): ", product.description);
                    fgets(input, sizeof(input), stdin);
                    if (input[0] != '\n') {
                        input[strcspn(input, "\n")] = '\0';
                        strncpy(product.description, input, sizeof(product.description) - 1);
                    }
                    
                    printf("New Price (current: %.2f): ", product.price);
                    fgets(input, sizeof(input), stdin);
                    if (input[0] != '\n') {
                        product.price = atof(input);
                    }
                    
                    printf("New Stock (current: %d): ", product.stock);
                    fgets(input, sizeof(input), stdin);
                    if (input[0] != '\n') {
                        product.stock = atoi(input);
                    }
                    
                    printf("New Category ID (current: %d): ", product.category_id);
                    fgets(input, sizeof(input), stdin);
                    if (input[0] != '\n') {
                        product.category_id = atoi(input);
                    }
                    
                    product.updated_at = time(NULL);
                    
                    if (update_product(&product)) {
                        printf("\nProduct updated successfully!\n");
                    } else {
                        printf("\nFailed to update product.\n");
                    }
                } else {
                    printf("\nProduct not found.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 3: {
                printf("\nEnter Product ID to delete: ");
                int product_id;
                scanf("%d", &product_id);
                
                if (delete_product(product_id)) {
                    printf("\nProduct deleted successfully!\n");
                } else {
                    printf("\nFailed to delete product.\n");
                }
                PRESS_ENTER();
                break;
            }
            case 4:
                print_products();
                PRESS_ENTER();
                break;
            case 5: {
                Category* categories;
                int count;
                if (get_all_categorys(&categories, &count)) {
                    printf("\nSelect Category:\n");
                    for (int i = 0; i < count; i++) {
                        printf("%d. %s\n", categories[i].category_id, categories[i].name);
                    }
                    free(categories);
                    
                    printf("\nEnter Category ID: ");
                    int category_id;
                    scanf("%d", &category_id);
                    
                    Product* products;
                    int product_count;
                    if (get_category_products(category_id, &products, &product_count)) {
                        printf("\nProducts in Category:\n");
                        printf("ID  | Name                 | Price    | Stock\n");
                        printf("----+----------------------+----------+-------\n");
                        
                        for (int i = 0; i < product_count; i++) {
                            printf("%-4d| %-20s | $%-8.2f| %d\n",
                                   products[i].product_id,
                                   products[i].name,
                                   products[i].price,
                                   products[i].stock);
                        }
                        free(products);
                    } else {
                        printf("\nNo products found in this category.\n");
                    }
                }
                PRESS_ENTER();
                break;
            }
            case 6:
                return;
            default:
                printf("\nInvalid choice. Please try again.\n");
                PRESS_ENTER();
        }
    }
}