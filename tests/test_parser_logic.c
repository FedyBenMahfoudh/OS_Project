#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../headers/parser/config_parser.h"
#include "../headers/data_structures/process.h"


void check_process(const Process* p, const char* name, int arrival, int burst, int priority) {
    assert(p != NULL);
    assert(strcmp(p->name, name) == 0);
    assert(p->arrival_time == arrival);
    assert(p->burst_time == burst);
    assert(p->priority == priority);
    printf("  ✅ Verified Process %s\n", name);
}

int main() {
    printf("--- Running Automated Parser Logic Test ---\n");
    const char* config_filepath = "configs/test1.conf";
    int process_count = 0;

    
    Process* processes = parse_config_file(config_filepath, &process_count);

    
    printf("Asserting parser results...\n");
    assert(processes != NULL);
    assert(process_count == 6);
    printf("  ✅ Correct number of processes parsed (6).\n");

    
    check_process(&processes[0], "P1", 0, 5, 3);
    check_process(&processes[1], "P2", 2, 8, 1);
    
    
    check_process(&processes[2], "P3", 4, 2, 0); 
    
    check_process(&processes[3], "P4", 6, 3, 2);
    check_process(&processes[4], "P5", 6, 4, 1);
    check_process(&processes[5], "P6", 8, 3, 0);

    
    free(processes);

    printf("\nTEST PASSED: Parser correctly processed 'test1.conf'.\n");
    return 0;
}
