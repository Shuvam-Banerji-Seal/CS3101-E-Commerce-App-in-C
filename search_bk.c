#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_INPUT_SIZE 256
#define MAX_WORD_LENGTH 100
#define MAX_CHILDREN 50 // Maximum edit distance we'll consider

typedef struct BKNode {
    char* word;
    struct BKNode* children[MAX_CHILDREN];
} BKNode;

// Utility functions
static inline void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Utility function to get minimum of three numbers
static inline int min3(int a, int b, int c) {
    return (a < b) ? (a < c ? a : c) : (b < c ? b : c);
}

// Function to calculate edit distance between two words
int edit_distance(const char* s1, const char* s2) {
    int len1 = strlen(s1), len2 = strlen(s2);
    int dp[len1 + 1][len2 + 1];

    for (int i = 0; i <= len1; i++) {
        for (int j = 0; j <= len2; j++) {
            if (i == 0)
                dp[i][j] = j;
            else if (j == 0)
                dp[i][j] = i;
            else if (s1[i - 1] == s2[j - 1])
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = 1 + min3(dp[i - 1][j - 1], dp[i - 1][j], dp[i][j - 1]);
        }
    }

    return dp[len1][len2];
}

// Create a new BKNode
BKNode* create_node(const char* word) {
    BKNode* node = (BKNode*)malloc(sizeof(BKNode));
    if (!node) return NULL;

    node->word = strdup(word);
    if (!node->word) {
        free(node);
        return NULL;
    }

    memset(node->children, 0, sizeof(node->children));
    return node;
}

// Insert a word into the BK-tree
void insert_word(BKNode* root, const char* word) {
    if (!root || !word) return;

    int distance = edit_distance(root->word, word);

    if (distance == 0) return;

    if (root->children[distance] == NULL) {
        root->children[distance] = create_node(word);
    } else {
        insert_word(root->children[distance], word);
    }
}

// Search for words within a specified distance
void search_word(BKNode* root, const char* word, int max_distance, char matches[][MAX_WORD_LENGTH], int* match_count) {
    if (!root) return;

    int distance = edit_distance(root->word, word);

    if (distance <= max_distance) {
        strncpy(matches[*match_count], root->word, MAX_WORD_LENGTH - 1);
        matches[*match_count][MAX_WORD_LENGTH - 1] = '\0';
        (*match_count)++;
    }

    for (int i = (distance - max_distance > 0) ? distance - max_distance : 1; i <= distance + max_distance && i < MAX_CHILDREN; i++) {
        if (root->children[i]) {
            search_word(root->children[i], word, max_distance, matches, match_count);
        }
    }
}

// Function to load words from a file and build a BK-tree
BKNode* build_bktree_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    BKNode* root = NULL;
    char word[MAX_WORD_LENGTH];

    while (fscanf(file, "%s", word) != EOF) {
        if (!root) {
            root = create_node(word);
        } else {
            insert_word(root, word);
        }
    }

    fclose(file);
    return root;
}

int search_helper(char input_word[256],char checking[256]) {
    // File containing words
    const char* filename = "words.txt";
    //char checking[256];
    // Build the BK-tree
    BKNode* root = build_bktree_from_file(filename);
    if (!root) {
        fprintf(stderr, "Failed to build BK-tree\n");
        return 1;
    }

    // Ask the user for a word
    //char input_word[MAX_WORD_LENGTH];
   // printf("Enter a word to search: ");
   // scanf("%s", input_word);

    // Find matches
    char matches[100][MAX_WORD_LENGTH];
    int match_count = 0;
    search_word(root, input_word, 2, matches, &match_count);

    // Print matches
   // printf("Words within an edit distance of 1 from '%s':\n", input_word);
    for (int i = 0; i < match_count; i++) {
       // printf("- %s\n", matches[i]);
    }
    if(match_count!=0){
    strcpy(checking, matches[0]);
   // printf("Checking: %s\n", checking);
   }
    else {
        strcpy(checking, input_word);
;
    }

    return 0;
}


#define MAX_WORDS 20       // Maximum number of words
//#define MAX_WORD_LENGTH 300 // Maximum length of each word

// Function to split a string into words
/*int parse_words(const char* input, char words[MAX_WORDS][MAX_WORD_LENGTH]) {
    int word_count = 0; // Tracks the number of words extracted
    int start = 0;      // Tracks the start of each word
    int length = strlen(input);
    
    for (int i = 0; i <= length; i++) {
        if (isalnum(input[i])) {
            // Mark the beginning of a word
            if (start == 0) start = i;
        } else if (start != 0) {
            // Extract the word
            int word_len = i - start;
            if (word_count < MAX_WORDS && word_len < MAX_WORD_LENGTH) {
                strncpy(words[word_count], &input[start], word_len);
                words[word_count][word_len] = '\0'; // Null-terminate the string
                word_count++;
            }
            start = 0; // Reset the word start position
        }
    }
    
    return word_count;
}*/

/*int stng_breaker1(char checker1[256],char checker2[256]) {
    char input[600]; // Assumes no more than 20 words of length 300 each
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    
    printf("Enter a string (max 20 words, no word longer than 300 characters):\n");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0'; // Remove trailing newline if present

    // Parse the input string
    int word_count = parse_words(input, words);

    // Display the results
    printf("Your inputted string contains: %d words\n", word_count);
    if (word_count > 0) {
        printf("The words are: ");
        for (int i = 0; i < word_count; i++) {
            printf("%s   %d \n", words[i],i);
            if (i < word_count - 1) printf(", ");
        }
        printf(".\n");
    }

    
    strcpy(checker1,words[0]);
    printf("Checking: %s\n", checker1);

    if (word_count<2){
        strcpy(checker2,checker1);
        
    }
    else{
        strcpy(checker2,words[1]);
        
    }
    return 0;
}*/



#define MAX_WORDS 20       // Maximum number of words
//#define MAX_WORD_LENGTH 300 // Maximum length of each word

// Function to split a string into words
int parse_words(const char* input, char words[MAX_WORDS][MAX_WORD_LENGTH]) {
    int word_count = 0; // Tracks the number of words extracted
    int start = -1;     // Tracks the start of each word (initialized to -1)
    int length = strlen(input);
    
    for (int i = 0; i <= length; i++) {
        if (isalnum(input[i])) {
            // Mark the beginning of a word
            if (start == -1) start = i;
        } else if (start != -1) {
            // Extract the word
            int word_len = i - start;
            if (word_count < MAX_WORDS && word_len < MAX_WORD_LENGTH) {
                strncpy(words[word_count], &input[start], word_len);
                words[word_count][word_len] = '\0'; // Null-terminate the string
                word_count++;
            }
            start = -1; // Reset the word start position
        }
    }
    
    return word_count;
}

int stng_breaker(char checker1[256],char checker2[256]) {
    char input[600]; // Assumes no more than 20 words of length 300 each
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    
    printf("Enter a  search string:\n");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0'; // Remove trailing newline if present

    // Parse the input string
    int word_count = parse_words(input, words);

    // Display the results
    // printf("Your inputted string contains: %d words\n", word_count);
    // if (word_count > 0) {
    //     printf("The words are: ");
    //     for (int i = 0; i < word_count; i++) {
    //         printf("%s", words[i]);
    //         if (i < word_count - 1) printf(", ");
    //     }
    //     printf(".\n");
    // }

    strcpy(checker1,words[0]);
    //printf("Checking: %s\n", checker1);

    if (word_count<2){
        strcpy(checker2,checker1);
        
    }
    else{
        strcpy(checker2,words[1]);
        
    }   
    

    return 0;
}























// int main() {
// //    printf("Enter a word to search: ");
// //    char input_word[256];
// //    scanf("%s", input_word);
// //    char checking[256];
// //     search_helper(input_word,checking);

// //    printf("Checking: %s\n", checking);

// //    return 0;
//     char checker1[256];
//     char checker2[256];
//     char checker3[256];
//     char checker4[256];
//     stng_breaker(checker1,checker2);
//     printf("Checking: %s\n", checker1);
//     printf("Checking: %s\n", checker2);

//     search_helper(checker1,checker3);
//     search_helper(checker2,checker4);
//     printf("Checking: %s\n", checker3);
//     printf("Checking: %s\n", checker4);
//     return 0;
// }
