#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>


extern int generate_bill(int user_id);


#define MAX_PATH 512


int create_latex_bill(int user_id) {
  
    char username[100] = {0};
    char email[100] = {0};
    char phone[20] = {0};
    char address[200] = {0};
    char city[100] = {0};
    char postal_code[20] = {0};
    char country[100] = {0};
    int order_id;
    double total_amount = 0.0;
    char order_date[50] = {0};
    int delivery_type = 0;
    int payment_status = 1;
    int is_delayed = 0;
    char delivery_time[50] = {0};


    sqlite3_stmt *stmt;
    const char *user_query =
        "SELECT users.username, users.email, users.phone, addresses.address_line, "
        "addresses.city, addresses.postal_code, addresses.country "
        "FROM users "
        "JOIN addresses ON users.user_id = addresses.user_id "
        "WHERE users.user_id = ?;";

    const char *order_query =
        "SELECT orders.order_id, orders.total_amount, orders.created_at, "
        "orders.delivery_type, orders.payment_status, orders.is_delayed, "
        "orders.delivery_time "
        "FROM orders "
        "WHERE orders.user_id = ? AND orders.bill_status = 0 "
        "ORDER BY orders.order_id DESC LIMIT 1;";

    const char *products_query =
        "SELECT products.name, products.price, cart.quantity, "
        "(products.price * cart.quantity) AS total_price "
        "FROM cart "
        "JOIN products ON cart.product_id = products.product_id "
        "WHERE cart.user_id = ?;";

   
    if (sqlite3_prepare_v2(db, user_query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing user query: %s\n", sqlite3_errmsg(db));
        return 20;
    }
    sqlite3_bind_int(stmt, 1, user_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        strncpy(username, (const char *)sqlite3_column_text(stmt, 0), sizeof(username) - 1);
        strncpy(email, (const char *)sqlite3_column_text(stmt, 1), sizeof(email) - 1);
        strncpy(phone, (const char *)sqlite3_column_text(stmt, 2), sizeof(phone) - 1);
        strncpy(address, (const char *)sqlite3_column_text(stmt, 3), sizeof(address) - 1);
        strncpy(city, (const char *)sqlite3_column_text(stmt, 4), sizeof(city) - 1);
        strncpy(postal_code, (const char *)sqlite3_column_text(stmt, 5), sizeof(postal_code) - 1);
        strncpy(country, (const char *)sqlite3_column_text(stmt, 6), sizeof(country) - 1);
    }
    sqlite3_finalize(stmt);


    if (sqlite3_prepare_v2(db, order_query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing order query: %s\n", sqlite3_errmsg(db));
        return 30;
    }
    sqlite3_bind_int(stmt, 1, user_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        order_id = sqlite3_column_int(stmt, 0);
        total_amount = sqlite3_column_double(stmt, 1);
        strncpy(order_date, (const char *)sqlite3_column_text(stmt, 2), sizeof(order_date) - 1);
        delivery_type = sqlite3_column_int(stmt, 3);
        payment_status = sqlite3_column_int(stmt, 4);
        is_delayed = sqlite3_column_int(stmt, 5);
        strncpy(delivery_time, (const char *)sqlite3_column_text(stmt, 6), sizeof(delivery_time) - 1);
    }
    sqlite3_finalize(stmt);


    char latex_filename[MAX_PATH];
    char pdf_filename[MAX_PATH];
    snprintf(latex_filename, sizeof(latex_filename), "bill_%d.tex", user_id);
    snprintf(pdf_filename, sizeof(pdf_filename), "bill_%d.pdf", user_id);

  
    FILE *latex_file = fopen(latex_filename, "w");
    if (!latex_file) {
        fprintf(stderr, "Error creating LaTeX file\n");
        return 40;
    }


    fprintf(latex_file, "\\documentclass[12pt]{article}\n");
    fprintf(latex_file, "\\usepackage{geometry}\n");
    fprintf(latex_file, "\\usepackage{graphicx}\n");
    fprintf(latex_file, "\\usepackage{booktabs}\n");
    fprintf(latex_file, "\\usepackage{xcolor}\n");
    fprintf(latex_file, "\\geometry{a4paper, margin=1in}\n\n");

    fprintf(latex_file, "\\begin{document}\n\n");

  
    fprintf(latex_file, "\\begin{center}\n");
    fprintf(latex_file, "{\\large\\textbf{FUNCTION FORTRESS}}\\\\\n");
    fprintf(latex_file, "\\textit{Ecommerce Solutions, One Function at a Time}\\\\\n");
    fprintf(latex_file, "IISER-Kolkata, Kalyani, West Bengal\\\\\n");
    fprintf(latex_file, "Email: sbs22ms076@iiserkol.ac.in and sg005@iiserkol.ac.in\\\\\n");
    fprintf(latex_file, "Phone: +91 1234 CODE-DEV\n");
    fprintf(latex_file, "\\end{center}\n\n");

  
    fprintf(latex_file, "\\begin{center}\n");
    fprintf(latex_file, "{\\Large\\textbf{SALES INVOICE}}\n");
    fprintf(latex_file, "\\end{center}\n\n");

   
    fprintf(latex_file, "\\section*{Customer Details}\n");
    fprintf(latex_file, "\\begin{tabular}{ll}\n");
    fprintf(latex_file, "\\textbf{Name:} & %s \\\\\n", username);
    fprintf(latex_file, "\\textbf{Email:} & %s \\\\\n", email);
    fprintf(latex_file, "\\textbf{Phone:} & %s \\\\\n", phone);
    fprintf(latex_file, "\\textbf{Address:} & %s, %s %s \\\\\n", address, city, postal_code);
    fprintf(latex_file, "\\textbf{Country:} & %s\n", country);
    fprintf(latex_file, "\\end{tabular}\n\n");

    // Order Details
    fprintf(latex_file, "\\section*{Order Details}\n");
    fprintf(latex_file, "\\begin{tabular}{ll}\n");
    fprintf(latex_file, "\\textbf{Order ID:} & %d \\\\\n", order_id);
    fprintf(latex_file, "\\textbf{Order Date:} & %s \\\\\n", order_date);
    fprintf(latex_file, "\\textbf{Delivery Type:} & %s \\\\\n", 
            (delivery_type == 1) ? "Home Delivery" : "Takeaway");
    
    if (delivery_type == 1) {
        fprintf(latex_file, "\\textbf{Delivery Time:} & %s \\\\\n", 
                delivery_time ? delivery_time : "Not specified");
        if (is_delayed) {
            fprintf(latex_file, "\\textbf{Delivery Note:} & {\\color{red}Possible 1-2 hour delay due to high demand} \\\\\n");
        }
    }
    
    fprintf(latex_file, "\\textbf{Payment Status:} & %s\n", 
            (payment_status == 1) ? "Paid" : "Unpaid");
    fprintf(latex_file, "\\end{tabular}\n\n");

    // Products Table
    fprintf(latex_file, "\\section*{Ordered Items}\n");
    fprintf(latex_file, "\\begin{tabular}{lrrr}\n");
    fprintf(latex_file, "\\toprule\n");
    fprintf(latex_file, "\\textbf{Product} & \\textbf{Price} & \\textbf{Quantity} & \\textbf{Total} \\\\\n");
    fprintf(latex_file, "\\midrule\n");

 
    if (sqlite3_prepare_v2(db, products_query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing products query: %s\n", sqlite3_errmsg(db));
        fclose(latex_file);
        return 50;
    }
    sqlite3_bind_int(stmt, 1, user_id);

    double grand_total = 0.0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *product_name = (const char *)sqlite3_column_text(stmt, 0);
        double price = sqlite3_column_double(stmt, 1);
        int quantity = sqlite3_column_int(stmt, 2);
        double total_price = sqlite3_column_double(stmt, 3);
        grand_total += total_price;

        fprintf(latex_file, "%s & \\${%.2f} & %d & \\${%.2f} \\\\\n", 
                product_name, price, quantity, total_price);
    }
    sqlite3_finalize(stmt);

 
    fprintf(latex_file, "\\midrule\n");
    fprintf(latex_file, "\\textbf{Grand Total} & \\multicolumn{3}{r}{\\textbf{\\${%.2f}}} \\\\\n", grand_total);
    fprintf(latex_file, "\\bottomrule\n");
    fprintf(latex_file, "\\end{tabular}\n\n");

   
    fprintf(latex_file, "\\vfill\n");
    fprintf(latex_file, "\\begin{center}\n");
    fprintf(latex_file, "{\\small Thank you for your purchase!}\\\\\n");
    fprintf(latex_file, "{\\small All prices are in USD. Taxes may apply.}\n");
    fprintf(latex_file, "\\end{center}\n\n");

    fprintf(latex_file, "\\end{document}\n");

    fclose(latex_file);
    // Prepare filename for LaTeX and PDF
   // char latex_filename[MAX_PATH];
    //char pdf_filename[MAX_PATH];
    char compile_command[MAX_PATH];
    snprintf(latex_filename, sizeof(latex_filename), "bill_%d.tex", user_id);
    snprintf(pdf_filename, sizeof(pdf_filename), "bill_%d.pdf", user_id);

    //using system call because it's easy
    snprintf(compile_command, sizeof(compile_command), 
             "pdflatex -interaction=nonstopmode %s", latex_filename);
    
    printf("\nCompiling bill PDF...\n");
    int compile_result = system(compile_command);

  
    if (compile_result == 0) {
        printf("\n===== Bill Generation Successful =====\n");
        printf("PDF Bill Location  : %s\n", pdf_filename);
        printf("LaTeX Source File  : %s\n", latex_filename);
        
        // Optional: Open the generated PDF
        char open_command[MAX_PATH];
        snprintf(open_command, sizeof(open_command), "xdg-open %s", pdf_filename);
        system(open_command);
    } else {
        fprintf(stderr, "Error compiling LaTeX document\n");
        return 60;
    }

    return 0;
}