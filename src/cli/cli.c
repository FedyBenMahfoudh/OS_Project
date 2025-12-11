#include "../../headers/cli/cli.h"
#include "../../headers/policies/policies.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>

// Helper to convert string to uppercase for display
static void to_upper(char* str) {
    // Ensure str is not NULL before processing
    if (!str) return;
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

/**
 * @brief Prompts the user to select a scheduling policy from a dynamically generated menu.
 *
 * This function first ensures all compiled policies are registered. It then retrieves the
 * list of available policy names from the central registrar, displays them to the user,
 * and captures their selection.
 *
 * @param policy_input A pointer to a char* that will be set to the dynamically
 *                     allocated name of the chosen policy. The caller is responsible
 *                     for freeing this memory.
 * @return true if the user successfully makes a selection, false on error or if no
 *         policies are registered.
 */
bool get_policy_input(char** policy_input) {
    // Registering all available policies
    register_all_policies();

    int policy_count = 0;
    const char** available_policies = get_available_policies(&policy_count);

    if (policy_count == 0) {
        fprintf(stderr, "CLI Error: No scheduling policies have been registered.\n");
        return false;
    }

    printf("\nAvailable scheduling policies:\n");
    for (int i = 0; i < policy_count; i++) {
        // Using a temporary buffer for the uppercase version of the name
        char temp_name[100];
        strncpy(temp_name, available_policies[i], sizeof(temp_name) - 1);
        temp_name[sizeof(temp_name) - 1] = '\0';
        to_upper(temp_name);
        printf("  %d: %s\n", i + 1, temp_name);
    }

    int user_choice = -1;
    while (true) {
        printf("Enter your choice (1-%d): ", policy_count);
        if (scanf("%d", &user_choice) == 1 && user_choice >= 1 && user_choice <= policy_count) {
            // Clearing the rest of the input buffer after a valid number
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            break;
        } else {
            fprintf(stderr, "Invalid choice. Please try again.\n");
            // Clearing the entire buffer on invalid input
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }

    const char* selected_policy_name = available_policies[user_choice - 1];
    
    // Duplicating the string
    *policy_input = strdup(selected_policy_name);
    if (!*policy_input) {
        perror("CLI Error: Could not allocate memory for policy name");
        return false;
    }

    return true;
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
    // Initializing parameters (default values)
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
                return -1;
            
            case '?':
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

    // Checking that the config file exists and is readable
    if (access(params->config_filepath, R_OK) != 0) {
        perror("Error accessing config file");
        return -1;
    }

    return 0;
}
