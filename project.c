#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <regex.h>
#include <ctype.h>

#define MAX_FILES 100
#define MAX_WORD_LENGTH 100
#define CHUNK_SIZE 1024

// custom strcasestr function for case-insensitive search
char *my_strcasestr(const char *haystack, const char *needle);

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
void search_regex(const char *filename, const char *pattern, int *count) {
    FILE *file = fopen(filename, "r");
    
    // checks if the file was opened successfully or not
    if (!file) {
        perror("File opening failed");
    
        return;
    }

    regex_t regex;
    int reti = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE); // compile the regular expression with extended and case-insensitive flags
    
    // checks if the regex compilation is successful
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

    // reads each line from the file
    while (fgets(buffer, sizeof(buffer), file)) {
        line_number++;
        regmatch_t match;
        char *pos = buffer;
        
        // execute the regular expression on the current line
        while (regexec(&regex, pos, 1, &match, 0) == 0) {
            (*count)++;  // increment the counter if a match is found
            
            int position = match.rm_so + 1;
            
            printf("Found in File: %s, Text: '%.*s' at line %d, position %d\n", filename, match.rm_eo - match.rm_so, pos + match.rm_so, position);
    
            pos += match.rm_eo; // moves the position to the end of the current match
        }
    }

    // clean up
    regfree(&regex);
    fclose(file);
}

// function to search case insensitive text in files
void search_case_insensitive(const char *filename, const char *search_word, int *count) {
    FILE *file = fopen(filename, "r");
    
    // checks if the file is opened successfully
    if (!file) {
        perror("File opening failed");
    
        return;
    }

    char buffer[CHUNK_SIZE];
    *count = 0;
    int line_number = 0;

    // read each line from the file
    while (fgets(buffer, sizeof(buffer), file)) {
        line_number++;
        char *pos = buffer;
    
        // search for the word in the current line
        while ((pos = my_strcasestr(pos, search_word)) != NULL) {
            (*count)++; // increment the counter if a match is found
            
            int position = pos - buffer + 1; // calculates the position of the word
            
            printf("Found in File: %s, Text: '%s' at line %d, position %d\n", filename, search_word, line_number, position);
    
            pos += strlen(search_word); // moves pointer to the end of the current matched word
        }
    }

    // clean up
    fclose(file);
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

// function to count the matching words in a file 
void count(const char *filename, const char *search_word, int *count) {
    FILE *file = fopen(filename, "r");
    
    // checks if the log file has been opened successfully or not
    if (!file) {
        perror("File opening failed");
    
        return;
    }

    char buffer[CHUNK_SIZE];
    *count = 0;

    // reads each line from the file
    while (fgets(buffer, sizeof(buffer), file)) {
        char *pos = buffer;
    
        // search for the word in the current line
        while ((pos = strstr(pos, search_word)) != NULL) {
            (*count)++; // increment the counter if a match is found
    
            pos += strlen(search_word); // moves pointer to the end of the current matched word
        }
    }

    // clean up
    fclose(file);
}

// function to replace the text in a file
void replace(const char *filename, const char *search_word, const char *replace_word, int *count) {
    FILE *file = fopen(filename, "r");
    
    // checks if the log file has been opened successfully or not
    if (!file) {
        perror("File opening failed");
    
        return;
    }

    char buffer[CHUNK_SIZE];
    char temp_filename[MAX_WORD_LENGTH];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    // open a temporary file
    FILE *temp_file = fopen(temp_filename, "w");
    
    // check if the temporary file is opened
    if (!temp_file) {
        perror("Temporary file opening failed");
        fclose(file);
    
        return;
    }

    *count = 0;

    // read each line from the file
    while (fgets(buffer, sizeof(buffer), file)) {
        char *pos = buffer;
    
        // search for the word in the current line
        while ((pos = strstr(pos, search_word)) != NULL) {
            (*count)++; // increment the counter if a match is found
            fwrite(buffer, sizeof(char), pos - buffer, temp_file); // writes buffer to the temporary file
            fwrite(replace_word, sizeof(char), strlen(replace_word), temp_file); // replace word in the temporary file
    
            pos += strlen(search_word); // moves pointer to the end of the current matched word
    
            memmove(buffer, pos, strlen(pos) + 1); // moves the buffer in the beginning
    
            pos = buffer; // reset the position pointer
        }
    
        fwrite(buffer, sizeof(char), strlen(buffer), temp_file); // writes the remaining buffer to the temporary file
    }

    // clean up
    fclose(file);
    fclose(temp_file);

    // replace the original file with the temporary file
    remove(filename);
    rename(temp_filename, filename);
}

// function to search text in a file using case sensitive approach
void search_case_sensitive(const char *filename, const char *search_word, int *count) {
    FILE *file = fopen(filename, "r");
    
    // checks if the log file has been opened successfully or not
    if (!file) {
        perror("File opening failed");
    
        return;
    }

    char buffer[CHUNK_SIZE];
    *count = 0;
    int line_number = 0;

    // read each line from the file
    while (fgets(buffer, sizeof(buffer), file)) {
        line_number++;
        char *pos = buffer;
    
        // search for the word in the current line
        while ((pos = strstr(pos, search_word)) != NULL) {
            (*count)++; // increment the counter if a match is found
            
            int position = pos - buffer + 1; // calculates the position of the word
            
            printf("Found in File: %s, Text: '%s' at line %d, position %d\n", filename, search_word, line_number, position);
    
            pos += strlen(search_word); // moves pointer to the end of the current matched word
        }
    }

    // clean up
    fclose(file);
}

int main(int argc, char* argv[]) {
    int rank, size, i, option;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char *files[MAX_FILES];
    int num_files = 0;
    int file_exists = 1; // flag to check if all files exist
    long file_sizes[MAX_FILES] = {0};
    char search_word[MAX_WORD_LENGTH] = {0}, replace_word[MAX_WORD_LENGTH] = {0}, regex_pattern[MAX_WORD_LENGTH] = {0};

    if (rank == 0) {
        // takes the numebr of files input from the suer
        printf("Enter the number of files (1 - %d): ", MAX_FILES);
        fflush(stdout);
        scanf("%d", &num_files);

        // checks if the file numebr is valid
        if (num_files < 1 || num_files > MAX_FILES) {
            printf("Invalid number of files. Please enter a number between 1 and %d.\n", MAX_FILES);
            
            MPI_Finalize();
    
            return 1;
        }

        // reads the file names 
        for (i = 0; i < num_files; i++) {
            files[i] = (char *) malloc(MAX_WORD_LENGTH * sizeof(char));
            
            if (!files[i]) {
                perror("Memory allocation failed");
                MPI_Finalize();
    
                return 1;
            }
            
            printf("Enter the name of file %d: ", i + 1);
            fflush(stdout);
            scanf("%s", files[i]);

            // check if the file exists
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

    // Send the file existence flag to all processes
    MPI_Bcast(&file_exists, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // if any file does not exist, terminate the program
    if (!file_exists) {
        if (rank == 0) {
            printf("Terminating program due to missing file(s).\n");
        }
    
        MPI_Finalize();
    
        return 1;
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

    // displays the information on the console and saves the data to a file
    for (i = 0; i < num_files; i++) {
        if (current_size >= start_size && current_size < end_size) {
            if (option == 1) {
                count(files[i], search_word, &local_count);
                log_operation("Count", search_word, files[i], local_count);            
                
                printf("File: %s, Count: %d\n", files[i], local_count);
            } else if (option == 2) {
                search_case_sensitive(files[i], search_word, &local_count);
                log_operation("Search (Case Sensitive)", search_word, files[i], local_count);
                
                printf("File: %s, Count: %d\n", files[i], local_count);
            }  else if (option == 3) {
                search_case_insensitive(files[i], search_word, &local_count);
                log_operation("Search (Case Insensitive)", search_word, files[i], local_count);
                
                printf("File: %s, Count: %d\n", files[i], local_count);
            } else if (option == 4) {
                search_regex(files[i], regex_pattern, &local_count);
                log_operation("Search (Regular Expression)", regex_pattern, files[i], local_count);
                
                printf("File: %s, Count: %d\n", files[i], local_count);
            }  else if (option == 5) {
                replace(files[i], search_word, replace_word, &local_count);
                log_operation_replace("Replace", replace_word, "Search", search_word, files[i], local_count);
                
                printf("File: %s, Count: %d, Search: %s, Replace: %s\n", files[i], local_count, search_word, replace_word);
            }
            
            total_count += local_count;
        }
        
        current_size += file_sizes[i];
    }

    int global_count;
    
    MPI_Reduce(&total_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        if (option == 1) {
            printf("Total occurrences of '%s': %d\n", search_word, global_count);
        } else if (option == 2) {
            printf("Total occurrences of '%s': %d\n", search_word, global_count);
        }  else if (option == 3) {
            printf("Total occurrences of '%s': %d\n", search_word, global_count);
        } else if (option == 4) {
            printf("Total occurrences of '%s': %d\n", regex_pattern, global_count);
        }  else if (option == 5) {
            printf("Total replacements of '%s' with '%s': %d\n", search_word, replace_word, global_count);
        }
    }

    // free allocated memory
    for (i = 0; i < num_files; i++) {
        free(files[i]);
    }

    MPI_Finalize();

    return 0;
}
