#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/policies/lifo.h"
#include "../headers/data_structures/process.h"
#include "../headers/parser/config_parser.h" // New include

int main() {
    printf("--- Testing LIFO Policy with config file ---\n");

    // 1. Create Policy
    Policy* policy = lifo_policy_create(0); // Quantum is ignored for LIFO
    if (!policy) {
        fprintf(stderr, "TEST FAILED: lifo_policy_create returned NULL.\n");
        return 1;
    }

    // 2. Parse processes from config file
    const char* config_filepath = "configs/test1.conf";
    int process_count = 0;
    Process* processes = parse_config_file(config_filepath, &process_count);

    if (processes == NULL) {
        fprintf(stderr, "TEST FAILED: Failed to parse config file '%s'.\n", config_filepath);
        lifo_policy_destroy(policy);
        return 1;
    }
    if (process_count == 0) {
        fprintf(stderr, "TEST FAILED: No processes parsed from config file '%s'.\n", config_filepath);
        free(processes);
        lifo_policy_destroy(policy);
        return 1;
    }

    // 3. Add parsed processes to the policy
    printf("Adding %d processes to LIFO policy from '%s':\n", process_count, config_filepath);
    for (int i = 0; i < process_count; i++) {
        printf("  Adding process: %s\n", processes[i].name);
        lifo_policy_add_process(policy, &processes[i]);
    }

    // 4. Retrieve processes and check order
    printf("Retrieving processes...\n");
    Process* next;
    int pass = 1;

    // Define expected order based on config1.conf processes added (P1-P6)
    // but retrieved in reverse order (LIFO)
    char* expected_order[] = {"P6", "P5", "P4", "P3", "P2", "P1"};
    int expected_count = sizeof(expected_order) / sizeof(expected_order[0]);

    if (process_count != expected_count) {
        fprintf(stderr, "TEST FAILED: Mismatch in process count. Expected %d, got %d.\n", expected_count, process_count);
        pass = 0;
    } else {
        for (int i = 0; i < expected_count; i++) {
            next = lifo_policy_get_next_process(policy);
            printf("Expected: %s, Got: %s\n", expected_order[i], next ? next->name : "(null)");
            if (!next || strcmp(next->name, expected_order[i]) != 0) {
                pass = 0;
                fprintf(stderr, "TEST FAILED: Process %s not in expected order.\n", expected_order[i]);
                break;
            }
        }
    }
    
    // 5. Check if stack is empty
    next = lifo_policy_get_next_process(policy);
    if (next != NULL) {
        pass = 0;
        fprintf(stderr, "TEST FAILED: Expected stack to be empty but got a process (%s).\n", next->name);
    } else {
        printf("Stack is empty as expected.\n");
    }

    // 6. Clean up
    free(processes); // Free memory allocated by parser
    lifo_policy_destroy(policy);

    if (pass) {
        printf("\nTEST PASSED: LIFO policy test complete.\n");
        return 0;
    } else {
        fprintf(stderr, "\nTEST FAILED: LIFO policy behavior is incorrect.\n");
        return 1;
    }
}
