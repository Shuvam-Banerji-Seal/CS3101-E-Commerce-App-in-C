// Soumya what is this for? 

int initializer25() {
    FILE *file;

   
    file = fopen("words.txt", "r");

    if (file) {
       
        fclose(file);
        //printf("File already exists. Nothing to do.\n");
        return 0;
    }

    // If the file does not exist, create it and write the character 'c'
    file = fopen("words.txt", "w");
    if (file == NULL) {
        perror("Error creating file");
        exit(EXIT_FAILURE);
    }

    fputc('c', file);
    fclose(file);

    //printf("File 'words.txt' created and character 'c' inserted.\n");
    return 0;
}


int is_valid_char(char ch) {
    return isalnum(ch); // Allows only letters and numbers
}

// Function to process a string into words and write to a file
void process_and_write_words(const char *text, FILE *file) {
    char word[1024] = {0};
    int index = 0;

    for (int i = 0; text[i] != '\0'; i++) {
        if (is_valid_char(text[i])) {
            word[index++] = text[i];
        } else if (index > 0) {
            word[index] = '\0'; // Null-terminate the word
            fprintf(file, "%s\n", word); // Write the word to the file
            index = 0; // Reset for the next word
        }
    }

    // Write the last word if any
    if (index > 0) {
        word[index] = '\0';
        fprintf(file, "%s\n", word);
    }
}

// Main function to extract words from the database and store them in a file
void extract_words_from_db_to_file() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    FILE *file;

    // Open the database
    if (sqlite3_open("ecommerce.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // SQL query to fetch text fields from the products table
    const char *query = "SELECT name, description FROM products";

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Open the file for writing
    file = fopen("words.txt", "w");
    if (!file) {
        perror("Error opening file");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    // Process each row from the query result
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *name = sqlite3_column_text(stmt, 0);
        const unsigned char *description = sqlite3_column_text(stmt, 1);

        if (name) {
            process_and_write_words((const char *)name, file);
        }
        if (description) {
            process_and_write_words((const char *)description, file);
        }
    }

    // Clean up
    fclose(file);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    printf("Words have been extracted and saved to 'words.txt'.\n");
}
