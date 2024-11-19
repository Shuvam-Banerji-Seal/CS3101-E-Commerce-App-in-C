
#include "menu2.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ascii_art.c"
//#include "other_functions.c"


void main()
{       initializer25();//this is for creating the words.txt file
        extract_words_from_db_to_file();//this is for creating the words.txt file
        //team_name();
        if (!init_database()) {
        printf("Database initialization failed. Exiting.\n");
        return; // Exit the program if the database cannot be initialized
    }

    
    home_page();

}