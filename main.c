#include "database.c"
#include "menu.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ascii_art.c"


void main()
{       initializer25();//this is for creating the words.txt file
        extract_words_from_db_to_file();//this is for creating the words.txt file
        if (!init_database()) {
        printf("Database initialization failed. Exiting.\n");
        return; // Exit the program if the database cannot be initialized
    }

    
    home_page();

}