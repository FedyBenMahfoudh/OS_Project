#include <stdio.h>
#include "../../headers/utils/utils.h"

void print_process_table(const Process* processes, int count) {
    if (!processes || count == 0) {
        printf("No processes to display.\n");
        return;
    }

    printf("\nSuccessfully parsed %d processes:\n", count);
    printf("-----------------------------------------------------\n");
    printf("| %-20s | Arrival | Burst | Priority |\n", "Name");
    printf("-----------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        printf("| %-20s | %7d | %5d | %8d |\n",
               processes[i].name,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].priority);
    }
    printf("-----------------------------------------------------\n");
}
