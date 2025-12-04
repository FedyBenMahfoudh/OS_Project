/**
 * @file cli.h
 * @brief Header for the Command-Line Interface (CLI) module.
 *
 * This module provides functions for interactive user input
 * to select simulation parameters.
 */
#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

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
 