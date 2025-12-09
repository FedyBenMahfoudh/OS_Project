#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/policies/sjf.h"
#include "../headers/data_structures/process.h"
#include "../headers/parser/config_parser.h"

int main() {
    printf("--- Testing SJF Policy with config file ---\n");

    // 1. Create Policy
    Policy* policy = sjf_policy_create(0); // Quantum is ignored for SJF
    if (!policy) {
        fprintf(stderr, "TEST FAILED: sjf_policy_create returned NULL.\n");
        return 1;
    }

    // 2. Parse processes from config file
    const char* config_filepath = "configs/test1.conf";
    int process_count = 0;
    Process* processes = parse_config_file(config_filepath, &process_count);

    if (processes == NULL) {
        fprintf(stderr, "TEST FAILED: Failed to parse config file '%s'.\n", config_filepath);
        sjf_policy_destroy(policy);
        return 1;
    }
    if (process_count == 0) {
        fprintf(stderr, "TEST FAILED: No processes parsed from config file '%s'.\n", config_filepath);
        free(processes);
        sjf_policy_destroy(policy);
        return 1;
    }

    // 3. Add parsed processes to the policy
    printf("Adding %d processes to SJF policy from '%s':\n", process_count, config_filepath);
    for (int i = 0; i < process_count; i++) {
        printf("  Adding process: %s (Burst: %d)\n", processes[i].name, processes[i].burst_time);
        sjf_policy_add_process(policy, &processes[i]);
    }

    // 4. Retrieve processes and check order
    printf("Retrieving processes...\n");
    Process* next;
    int pass = 1;

    // Define expected order based on config1.conf and SJF rules (Shorter burst time = Higher Priority)
    // P1(5), P2(8), P3(2), P4(3), P5(4), P6(3)
    // Expected: P3 (2), P4 (3), P6 (3), P5 (4), P1 (5), P2 (8)
    char* expected_order[] = {"P3", "P4", "P6", "P5", "P1", "P2"};
    int expected_count = sizeof(expected_order) / sizeof(expected_order[0]);

    if (process_count != expected_count) {
        fprintf(stderr, "TEST FAILED: Mismatch in process count. Expected %d, got %d.\n", expected_count, process_count);
        pass = 0;
    } else {
        for (int i = 0; i < expected_count; i++) {
            next = sjf_policy_get_next_process(policy);
            printf("Expected: %s, Got: %s\n", expected_order[i], next ? next->name : "(null)");
            if (!next || strcmp(next->name, expected_order[i]) != 0) {
                pass = 0;
                fprintf(stderr, "TEST FAILED: Process %s not in expected order.\n", expected_order[i]);
                break;
            }
        }
    }
    
    // 5. Check if heap is empty
    next = sjf_policy_get_next_process(policy);
    if (next != NULL) {
        pass = 0;
        fprintf(stderr, "TEST FAILED: Expected heap to be empty but got a process (%s).\n", next->name);
    } else {
        printf("Heap is empty as expected.\n");
    }

    // 6. Clean up
    free(processes); // Free memory allocated by parser
    sjf_policy_destroy(policy);

    if (pass) {
        printf("\nTEST PASSED: SJF policy test complete.\n");
        return 0;
    } else {
        fprintf(stderr, "\nTEST FAILED: SJF policy behavior is incorrect.\n");
        return 1;
    }
}
