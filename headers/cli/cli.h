/**
 * @file cli.h
 * @brief Header for the Command-Line Interface (CLI) module.
 *
 * This module provides functions for interactive user input
 * to select simulation parameters and parse command-line arguments.
 */
#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

/**
 * @brief Structure to hold parsed command-line arguments.
 */
typedef struct {
    char* config_filepath;  // Path to configuration file
    bool verbose;           // Verbose mode flag
} CLIParams;

/**
 * @brief Parses command-line arguments.
 *
 * Supports the following options:
 *   -c, --config FILE : Path to configuration file (required)
 *   --verbose         : Enable verbose output (optional)
 *   -h, --help        : Display help message
 *
 * @param argc Argument count from main.
 * @param argv Argument vector from main.
 * @param params Pointer to CLIParams structure to be populated.
 * @return 0 on success, -1 on error or if help was displayed.
 */
int parse_arguments(int argc, char* argv[], CLIParams* params);

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
bool get_policy_input(char** policy_input);

#endif
 