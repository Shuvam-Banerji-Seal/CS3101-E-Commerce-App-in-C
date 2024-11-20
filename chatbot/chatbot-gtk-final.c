#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <ctype.h>
#include <adwaita.h>
#include <gio/gio.h> // For GResource functions

#define MAX_FILES 1000
#define MAX_TERMS 100000
#define MAX_FILENAME 256
#define MAX_LINE 10240
#define MAX_QUERY 256
#define K1 0.6
#define B 1.0
#define APP_ID "org.example.master-chatbot"
#define APP_ICON "org.example.master-chatbot"


extern GResource *bm25search_get_resource(void);

// Global variables for the new widgets
static GtkWidget *status_page;
static GtkWidget *main_content;
static AdwNavigationView *nav_view;
static GtkWidget *search_entry;
static GtkTextBuffer *results_buffer;
static GtkTextBuffer *file_content_buffer;
static GtkWidget *results_text_view;
static GtkWidget *file_content_text_view;
static AdwWindow *window;


// Structure definitions
typedef struct {
    char* term;
    int freq;
} TermFreq;

typedef struct {
    char filename[MAX_FILENAME];
    TermFreq* terms;
    int num_terms;
    int total_terms;
    double score;
} Document;

typedef struct {
    char* term;
    double idf;
} IDFEntry;

// Global variables
Document docs[MAX_FILES];
IDFEntry idf_values[MAX_TERMS];
int num_docs = 0;
int num_terms = 0;

// GTK widgets
// GtkWidget *window;
// GtkWidget *search_entry;
// GtkWidget *results_text_view;
// GtkTextBuffer *results_buffer;
// GtkWidget *file_content_text_view;
// GtkTextBuffer *file_content_buffer;

char current_directory[MAX_FILENAME];


void to_lower(char* str);
double get_idf(const char* term);
char** tokenize(char* text, int* num_tokens);
double get_avg_doc_length();
void calculate_idf_values();
void index_documents(const char* directory);
void calculate_bm25_scores(const char* query);
int compare_docs(const void* a, const void* b);

// // GTK-specific function declarations
// static void activate(GtkApplication *app, gpointer user_data);
// static void perform_search(GtkWidget *widget, gpointer data);
// static void show_file_content(const char *filename);
// static void on_result_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);

// Add these declarations at the top of your file, after the existing declarations
// Currently I have no idea why am I adding these 
static void folder_selected_callback(GObject *source_object,
                                   GAsyncResult *result,
                                   gpointer user_data);
static void perform_search(GtkWidget *widget, gpointer data);
static void show_file_content(const char *filename);
static void on_result_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);
static void activate(GtkApplication *app, gpointer user_data);



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
    }
}

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

// Modified display function to show all results
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

// Modified perform_search function with correct GTK4 entry text retrieval
static void perform_search(GtkWidget *widget, gpointer data) {
    const char *query = gtk_editable_get_text(GTK_EDITABLE(search_entry));
    
    if (strlen(query) == 0) {
        gtk_text_buffer_set_text(results_buffer, "Please enter a search query", -1);
        return;
    }

    calculate_bm25_scores(query);
    qsort(docs, num_docs, sizeof(Document), compare_docs);

    // Format and display results
    GString *results_text = g_string_new("");
    g_string_append(results_text, "Search Results:\n");
    g_string_append(results_text, "----------------------------------------\n\n");

    for (int i = 0; i < num_docs; i++) {
        char doc_info[1024];
        snprintf(doc_info, sizeof(doc_info),
                "%d. %s\nScore: %.4f\nDocument length: %d terms\nUnique terms: %d\n\n",
                i + 1, docs[i].filename, docs[i].score,
                docs[i].total_terms, docs[i].num_terms);
        g_string_append(results_text, doc_info);
    }

    gtk_text_buffer_set_text(results_buffer, results_text->str, -1);
    g_string_free(results_text, TRUE);
}

static void show_file_content(const char *filename) {
    char filepath[MAX_FILENAME];
    snprintf(filepath, sizeof(filepath), "%s/%s", current_directory, filename);
    
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        gtk_text_buffer_set_text(file_content_buffer, "Error opening file", -1);
        return;
    }

    char *content = NULL;
    size_t content_size = 0;
    FILE *temp = open_memstream(&content, &content_size);
    
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        fputs(line, temp);
    }
    
    fclose(temp);
    fclose(file);

    gtk_text_buffer_set_text(file_content_buffer, content, -1);
    free(content);
}

static void on_result_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
    GtkTextIter iter;
    int buffer_x, buffer_y;
    
    // Convert coordinates to buffer position
    gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(results_text_view),
                                        GTK_TEXT_WINDOW_WIDGET,
                                        x, y,
                                        &buffer_x, &buffer_y);
    
    gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(results_text_view),
                                     &iter,
                                     buffer_x, buffer_y);
    
    // Get the line number
    int line = gtk_text_iter_get_line(&iter);
    
    // Each document entry takes 5 lines (title, score, length, terms, blank line)
    int doc_index = line / 5;
    
    if (doc_index < num_docs) {
        show_file_content(docs[doc_index].filename);
    }
}



// Modified folder selection callback
static void on_select_folder_clicked(GtkWidget *widget, gpointer user_data) {
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Select Directory");
    gtk_file_dialog_set_modal(dialog, TRUE);
    
    GFile *initial_folder = g_file_new_for_path(g_get_home_dir());
    gtk_file_dialog_set_initial_folder(dialog, initial_folder);
    g_object_unref(initial_folder);
    
    gtk_file_dialog_select_folder(dialog,
                                GTK_WINDOW(window),
                                NULL,
                                folder_selected_callback,
                                NULL);
}

// Function to  the welcome page
static AdwNavigationPage* create_welcome_page() {
    status_page = adw_status_page_new();
    adw_status_page_set_icon_name(ADW_STATUS_PAGE(status_page), "user-symbolic");
//     // GdkTexture *texture = gdk_texture_new_from_file("/home/shuvam/codes/c_ir/org.example.bm25search.svg", NULL);
//     GFile *file = g_file_new_for_path("/home/shuvam/codes/c_ir/org.example.bm25search.svg");
// GdkTexture *texture = gdk_texture_new_from_file(file, NULL);
// g_object_unref(file);  // Clean up the GFile object

// GdkPaintable *paintable = GDK_PAINTABLE(texture);
// adw_status_page_set_icon_name(ADW_STATUS_PAGE(status_page), paintable);
// adw_status_page_set_custom_icon(ADW_STATUS_PAGE(status_page), GDK_PAINTABLE(texture));

//g_object_unref(texture);


    adw_status_page_set_title(ADW_STATUS_PAGE(status_page), "Welcome to ECOM Chatbot");
    adw_status_page_set_description(ADW_STATUS_PAGE(status_page), 
        "Start by selecting a folder containing the documentation and FAQs, so that you can ask me questions");

    //  a button box
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(button_box, GTK_ALIGN_CENTER);
    
    //  select folder button
    GtkWidget *select_button = gtk_button_new_with_label("Select Folder");
    gtk_widget_add_css_class(select_button, "suggested-action");
    gtk_widget_add_css_class(select_button, "pill");
    g_signal_connect(select_button, "clicked", G_CALLBACK(on_select_folder_clicked), NULL);
    
    gtk_box_append(GTK_BOX(button_box), select_button);
    adw_status_page_set_child(ADW_STATUS_PAGE(status_page), button_box);
    
    //  and return navigation page
    AdwNavigationPage *page = adw_navigation_page_new(status_page, "Welcome");
    return page;
}
// Function to  the main search interface
static AdwNavigationPage* create_search_interface() {
    //  main content box
    main_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    //  header bar with search
    AdwHeaderBar *header = ADW_HEADER_BAR(adw_header_bar_new());
    
    //  search entry with suggestions
    search_entry = gtk_search_entry_new();
    // I am trying to change the placeholder text like atleast to have something
    // I followed libadwaita guides but couldn't get it to work
    // At this point I have no idea what I am doing wrong
    // Funnily it reverts back to old design sometimes
    //gtk_search_entry_set_placeholder_text(GTK_SEARCH_ENTRY(search_entry), "Search...");

    gtk_widget_set_hexpand(search_entry, TRUE);
    adw_header_bar_set_title_widget(header, search_entry);
    
    gtk_box_append(GTK_BOX(main_content), GTK_WIDGET(header));
    
    //  paned container for results and file content
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_vexpand(paned, TRUE);
    
    // Results sidebar
    AdwToolbarView *sidebar = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());
    GtkWidget *results_scroll = gtk_scrolled_window_new();
    results_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(results_text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(results_text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(results_text_view), GTK_WRAP_WORD_CHAR);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(results_scroll), results_text_view);
    adw_toolbar_view_set_content(sidebar, results_scroll);
    
    // Add click gesture to results
    GtkGesture *click = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_PRIMARY);
    g_signal_connect(click, "pressed", G_CALLBACK(on_result_clicked), NULL);
    gtk_widget_add_controller(results_text_view, GTK_EVENT_CONTROLLER(click));
    
    results_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(results_text_view));
    
    // Content view
    AdwToolbarView *content_view = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());
    GtkWidget *content_scroll = gtk_scrolled_window_new();
    file_content_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(file_content_text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(file_content_text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(file_content_text_view), GTK_WRAP_WORD_CHAR);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(content_scroll), file_content_text_view);
    adw_toolbar_view_set_content(content_view, content_scroll);
    
    file_content_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(file_content_text_view));
    
    // Set up paned with both views
    gtk_paned_set_start_child(GTK_PANED(paned), GTK_WIDGET(sidebar));
    gtk_paned_set_end_child(GTK_PANED(paned), GTK_WIDGET(content_view));
    gtk_paned_set_position(GTK_PANED(paned), 300);
    
    gtk_box_append(GTK_BOX(main_content), paned);
    
    // Connect search entry signal
    g_signal_connect(search_entry, "activate", G_CALLBACK(perform_search), NULL);
    
    //  and return navigation page
    AdwNavigationPage *page = adw_navigation_page_new(main_content, "Search");
    return page;
}


//old stuffffffff


// static GtkWidget* create_welcome_page() {
//     status_page = adw_status_page_new();
//     adw_status_page_set_icon_name(ADW_STATUS_PAGE(status_page), "folder-symbolic");
//     adw_status_page_set_title(ADW_STATUS_PAGE(status_page), "Welcome to BM25 Search");
//     adw_status_page_set_description(ADW_STATUS_PAGE(status_page), 
//         "Start by selecting a folder containing the documents you want to search through.");

//     //  a button box
//     GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
//     gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
//     gtk_widget_set_valign(button_box, GTK_ALIGN_CENTER);
    
//     //  select folder button
//     GtkWidget *select_button = gtk_button_new_with_label("Select Folder");
//     gtk_widget_add_css_class(select_button, "suggested-action");
//     gtk_widget_add_css_class(select_button, "pill");
//     g_signal_connect(select_button, "clicked", G_CALLBACK(on_select_folder_clicked), NULL);
    
//     gtk_box_append(GTK_BOX(button_box), select_button);
//     adw_status_page_set_child(ADW_STATUS_PAGE(status_page), button_box);
    
//     return status_page;
// }

// // Function to  the main search interface
// static GtkWidget* create_search_interface() {
//     //  main content box
//     main_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
//     //  header bar with search
//     AdwHeaderBar *header = ADW_HEADER_BAR(adw_header_bar_new());
    
//     //  search entry with suggestions
//     search_entry = gtk_search_entry_new();
//     gtk_widget_set_hexpand(search_entry, TRUE);
//     adw_header_bar_set_title_widget(header, search_entry);
    
//     gtk_box_append(GTK_BOX(main_content), GTK_WIDGET(header));
    
//     //  paned container for results and file content
//     GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
//     gtk_widget_set_vexpand(paned, TRUE);
    
//     // Results sidebar
//     AdwToolbarView *sidebar = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());
//     GtkWidget *results_scroll = gtk_scrolled_window_new();
//     results_text_view = gtk_text_view_new();
//     gtk_text_view_set_editable(GTK_TEXT_VIEW(results_text_view), FALSE);
//     gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(results_text_view), FALSE);
//     gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(results_text_view), GTK_WRAP_WORD_CHAR);
//     gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(results_scroll), results_text_view);
//     adw_toolbar_view_set_content(sidebar, results_scroll);
    
//     // Add click gesture to results
//     GtkGesture *click = gtk_gesture_click_new();
//     gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_PRIMARY);
//     g_signal_connect(click, "pressed", G_CALLBACK(on_result_clicked), NULL);
//     gtk_widget_add_controller(results_text_view, GTK_EVENT_CONTROLLER(click));
    
//     results_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(results_text_view));
    
//     // Content view
//     AdwToolbarView *content_view = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());
//     GtkWidget *content_scroll = gtk_scrolled_window_new();
//     file_content_text_view = gtk_text_view_new();
//     gtk_text_view_set_editable(GTK_TEXT_VIEW(file_content_text_view), FALSE);
//     gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(file_content_text_view), FALSE);
//     gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(file_content_text_view), GTK_WRAP_WORD_CHAR);
//     gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(content_scroll), file_content_text_view);
//     adw_toolbar_view_set_content(content_view, content_scroll);
    
//     file_content_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(file_content_text_view));
    
//     // Set up paned with both views
//     gtk_paned_set_start_child(GTK_PANED(paned), GTK_WIDGET(sidebar));
//     gtk_paned_set_end_child(GTK_PANED(paned), GTK_WIDGET(content_view));
//     gtk_paned_set_position(GTK_PANED(paned), 300);
    
//     gtk_box_append(GTK_BOX(main_content), paned);
    
//     // Connect search entry signal
//     g_signal_connect(search_entry, "activate", G_CALLBACK(perform_search), NULL);
    
//     return main_content;
// }



// Get that file manager
static void folder_selected_callback(GObject *source_object,
                                   GAsyncResult *result,
                                   gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
    GFile *folder = gtk_file_dialog_select_folder_finish(dialog, result, NULL);
    
    if (folder != NULL) {
        char *folder_path = g_file_get_path(folder);
        strncpy(current_directory, folder_path, MAX_FILENAME - 1);
        g_free(folder_path);
        g_object_unref(folder);
        
        // Index documents
        index_documents(current_directory);
        
        // Switch to main interface
        AdwNavigationPage *search_page = create_search_interface();
        adw_navigation_view_push(nav_view, search_page);
        
        // Update search entry placeholder
        char placeholder[256];
        snprintf(placeholder, sizeof(placeholder), 
                "Search %d documents...", num_docs);
        gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), placeholder);
    }
    
    g_object_unref(dialog);
}



// The all heil the activate
static void activate(GtkApplication *app, gpointer user_data) {
    // The application window
    window = ADW_WINDOW(adw_window_new());
    gtk_window_set_application(GTK_WINDOW(window), app);
    gtk_window_set_title(GTK_WINDOW(window), "ECOM ChatBot");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);
    
    // The navigation view
    nav_view = ADW_NAVIGATION_VIEW(adw_navigation_view_new());
    
    //  and push welcome page
    AdwNavigationPage *welcome_page = create_welcome_page();
    adw_navigation_view_push(nav_view, welcome_page);
    
    // nav view to main content
    adw_window_set_content(window, GTK_WIDGET(nav_view));
    
    gtk_window_present(GTK_WINDOW(window));
}

// int main(int argc, char *argv[]) {
//     // Initialize resources (you'll need to  these)
//     GResource* resource[] = "bm25search_gresources.xml"
//     g_resources_register(bm25search_get_resource(resource));
    
//     AdwApplication *app;
//     int status;
    
//     app = adw_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
//     g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
//     status = g_application_run(G_APPLICATION(app), argc, argv);
//     g_object_unref(app);
    
//     // Cleanup
//     cleanup_resources();
    
//     return status;
// }



static void setup_icon_theme(void) {
    g_autoptr(GtkIconTheme) theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
    gtk_icon_theme_add_resource_path(theme, "/org/example/bm25search");
}


int main(int argc, char *argv[]) {
    // Register the GResource
    //g_resources_register(bm25search_get_resource());
     //g_resources_register(bm25search_get_resource());
//no idea why the xml is not working

    AdwApplication *app;
    int status;
    
    app = adw_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // Optionally unregister the resource (not strictly needed since program exits)
    //g_resources_unregister(bm25search_get_resource());

    return status;
}