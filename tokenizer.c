// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <ctype.h>
// need to be careful so that the libraries don't get added multiple times
//#define MAX_WORD_LENGTH 100
//#define BUFFER_SIZE 1000

// Function to check if a character is a delimiter
/*int isDelimiter(char ch) {
    return isspace(ch) || ispunct(ch) || ch == '\n' || ch == '\0';
}*/

// Function to write words to a file, appending them without overwriting existing content
void appendWordsToFile(char *words[], int wordCount, const char *filename) {
    FILE *file = fopen(filename, "a"); // Open file in append mode
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Write each word to a new line in the file
    for (int i = 0; i < wordCount; i++) {
        fprintf(file, "%s\n", words[i]);
    }

    fclose(file);
}

// Main function
int file_appender1(char input[1024], char *words[1024], int wordCount) {
    // char input[BUFFER_SIZE];
    // char *words[BUFFER_SIZE ]; // Assuming half the buffer could be words
    // int wordCount = 0;

   // printf("Enter a string: ");
    //fgets(input, 1024, stdin);

    char *token = strtok(input, " \t\n\r.,;!?\"'()[]{}<>\\|/~`@#$%^&*-_=+");
    while (token != NULL) {
        words[wordCount] = malloc(strlen(token) + 1); // Allocate memory for each word
        if (words[wordCount] == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        strcpy(words[wordCount], token); // Copy the word into the array
        wordCount++;
        token = strtok(NULL, " \t\n\r.,;!?\"'()[]{}<>\\|/~`@#$%^&*-_=+"); // Get the next token
    }

    if (wordCount > 0) {
        appendWordsToFile(words, wordCount, "words.txt");
       // printf("Words successfully appended to 'words.txt'.\n");
    } 
    // Free allocated memory
    for (int i = 0; i < wordCount; i++) {
        free(words[i]);
    }

    return 0;
}

// int main(){
//     char input[BUFFER_SIZE];
//     char *words[BUFFER_SIZE]; // Assuming half the buffer could be words
//     int wordCount = 0;

//     // printf("Enter a string: ");
//     // fgets(input, BUFFER_SIZE, stdin);
//     file_appender1(input, words, wordCount);
//     return 0;}

