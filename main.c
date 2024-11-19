#include "database.c"
#include <menu.c>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ascii_art.c"


void main()
{       initializer25();
        if (!init_database()) {
        printf("Database initialization failed. Exiting.\n");
        return; // Exit the program if the database cannot be initialized
    }

    
    home_page();

}