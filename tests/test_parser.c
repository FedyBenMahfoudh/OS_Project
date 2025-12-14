#include <stdio.h>
#include <stdlib.h>
#include "../headers/parser/config_parser.h"
#include "../headers/data_structures/process.h"


int main(int argc, char *argv[]) {
    
    const char* config_filepath = "configs/test1.conf";
    
    
    if (argc > 1) {
        config_filepath = argv[1];
    }

    printf("--- Testing Parser with Config File: %s ---\n", config_filepath);

    int process_count = 0;
    Process* processes = parse_config_file(config_filepath, &process_count);

    if (processes == NULL) {
        fprintf(stderr, "TEST FAILED: The parser returned NULL.\n");
        return 1;
    }

    
    if (process_count <= 0) {
        fprintf(stderr, "TEST FAILED: The parser returned 0 or fewer processes.\n");
        free(processes);
        return 1;
    }

    printf("\nSuccessfully parsed %d processes:\n", process_count);
    printf("--------------------------------------------------\n");
    printf("| %-10s | Arrival | Burst | Priority |\n", "Name");
    printf("--------------------------------------------------\n");

    for (int i = 0; i < process_count; i++) {
        printf("| %-10s | %7d | %5d | %8d |\n",
               processes[i].name,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].priority);
    }
    printf("--------------------------------------------------\n");

    
    free(processes);
    processes = NULL;

    printf("\nTEST PASSED: Parser test complete and memory freed.\n");

    return 0;
}
