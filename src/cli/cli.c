#include "../../headers/cli/cli.h"
#include "../../headers/policies/policies.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>

static void to_upper(char* str) {
    if (!str) return;
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

bool get_policy_input(char** policy_input) {
    register_all_policies();

    int policy_count = 0;
    const char** available_policies = get_available_policies(&policy_count);

    if (policy_count == 0) {
        fprintf(stderr, "CLI Error: No scheduling policies have been registered.\n");
        return false;
    }

    printf("\nAvailable scheduling policies:\n");
    for (int i = 0; i < policy_count; i++) {

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
    
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            break;
        } else {
            fprintf(stderr, "Invalid choice. Please try again.\n");
    
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }

    const char* selected_policy_name = available_policies[user_choice - 1];
    
    *policy_input = strdup(selected_policy_name);
    if (!*policy_input) {
        perror("CLI Error: Could not allocate memory for policy name");
        return false;
    }

    return true;
}

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

int parse_arguments(int argc, char* argv[], CLIParams* params) {
    params->config_filepath = NULL;
    params->verbose = false;

    const struct option long_options[] = {
        {"config",  required_argument, 0, 'c'},
        {"verbose", no_argument,       0, 'v'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

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

    if (!params->config_filepath) {
        fprintf(stderr, "Error: Configuration file is required.\n");
        print_usage(argv[0]);
        return -1;
    }

    if (access(params->config_filepath, R_OK) != 0) {
        perror("Error accessing config file");
        return -1;
    }

    return 0;
}
