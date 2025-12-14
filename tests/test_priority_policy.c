#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/policies/priority.h"
#include "../headers/data_structures/process.h"
#include "../headers/parser/config_parser.h"

int main() {
    printf("--- Testing Priority Policy with config file ---\n");

    Policy* policy = priority_policy_create(0); 
    if (!policy) {
        fprintf(stderr, "TEST FAILED: priority_policy_create returned NULL.\n");
        return 1;
    }

    const char* config_filepath = "configs/test1.conf";
    int process_count = 0;
    Process* processes = parse_config_file(config_filepath, &process_count);

    if (processes == NULL) {
        fprintf(stderr, "TEST FAILED: Failed to parse config file '%s'.\n", config_filepath);
        priority_policy_destroy(policy);
        return 1;
    }
    if (process_count == 0) {
        fprintf(stderr, "TEST FAILED: No processes parsed from config file '%s'.\n", config_filepath);
        free(processes);
        priority_policy_destroy(policy);
        return 1;
    }

    printf("Adding %d processes to Priority policy from '%s':\n", process_count, config_filepath);
    for (int i = 0; i < process_count; i++) {
        printf("  Adding process: %s (P: %d)\n", processes[i].name, processes[i].priority);
        priority_policy_add_process(policy, &processes[i]);
    }

    printf("Retrieving processes...\n");
    Process* next;
    int pass = 1;

    char* expected_order[] = {"P1", "P4", "P2", "P5", "P3", "P6"};
    int expected_count = sizeof(expected_order) / sizeof(expected_order[0]);

    if (process_count != expected_count) {
        fprintf(stderr, "TEST FAILED: Mismatch in process count. Expected %d, got %d.\n", expected_count, process_count);
        pass = 0;
    } else {
        for (int i = 0; i < expected_count; i++) {
            next = priority_policy_get_next_process(policy);
            printf("Expected: %s, Got: %s\n", expected_order[i], next ? next->name : "(null)");
            if (!next || strcmp(next->name, expected_order[i]) != 0) {
                pass = 0;
                fprintf(stderr, "TEST FAILED: Process %s not in expected order.\n", expected_order[i]);
                break;
            }
        }
    }

    next = priority_policy_get_next_process(policy);
    if (next != NULL) {
        pass = 0;
        fprintf(stderr, "TEST FAILED: Expected heap to be empty but got a process (%s).\n", next->name);
    } else {
        printf("Heap is empty as expected.\n");
    }

    free(processes); 
    priority_policy_destroy(policy);

    if (pass) {
        printf("\nTEST PASSED: Priority policy test complete.\n");
        return 0;
    } else {
        fprintf(stderr, "\nTEST FAILED: Priority policy behavior is incorrect.\n");
        return 1;
    }
}
