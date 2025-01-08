#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <regex.h>
#include <ctype.h>
#include <dirent.h>

#define MAX_FILES 100
#define MAX_WORD_LENGTH 100
#define CHUNK_SIZE 1024

// custom strcasestr function for case-insensitive search
char *my_strcasestr(const char *haystack, const char *needle);
bool is_regex_pattern(const char *pattern);
void replace(const char *filename, const char *search_word, const char *replace_word, int *count, bool whole_word_only);
char *my_strcasestr_1(const char *haystack, const char *needle);

// function to check if a file has a ".txt" extension
bool has_txt_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    
    return dot && strcmp(dot, ".txt") == 0;
}

// function to record the replace operations in a text file
void log_operation_replace(const char *operation1, const char *text1, const char *operation2, const char *text2, const char *filename, int count) {
    FILE *log_file = fopen("operation_log.txt", "a");
    
    // checks if the log file has been opened successfully or not
    if (!log_file) {
        perror("Log file opening failed");
    
        return;
    }

    // record time
    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0';

    // write the operation details to the log file
    fprintf(log_file, "[%s] Operation-1: %s, Text-1: %s, Operation-2: %s, Text-2: %s, File: %s, Count: %d\n", timestamp, operation1, text1, operation2, text2, filename, count);
    
    // clean up
    fclose(log_file);
}

// function to search regex pattern in files
void search_regex(const char *filename, const char *pattern, int *count, bool whole_word_only, bool case_sensitive) {
    FILE *file = fopen(filename, "r");

    if (!file) {
        perror("File opening failed");
        
        return;
    }

    char modified_pattern[256];

    if (whole_word_only) {
        // modify the pattern to match whole words only
        snprintf(modified_pattern, sizeof(modified_pattern), "\\b%s\\b", pattern);
    } else {
        // use the pattern as is
        strncpy(modified_pattern, pattern, sizeof(modified_pattern) - 1);
        modified_pattern[sizeof(modified_pattern) - 1] = '\0'; // Ensure null termination
    }

    // set regex compilation flags based on case sensitivity
    int regex_flags = REG_EXTENDED;
    
    if (!case_sensitive) {
        regex_flags |= REG_ICASE;  // adds case-insensitivity flag if not case-sensitive
    }

    regex_t regex;
    int reti = regcomp(&regex, modified_pattern, regex_flags);

    if (reti) {
        char errbuf[100];
        
        regerror(reti, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "Regex compilation failed: %s\n", errbuf);
        fclose(file);
        
        return;
    }

    char buffer[CHUNK_SIZE];
    *count = 0;
    int line_number = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        line_number++;
        
        regmatch_t match;
        char *pos = buffer;

        while (regexec(&regex, pos, 1, &match, 0) == 0) {
            (*count)++;

            int position = match.rm_so + 1;
        
            printf("Found in File: %s, Text: '%.*s' at line %d, position %d\n", filename, match.rm_eo - match.rm_so, pos + match.rm_so, line_number, position);

            pos += match.rm_eo; // moves the position to the end of the current match
        }
    }

    regfree(&regex);
    fclose(file);
}

// function to search case insensitive text in files
void search_case_insensitive(const char *filename, const char *search_word, int *count, bool whole_word_only) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("File opening failed");
        return;
    }

    char buffer[CHUNK_SIZE];
    *count = 0;
    int line_number = 0, i;
    int search_len = strlen(search_word);
    char lower_search_word[MAX_WORD_LENGTH];

    // Convert search_word to lowercase once
    for (i = 0; i < search_len; i++) {
        lower_search_word[i] = tolower((unsigned char)search_word[i]);
    }
    lower_search_word[search_len] = '\0';

    while (fgets(buffer, sizeof(buffer), file)) {
        line_number++;

        char *pos = buffer;
        while ((pos = my_strcasestr_1(pos, lower_search_word)) != NULL) {
            if (whole_word_only) {
                // Simplified whole word check
                if ((pos == buffer || !isalnum((unsigned char)*(pos - 1))) &&
                    !isalnum((unsigned char)*(pos + search_len))) {
                    (*count)++;
                    int position = pos - buffer + 1;
                    printf("Found in File: %s, Text: '%s' at line %d, position %d\n", filename, search_word, line_number, position);
                }
            } else {
                (*count)++;
                int position = pos - buffer + 1;
                printf("Found in File: %s, Text: '%s' at line %d, position %d\n", filename, search_word, line_number, position);
            }
            pos += search_len; // Move past the matched word
        }
    }

    fclose(file);
}

char *my_strcasestr_1(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;

    char first_char = tolower((unsigned char)*needle);
    size_t needle_len = strlen(needle);

    for (; *haystack; ++haystack) {
        if (tolower((unsigned char)*haystack) == first_char) {
            if (strncasecmp(haystack, needle, needle_len) == 0) {
                return (char *)haystack;
            }
        }
    }

    return NULL;
}

// custom strcasestr function for case-insensitive search
char *my_strcasestr(const char *haystack, const char *needle) {
    // if the needle is empty, it returns the haystack
    if (!*needle) { 
        return (char *)haystack;
    }
    
    // iterates each character in the haystack
    for (; *haystack; ++haystack) {
        // checks if the current character in the haystack matches the first character in the needle
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            const char *h, *n;
            
            // iterates on both haystack and needle to check for a full match
            for (h = haystack, n = needle; *h && *n; ++h, ++n) {
                // breaks the loop if a character didn't match
                if (tolower((unsigned char)*h) != tolower((unsigned char)*n)) {
                    break;
                }
            }
            
            // return if a match is found
            if (!*n) {
                return (char *)haystack;
            }
        }
    }
    
    return NULL;
}

// function to record the operations in a text file
void log_operation(const char *operation, const char *text, const char *filename, int count) {
    FILE *log_file = fopen("operation_log.txt", "a");
    
    // checks if the log file has been opened successfully or not
    if (!log_file) {
        perror("Log file opening failed");
    
        return;
    }

    // record time
    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0';

    // write the operation details to the log file
    fprintf(log_file, "[%s] Operation: %s, Text: %s, File: %s, Count: %d\n", timestamp, operation, text, filename, count);
    
    // clean up
    fclose(log_file);
}

void count(const char *filename, const char *search_word, int *count, bool exact_match) {
    FILE *file = fopen(filename, "r");
    
    if (!file) {
        perror("File opening failed");
        
        return;
    }

    char buffer[CHUNK_SIZE];
    *count = 0;
    size_t search_word_len = strlen(search_word);

    while (fgets(buffer, sizeof(buffer), file)) {
        char *pos = buffer;
        
        while ((pos = strstr(pos, search_word)) != NULL) {
            // if exact_match is true, check for exact word boundaries
            if (exact_match) {
                // check if the match is an exact word match
                if ((pos == buffer || !isalnum(*(pos - 1))) && 
                    !isalnum(*(pos + search_word_len))) {
                    (*count)++;
                }
            } else {
                // count as a match regardless of surrounding characters
                (*count)++;
            }
        
            pos += search_word_len;  // move to the end of the current match
        }
    }

    fclose(file);
}

// function to replace the text in a file
void replace(const char *filename, const char *search_word, const char *replace_word, int *count, bool whole_word_only) {
    FILE *file = fopen(filename, "r");

    if (!file) {
        perror("File opening failed");
        
        return;
    }

    char temp_filename[MAX_WORD_LENGTH];
    
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    FILE *temp_file = fopen(temp_filename, "w");

    if (!temp_file) {
        perror("Temporary file opening failed");
        fclose(file);
    
        return;
    }

    *count = 0;

    char buffer[CHUNK_SIZE];
    bool case_sensitive = false;
    bool use_regex = is_regex_pattern(search_word);
    regex_t regex;
    int regex_compiled = 0;

    // determine if case sensitivity is needed
    {
        const char *c;
        
        for (c = search_word; *c; ++c) {
            if (isupper((unsigned char)*c)) {
                case_sensitive = true;
        
                break;
            }
        }
    }

    // compile regular expression if needed
    if (use_regex) {
        int reti = regcomp(&regex, search_word, REG_EXTENDED | (case_sensitive ? 0 : REG_ICASE));
        
        if (reti) {
            char errbuf[100];
        
            regerror(reti, &regex, errbuf, sizeof(errbuf));
            fprintf(stderr, "Regex compilation failed: %s\n", errbuf);
            fclose(file);
            fclose(temp_file);
        
            return;
        }
        
        regex_compiled = 1;
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        char *pos = buffer;
        char *match_start = NULL;
        regmatch_t match;
        int regmatch_result = -1;

        while (pos) {
            if (use_regex) {
                regmatch_result = regexec(&regex, pos, 1, &match, 0);
        
                if (regmatch_result == 0) {
                    match_start = pos + match.rm_so;
                } else {
                    match_start = NULL;
                }
            } else {
                if (case_sensitive) {
                    match_start = strstr(pos, search_word);
                } else {
                    match_start = my_strcasestr(pos, search_word);
                }
            }

            if (match_start) {
                if (whole_word_only) {
                    // check for whole word boundaries
                    if ((match_start > buffer && isalnum(*(match_start - 1))) || isalnum(*(match_start + strlen(search_word)))) {
                        pos = match_start + strlen(search_word);
                        
                        continue;
                    }
                }

                (*count)++;
                
                fwrite(buffer, sizeof(char), match_start - buffer, temp_file);
                fwrite(replace_word, sizeof(char), strlen(replace_word), temp_file);

                pos = match_start + strlen(search_word);
                
                memmove(buffer, pos, strlen(pos) + 1);
                
                pos = buffer;
            } else {
                break; // exit the loop if no more matches
            }
        }

        fwrite(buffer, sizeof(char), strlen(buffer), temp_file);
    }

    if (regex_compiled) {
        regfree(&regex);
    }

    fclose(file);
    fclose(temp_file);

    remove(filename);
    rename(temp_filename, filename);
}

// check if the pattern contains any regex special characters
bool is_regex_pattern(const char *pattern) {
    const char *special_chars = ".*+?^${}()|[]\\";
    
    while (*pattern) {
        if (strchr(special_chars, *pattern)) {
            return true;
        }
    
        pattern++;
    }
    
    return false;
}

// function to search text in a file using case sensitive approach
void search_case_sensitive(const char *filename, const char *search_word, int *count, bool whole_word_only) {
    FILE *file = fopen(filename, "r");

    if (!file) {
        perror("File opening failed");
    
        return;
    }

    char buffer[CHUNK_SIZE];
    *count = 0;
    int line_number = 0;
    int search_len = strlen(search_word);

    while (fgets(buffer, sizeof(buffer), file)) {
        line_number++;
        char *pos = buffer;

        while ((pos = strstr(pos, search_word)) != NULL) {
            if (whole_word_only) {
                // check if the match is a whole word
                bool is_whole_word = true;

                // check character before the match
                if (pos > buffer && (isalnum(*(pos - 1)) || *(pos - 1) == '_')) {
                    is_whole_word = false;
                }

                // check character after the match
                if (isalnum(*(pos + search_len)) || *(pos + search_len) == '_') {
                    is_whole_word = false;
                }

                if (!is_whole_word) {
                    pos += search_len; // move to the end of the matched substring
                    continue; // skip this match and continue searching
                }
            }

            (*count)++;
            int position = pos - buffer + 1;
            
            printf("Found in File: %s, Text: '%s' at line %d, position %d\n", filename, search_word, line_number, position);
            
            pos += search_len; // move to the end of the matched word
        }
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    int rank, size, i, option;
    char ch;
    bool isWholeWord = true;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char *files[MAX_FILES];
    int num_files = 0;
    int file_exists = 1; // flag to check if all files exist
    long file_sizes[MAX_FILES] = {0};
    char search_word[MAX_WORD_LENGTH] = {0}, replace_word[MAX_WORD_LENGTH] = {0}, regex_pattern[MAX_WORD_LENGTH] = {0};

    if (rank == 0) {
        // reads all .txt files in the current directory
        DIR *d;
        struct dirent *dir;
        
        d = opendir(".");
        
        if (d) {
            while ((dir = readdir(d)) != NULL && num_files < MAX_FILES) {
                if (has_txt_extension(dir->d_name)) {
                    if (strcmp(dir->d_name, "operation_log.txt") == 0) {
                        continue;  // skip's the operation_log.txt file
                    }
                    
                    files[num_files] = (char *) malloc(MAX_WORD_LENGTH * sizeof(char));
                    
                    if (!files[num_files]) {
                        perror("Memory allocation failed");
                    
                        MPI_Finalize();
                    
                        return 1;
                    }
                    
                    strncpy(files[num_files], dir->d_name, MAX_WORD_LENGTH);
                    
                    num_files++;
                }
            }
            
            closedir(d);
        }
        
        if (num_files == 0) {
            printf("No .txt files found in the current directory.\n");
            
            MPI_Finalize();
            
            return 1;
        }
        
        // check if files exist and get their sizes
        for (i = 0; i < num_files; i++) {
            FILE *file = fopen(files[i], "r");
            
            if (file) {
                fseek(file, 0, SEEK_END);
            
                file_sizes[i] = ftell(file);
            
                fclose(file);
            } else {
                printf("File %s does not exist.\n", files[i]);
            
                file_exists = 0;
            
                break;
            }
        }
    }

    // send the file existence flag to all processes
    MPI_Bcast(&file_exists, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // if any file does not exist, terminate the program
    if (!file_exists) {
        if (rank == 0) {
            printf("Terminating program due to missing file(s).\n");
        }
    
        MPI_Finalize();
    
        return 1;
    }
    
    // read the whole word input
    if (rank == 0) {
        while (true) {
            printf("\nDo you want to search a whole word? (Y | N): ");
            fflush(stdout);
            scanf(" %c", &ch);
            
            if (ch == 'y' || ch == 'Y') {
                isWholeWord = true;
                
                break;
            } else if (ch == 'n' || ch == 'N') {
                isWholeWord = false;
                
                break;
            } else {
                printf("\nInvalid input! Please enter a valid input.");
            }
        }
    }

    // shows the available options to the user
    do {
        if (rank == 0) {
            printf("\nSelect an option:\n1. Count\n2. Search (Case Sensitive)\n3. Search (Case Insensitive)\n4. Search (Regular Expression)\n5. Replace\n\nEnter your option: ");
            fflush(stdout);
            scanf("%d", &option);

            while (getchar() != '\n'); // clear the input buffer

            if (option == 1) {
                printf("\nEnter a word to count: ");
                fflush(stdout);
                scanf("%s", search_word);
            }  else if (option == 2) {
                printf("\nEnter a word to search (Case Sensitive): ");
                fflush(stdout);
                scanf("%s", search_word);
            } else if (option == 3) {
                printf("\nEnter a word to search (Case Insensitive): ");
                fflush(stdout);
                scanf("%s", search_word);
            } else if (option == 4) {
                printf("\nEnter a word to search (Regular Expression): ");
                fflush(stdout);
                scanf("%s", regex_pattern);
            } else if (option == 5) {
                printf("\nEnter a word to search: ");
                fflush(stdout);
                scanf("%s", search_word);
                
                printf("\nEnter a word to replace: ");
                fflush(stdout);
                scanf("%s", replace_word);
            } else {
                printf("\nInvalid option. Please select a valid input.\n");
                
                option = 0; // reset option to ensure the loop continues
            }
        }

        // send the option to all processes
        MPI_Bcast(&option, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (option == 1 || option == 2 || option == 3 || option == 5) {
            // send the search word to all processes
            MPI_Bcast(search_word, MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
            MPI_Bcast(&isWholeWord, 1, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);
           
            if (option == 5) {
                // send the replace word to all processes
                MPI_Bcast(replace_word, MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
            }
           
            break;
        }
        
        if (option == 4) {
            // send the regex word to all processes
            MPI_Bcast(regex_pattern, MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
            
            break;
        }
    } while (true);

    // send the number of files and file sizes to all processes
    MPI_Bcast(&num_files, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(file_sizes, num_files, MPI_LONG, 0, MPI_COMM_WORLD);

    // send the file names to all processes
    for (i = 0; i < num_files; i++) {
        if (rank != 0) {
            files[i] = (char *) malloc(MAX_WORD_LENGTH * sizeof(char));
            
            if (!files[i]) {
                perror("Memory allocation failed");
                MPI_Finalize();
              
                return 1;
            }
        }
        
        // send the file names to all processes
        MPI_Bcast(files[i], MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    // calculate the total size of all files
    long total_size = 0;
    
    for (i = 0; i < num_files; i++) {
        total_size += file_sizes[i];
    }

    // calculate the size of data each process should handle
    long size_per_process = total_size / size;
    long start_size = rank * size_per_process;
    long end_size = (rank == size - 1) ? total_size : start_size + size_per_process;

    int total_count = 0, local_count = 0;
    long current_size = 0;
    
    // measure execution time for count, search, and replace
    if (rank == 0) {
        start_time = MPI_Wtime();  // start timer
    }

    // displays the information on the console and saves the data to a file
    for (i = 0; i < num_files; i++) {
        if (current_size >= start_size && current_size < end_size) {
            if (option == 1) {
                count(files[i], search_word, &local_count, isWholeWord);
                log_operation("Count", search_word, files[i], local_count);
                
                printf("\nFile: %s, Count: %d\n", files[i], local_count);
            } else if (option == 2) {
                search_case_sensitive(files[i], search_word, &local_count, isWholeWord);
                log_operation("Search (Case Sensitive)", search_word, files[i], local_count);
                
                printf("\nFile: %s, Count: %d\n", files[i], local_count);
            }  else if (option == 3) {
                search_case_insensitive(files[i], search_word, &local_count, isWholeWord);
                log_operation("Search (Case Insensitive)", search_word, files[i], local_count);
                
                printf("\nFile: %s, Count: %d\n", files[i], local_count);
            } else if (option == 4) {
                search_regex(files[i], regex_pattern, &local_count, isWholeWord, isWholeWord);
                log_operation("Search (Regular Expression)", regex_pattern, files[i], local_count);
                
                printf("\nFile: %s, Count: %d\n", files[i], local_count);
            }  else if (option == 5) {
                replace(files[i], search_word, replace_word, &local_count, isWholeWord);
                log_operation_replace("Replace", replace_word, "Search", search_word, files[i], local_count);
                
                printf("\nFile: %s, Count: %d, Search: %s, Replace: %s\n", files[i], local_count, search_word, replace_word);
            }
            
            total_count += local_count;
        }
        
        current_size += file_sizes[i];
    }
    
    // measure execution time for count, search, and replace
    if (rank == 0) {
        end_time = MPI_Wtime();  // end timer
    }

    int global_count;
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Reduce(&total_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        if (option == 1) {
            printf("\nCount completed in %f seconds. Total occurrences of '%s': %d\n", end_time - start_time, search_word, global_count);
            //printf("Count completed in %f seconds. Total occurrences: %d\n", end_time - start_time, total_count);
        } else if (option == 2) {
            printf("\nSearch (Case Sensitive) completed in %f seconds. Total occurrences of '%s': %d\n", end_time - start_time, search_word, global_count);
        }  else if (option == 3) {
            printf("\nSearch (Case Insensitive) completed in %f seconds. Total occurrences of '%s': %d\n", end_time - start_time, search_word, global_count);
        } else if (option == 4) {
            printf("\nSearch (Regular Expression) completed in %f seconds. Total occurrences of '%s': %d\n", end_time - start_time, regex_pattern, global_count);
        }  else if (option == 5) {
            printf("\nReplace completed in %f seconds. Total replacements of '%s' with '%s': %d\n", end_time - start_time, search_word, replace_word, global_count);
        }
    }

    // free allocated memory
    for (i = 0; i < num_files; i++) {
        free(files[i]);
    }

    MPI_Finalize();

    return 0;
}
