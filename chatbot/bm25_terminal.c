#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <ctype.h>

#define MAX_FILES 1000
#define MAX_TERMS 100000
#define MAX_FILENAME 256
#define MAX_LINE 10240
#define MAX_QUERY 256
#define K1 0.6// These choice of values can be adjusted
#define B 1.0
// BTW the above choice of parameters are based on a recent study I did for TREC on ToT queries and found that these are 
// in most cases a better choice for vague queries.

//Command to execute gcc `pkg-config --cflags gtk4 libadwaita-1` bm25gtk_copy.c -o bm25neogtk `pkg-config --libs gtk4 libadwaita-1` -lm

//btw I am using codeium to generate a lot of the comments so that it helps in the understanding of the code

//all function declarations
// Function to convert string to lowercase
void to_lower(char* str);

// Function to check if a term exists in IDF values
double get_idf(const char* term);

// Function to tokenize text
char** tokenize(char* text, int* num_tokens);

// Function to calculate average document length
double get_avg_doc_length();

// Function to calculate document frequencies and IDF values separately
void calculate_idf_values();

// Function to index documents in the given directory
void index_documents(const char* directory);

// Function to calculate BM25 score
void calculate_bm25_scores(const char* query);

// Corrected comparison function for sorting documents by score
int compare_docs(const void* a, const void* b);

// Modified display function to show all results
void display_search_results(const char* directory);

// Function to display file content
void display_file_content(const char* filename, const char* directory);

// Function to save file list
void save_file_list(const char* directory);






// Structure to store term frequency in a document
typedef struct {
    char* term;
    int freq;
} TermFreq;

// Structure to store document information
typedef struct {
    char filename[MAX_FILENAME];
    TermFreq* terms;
    int num_terms;
    int total_terms;
    double score;
} Document;

// Structure to store IDF values
typedef struct {
    char* term;
    double idf;
} IDFEntry;

Document docs[MAX_FILES];
IDFEntry idf_values[MAX_TERMS];
int num_docs = 0;
int num_terms = 0;

// Function to convert string to lowercase like ofcourse
void to_lower(char* str) {
    for(int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Function to check if a term exists in IDF values
double get_idf(const char* term) {
    for(int i = 0; i < num_terms; i++) {
        if(strcmp(idf_values[i].term, term) == 0) {
            return idf_values[i].idf;
        }
    }
    return 0.0;
}

// Function to tokenize text, sadly there exists no preexisting library
char** tokenize(char* text, int* num_tokens) {
    char** tokens = malloc(MAX_TERMS * sizeof(char*));
    *num_tokens = 0;
    char* token = strtok(text, " \t\n\r\f.,!?\"';:()[]{}");
    
    while(token != NULL && *num_tokens < MAX_TERMS) {
        to_lower(token);
        tokens[*num_tokens] = strdup(token);
        (*num_tokens)++;
        token = strtok(NULL, " \t\n\r\f.,!?\"';:()[]{}");
    }
    
    return tokens;
}

// Function to calculate average document length
double get_avg_doc_length() {
    double total = 0;
    for(int i = 0; i < num_docs; i++) {
        total += docs[i].total_terms;
    }
    return total / num_docs;
}


// Modified function to calculate document frequencies and IDF values separately
void calculate_idf_values() {
    // Reset existing IDF values
    num_terms = 0;
    
    // First pass: collect all unique terms
    for(int i = 0; i < num_docs; i++) {
        for(int j = 0; j < docs[i].num_terms; j++) {
            const char* term = docs[i].terms[j].term;
            int found = 0;
            
            // Check if term is already in IDF values
            for(int k = 0; k < num_terms; k++) {
                if(strcmp(idf_values[k].term, term) == 0) {
                    found = 1;
                    break;
                }
            }
            
            if(!found && strlen(term) > 1) {  // Only add terms longer than 1 character
                idf_values[num_terms].term = strdup(term);
                idf_values[num_terms].idf = 0.0;  // Initialize IDF
                num_terms++;
            }
        }
    }
    
    // Second pass: calculate document frequencies and IDF
    for(int i = 0; i < num_terms; i++) {
        int doc_freq = 0;
        const char* current_term = idf_values[i].term;
        
        // Count in how many documents this term appears
        for(int j = 0; j < num_docs; j++) {
            for(int k = 0; k < docs[j].num_terms; k++) {
                if(strcmp(docs[j].terms[k].term, current_term) == 0) {
                    doc_freq++;
                    break;  // Term found in this document, move to next document
                }
            }
        }
        
        // Calculate IDF using the correct document frequency
        idf_values[i].idf = log((num_docs + 1.0) / (doc_freq + 0.5));
        // I still have confusion as okapi suggests no +0.5, but while looking up at differnet papers people tend to use different values as the constants
    }
}


//added debug printfsass
void index_documents(const char* directory) {
    DIR* dir = opendir(directory);
    if(dir == NULL) {
        printf("Error opening directory\n");
        return;
    }
    
    struct dirent* entry;
    while((entry = readdir(dir)) != NULL && num_docs < MAX_FILES) {
        // Skip . and .. directories
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char filepath[MAX_FILENAME];
        snprintf(filepath, sizeof(filepath), "%s/%s", directory, entry->d_name);
        
        // Try to open the file
        FILE* file = fopen(filepath, "r");
        if(file == NULL) continue;
        
        // Get file size and allocate content buffer
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        // Skip if file is empty or too large
        if(file_size <= 0 || file_size > MAX_LINE * 1024) {
            fclose(file);
            continue;
        }
        
        char* content = malloc(file_size + 1);
        if(content == NULL) {
            fclose(file);
            continue;
        }
        
        size_t bytes_read = fread(content, 1, file_size, file);
        content[bytes_read] = '\0';
        
        // Tokenize content
        int num_tokens;
        char** tokens = tokenize(content, &num_tokens);
        
        // Store document information
        strcpy(docs[num_docs].filename, entry->d_name);
        docs[num_docs].terms = malloc(MAX_TERMS * sizeof(TermFreq));
        docs[num_docs].num_terms = 0;
        docs[num_docs].total_terms = num_tokens;
        
        // Count term frequencies
        for(int i = 0; i < num_tokens; i++) {
            if(strlen(tokens[i]) <= 1) continue;  // Skip single-character terms
            
            int found = 0;
            for(int j = 0; j < docs[num_docs].num_terms; j++) {
                if(strcmp(docs[num_docs].terms[j].term, tokens[i]) == 0) {
                    docs[num_docs].terms[j].freq++;
                    found = 1;
                    break;
                }
            }
            if(!found && docs[num_docs].num_terms < MAX_TERMS) {
                docs[num_docs].terms[docs[num_docs].num_terms].term = strdup(tokens[i]);
                docs[num_docs].terms[docs[num_docs].num_terms].freq = 1;
                docs[num_docs].num_terms++;
            }
        }
        
        num_docs++;
        free(content);
        for(int i = 0; i < num_tokens; i++) {
            free(tokens[i]);
        }
        free(tokens);
        fclose(file);
    }
    closedir(dir);
    
    // Calculate IDF values after all documents are indexed
    calculate_idf_values();
}



void calculate_bm25_scores(const char* query) {
    int num_query_terms;
    char** query_terms = tokenize(strdup(query), &num_query_terms);
    double avg_doc_length = get_avg_doc_length();
    
    printf("\nDebug: Average document length: %.2f\n", avg_doc_length);
    printf("Debug: Query terms (%d): ", num_query_terms);
    for(int i = 0; i < num_query_terms; i++) {
        printf("%s ", query_terms[i]);
    }
    printf("\n");
    
    // Initialize all scores to 0
    for(int i = 0; i < num_docs; i++) {
        docs[i].score = 0.0;
    }
    
    for(int i = 0; i < num_docs; i++) {
        double score = 0.0;
        printf("\nDebug: Processing document: %s\n", docs[i].filename);
        printf("Debug: Document length: %d terms\n", docs[i].total_terms);
        
        for(int j = 0; j < num_query_terms; j++) {
            double tf = 0.0;
            
            // Find term frequency in document
            for(int k = 0; k < docs[i].num_terms; k++) {
                if(strcmp(docs[i].terms[k].term, query_terms[j]) == 0) {
                    tf = (double)docs[i].terms[k].freq;
                    break;
                }
            }
            
            double idf = get_idf(query_terms[j]);
            double doc_length = (double)docs[i].total_terms;
            
            printf("Debug: Term '%s': tf=%.2f, idf=%.4f\n", 
                   query_terms[j], tf, idf);
            
            // Prevent division by zero
            if(doc_length == 0) doc_length = 1.0;
            if(avg_doc_length == 0) avg_doc_length = 1.0;
            
            // Enhanced BM25 formula with detailed calculation steps
            if(tf > 0.0) {  // Remove idf check to include all matching terms
                double numerator = tf * (K1 + 1.0);
                double denominator = tf + K1 * (1.0 - B + B * (doc_length / avg_doc_length));
                double term_score = 0.0;
                
                // Prevent division by zero
                if(denominator > 0.0) {
                    term_score = idf * (numerator / denominator);
                }
                
                printf("Debug: Term score calculation:\n");
                printf("       numerator=%.4f, denominator=%.4f\n", 
                       numerator, denominator);
                printf("       term_score=%.4f\n", term_score);
                
                score += term_score;
            }
        }
        
        docs[i].score = score;
        printf("Debug: Final document score: %.4f\n", score);
    }
    
    // Clean up
    for(int i = 0; i < num_query_terms; i++) {
        free(query_terms[i]);
    }
    free(query_terms);
}

// Corrected comparison function for sorting documents by score
int compare_docs(const void* a, const void* b) {
    double score_a = ((const Document*)a)->score;
    double score_b = ((const Document*)b)->score;
    
    if (score_b > score_a) return 1;
    if (score_b < score_a) return -1;
    return 0;
}

// a better printf designfor displaying the outputs
void display_search_results(const char* directory) {
    printf("\nSearch Results:\n");
    printf("----------------------------------------\n");
    
    // Sort documents by score (already done by qsort)
    int displayed = 0;
    
    // Show all documents, even with zero scores
    for(int i = 0; i < num_docs; i++) {
        displayed++;
        printf("%d. %s\n", displayed, docs[i].filename);
        printf("   Score: %.4f\n", docs[i].score);
        
        // Add more detailed information about the document
        printf("   Document length: %d terms\n", docs[i].total_terms);
        printf("   Unique terms: %d\n", docs[i].num_terms);
        
        // Show top contributing terms (up to 5) 
        // or that was the plan... I still can't debug the error why it sometimes just lists all the files
        printf("   Top terms: ");
        int terms_to_show = docs[i].num_terms < 5 ? docs[i].num_terms : 5;
        for(int j = 0; j < terms_to_show; j++) {
            printf("%s(%d) ", docs[i].terms[j].term, docs[i].terms[j].freq);
        }
        printf("\n\n");
    }
    
    printf("----------------------------------------\n");
    printf("Displayed %d documents\n", displayed);
    printf("Results are sorted by relevance score (highest to lowest)\n");
}


// Function to display file content
// This needs a gtk version
void display_file_content(const char* filename, const char* directory) {
    char filepath[MAX_FILENAME];
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);
    
    FILE* file = fopen(filepath, "r");
    if(file == NULL) {
        printf("Error opening file: %s\n", filepath);
        return;
    }
    
    char line[MAX_LINE];
    printf("\nContent of %s:\n", filename);
    printf("----------------------------------------\n");
    while(fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    printf("----------------------------------------\n");
    
    fclose(file);
}



// more debugs
int main() {
    char directory[MAX_FILENAME];
    printf("Enter the directory path containing text files: ");
    if(scanf("%s", directory) != 1) {
        printf("Error reading directory path\n");
        return 1;
    }
    getchar();  // Consume newline
    
    printf("Indexing documents...\n");
    index_documents(directory);
    
    if(num_docs == 0) {
        printf("No documents found in the specified directory.\n");
        return 1;
    }
    //huaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaah
    printf("Successfully indexed %d documents\n", num_docs);
    
    while(1) {
        char query[MAX_QUERY];
        printf("\nEnter your search query (or 'quit' to exit): ");
        if(!fgets(query, sizeof(query), stdin)) {
            printf("Error reading query\n");
            continue;
        }
        query[strcspn(query, "\n")] = 0;  // Remove newline
        
        if(strcmp(query, "quit") == 0) break;
        
        calculate_bm25_scores(query);
        qsort(docs, num_docs, sizeof(Document), compare_docs);
        display_search_results(directory);
        
        // Handle document viewing
        int num_results = num_docs < 10 ? num_docs : 10;
        if(num_results > 0) {
            printf("\nEnter document number to view (1-%d) or 0 to search again: ", num_results);
            int selection;
            if(scanf("%d", &selection) != 1) {
                printf("Invalid input\n");
                getchar();  // Clear input buffer
                continue;
            }
            getchar();  // Consume newline
            
            if(selection > 0 && selection <= num_results) {
                display_file_content(docs[selection-1].filename, directory);
            }
        }
    }
    
    // Cleanup very much needed... ngl w/o it, I might have a bg running in the background
    for(int i = 0; i < num_docs; i++) {
        for(int j = 0; j < docs[i].num_terms; j++) {
            free(docs[i].terms[j].term);
        }
        free(docs[i].terms);
    }
    
    for(int i = 0; i < num_terms; i++) {
        free(idf_values[i].term);
    }
    
    return 0;
}