/**
 * @file cli.c
 * @brief Handles the command-line interface for interactive user input.
 *
 * This module is responsible for discovering available scheduling policies
 * and prompting the user to select one when required by the main application flow.
 */
#include "../../headers/cli/cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>


const char* headers_dir_path = "headers/policies";
const char* src_dir_path = "src/policies";


/**
 * @brief Converts a string to uppercase in-place.
 * @param str The string to modify.
 */
void to_upper(char* str) {
    for (int i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
}


/**
 * @brief Frees a dynamically allocated array of strings.
 *
 * Iterates through the array, freeing each string, and then frees the array itself.
 *
 * @param array The array of strings to free.
 * @param count The number of strings in the array.
 */
void free_array(char** array, int count) {
    for (int i = 0; i < count; i++) {
        free(array[i]);
    }

    free(array);
}


/**
 * @brief Scans policy directories to find valid, available scheduling policies.
 *
 * A policy is considered valid if it has both a .h file in the `headers/policies`
 * directory and a corresponding .c file in the `src/policies` directory.
 *
 * @param count A pointer to an integer that will be filled with the number of policies found.
 * @return A dynamically allocated array of strings containing the names of the policies.
 *         This array must be freed by the caller using `free_array()`. Returns NULL on failure.
 */
char** get_available_policies(int* count) {
    // Initializing the variables
    *count = 0;
    char** names = NULL;

    DIR* d = opendir(headers_dir_path);
    if (!d) {
        perror("Could not open headers/policies directory");
        return NULL;
    }

    struct dirent* dir;
    while ((dir = readdir(d)) != NULL) {
        char* filename = dir->d_name;
        char* dot = strrchr(filename, '.'); // gets the last occurence of the dot instead of the first (done by strchr).

        // The file must :
        //   - contain a dot
        //   - its extension must be .h
        if (dot && (strcmp(dot, ".h") == 0)) {
            
            
            char basename[100];
            size_t basename_len = dot - filename;
            
            // Preventing buffer overflow
            if (basename_len >= sizeof(basename)) {
                basename_len = sizeof(basename) - 1;
            } 

            // Copying the base name from the file name (and adding /0 to the end)
            strncpy(basename, filename, basename_len);
            basename[basename_len] = '\0';

            // Excluding the policies.h file
            if (strcmp(basename, "policies") == 0) {
                continue;
            }

            // Getting the filepath of the corresponding C file of the policy 
            char corresponding_c_filepath[512];
            snprintf(corresponding_c_filepath, sizeof(corresponding_c_filepath), "%s/%s.c", src_dir_path, basename);

            // Case : Corresponding policy C file exists 
            if (access(corresponding_c_filepath, F_OK) == 0) {
                (*count)++;
                names = realloc(names, (*count) * sizeof(char*));
                if (!names) {
                    perror("Failed to reallocate memory for policies");
                    closedir(d);
                    return NULL;
                }
                
                // Copying the name to the policies names array
                names[*count - 1] = malloc(basename_len + 1);
                if (!names[*count - 1]) {
                    perror("Failed to allocate memory for policy name string");
                    closedir(d);
                    free_array(names, *count - 1);
                    return NULL;
                }
                memcpy(names[*count - 1], basename, (basename_len + 1));
            }
        }
    }

    closedir(d);
    return names;
}


/**
 * @brief Prompts the user to select a scheduling policy through an interactive menu.
 *
 * This function discovers available policies, presents them to the user,
 * and records their choice. It allocates memory for the policy name,
 * which must be freed by the caller.
 *
 * @param policy_input A pointer to a char* that will be set to the
 *                     dynamically allocated name of the chosen policy.
 * @return true if the user successfully makes a selection, false otherwise.
 */
bool get_policy_input(char** policy_input) {
    // Getting the available policies to display them later in the dynamic menu
    int policy_count = 0;
    char** available_policies = get_available_policies(&policy_count);

    // Case : No available policies
    if (policy_count == 0) {
        fprintf(stderr, "CLI Error : No valid policies found.\n");
        free_array(available_policies, policy_count);
        return false;
    }

    // Prompting the user to select a policy
    int user_choice = -1;
    bool success = false;

    printf("\nHere are the available policies. Please select one :\n");
    for (int i = 0; i < policy_count; i++) {
        // Duplicating the name of the policy to print it in upper case. 
        char temp_name[100];
        strncpy(temp_name, available_policies[i], sizeof(temp_name) - 1);
        temp_name[sizeof(temp_name) - 1] = '\0';
        to_upper(temp_name);
        printf("  %d : %s\n", (i + 1), temp_name);
    }

    printf("Enter your choice (1 - %d): ", policy_count);
    // Case : The choice is invalid (Out of bounds)
    while ((scanf("%d", &user_choice) != 1) || (user_choice < 1) || (user_choice > policy_count)) {
        fprintf(stderr, "Invalid choice.\n");

        // Clearing the input buffer before reasking the user again for a value
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Enter your choice (1 - %d): ", policy_count);
    }

    // Allocating memory for chosen policy name and returning it 
    char* selected_policy = available_policies[user_choice - 1];
    size_t selected_policy_len = strlen(selected_policy) + 1;
    *policy_input = (char*) malloc(selected_policy_len);
    
    if (!(*policy_input)) {
        perror("CLI Error: Could not allocate memory for policy name.\n");
        goto cleanup;
    }

    memcpy(*policy_input, selected_policy, selected_policy_len);
    success = true;

    // Freeing the temporary list of names used by the helper function (get_available_policies)
cleanup:
    free_array(available_policies, policy_count);
    return success;
}


/**
 * @brief Displays usage information for the program.
 * @param prog_name The name of the program (argv[0]).
 */
void print_usage(const char* prog_name) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║       Linux Multi-Tasks Scheduler - Usage Information         ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Usage: %s -c <config_file> [OPTIONS]\n", prog_name);
    printf("\n");
    printf("Required Arguments:\n");
    printf("  -c, --config FILE    Path to the process configuration file\n");
    printf("\n");
    printf("Optional Arguments:\n");
    printf("  --verbose            Enable verbose output with detailed logs\n");
    printf("  -h, --help           Display this help message and exit\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -c configs/test1.conf\n", prog_name);
    printf("  %s --config configs/test1.conf --verbose\n", prog_name);
    printf("\n");
    printf("After starting, you will be prompted to select a scheduling policy\n");
    printf("from the available options discovered in your installation.\n");
    printf("\n");
}


/**
 * @brief Parses command-line arguments using getopt_long.
 *
 * @param argc Argument count from main.
 * @param argv Argument vector from main.
 * @param params Pointer to CLIParams structure to populate.
 * @return 0 on success, -1 on error or if help was displayed.
 */
int parse_arguments(int argc, char* argv[], CLIParams* params) {
    // Initialize parameters with default values
    params->config_filepath = NULL;
    params->verbose = false;

    // Defining long options for getopt_long
    const struct option long_options[] = {
        {"config",  required_argument, 0, 'c'},
        {"verbose", no_argument,       0, 'v'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    // Parsing command-line options
    while ((opt = getopt_long(argc, argv, "c:vh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'c':
                params->config_filepath = optarg;
                break;
            
            case 'v':
                params->verbose = true;
                break;
            
            case 'h':
                print_usage(argv[0]);
                return -1;  // Signal to exit (not an error, just showing help)
            
            case '?':
                // getopt_long already printed an error message
                fprintf(stderr, "\n");
                print_usage(argv[0]);
                return -1;
            
            default:
                print_usage(argv[0]);
                return -1;
        }
    }

    // Validating the required argument (config filepath)
    if (!params->config_filepath) {
        fprintf(stderr, "Error: Configuration file is required.\n");
        print_usage(argv[0]);
        return -1;
    }

    // Checking that the config file exists
    if (access(params->config_filepath, F_OK) != 0) {
        fprintf(stderr, "Error: Configuration file '%s' does not exist.\n", params->config_filepath);
        return -1;
    }

    // Checking that the config file is readable
    if (access(params->config_filepath, R_OK) != 0) {
        fprintf(stderr, "Error: Configuration file '%s' is not readable.\n", params->config_filepath);
        return -1;
    }

    return 0;  // Success
}
