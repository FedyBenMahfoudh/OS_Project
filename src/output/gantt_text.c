#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../headers/output/gantt_text.h"

// Helper to find the index of a process in the processes array by name
static int get_process_index(const char* name, Process* processes, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(processes[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void print_gantt_chart(const SimulationResult* results) {
    if (!results || !results->gantt_chart || results->gantt_event_count == 0 || results->process_count == 0) {
        printf("No Gantt chart data available.\n");
        return;
    }

    // 1. Determine total duration
    int total_time = 0;
    for (int i = 0; i < results->process_count; i++) {
        if (results->processes[i].finish_time > total_time) {
            total_time = results->processes[i].finish_time;
        }
    }
    
    // Fallback if finishes not populated properly but events exist
    if (total_time == 0 && results->gantt_event_count > 0) {
        total_time = results->gantt_chart[results->gantt_event_count - 1].time;
    }

    // 2. Prepare grid
    char* grid = (char*)calloc(results->process_count * total_time, sizeof(char));
    if (!grid) {
        fprintf(stderr, "Memory allocation failed for Gantt grid.\n");
        return;
    }

    // 3. Fill grid
    for (int i = 0; i < results->gantt_event_count; i++) {
        int start_t = results->gantt_chart[i].time;
        int end_t = (i + 1 < results->gantt_event_count) ? results->gantt_chart[i+1].time : total_time;
        char* p_name = results->gantt_chart[i].process_name;
        
        int p_idx = get_process_index(p_name, results->processes, results->process_count);
        
        if (p_idx != -1 && start_t < total_time) {
             for (int t = start_t; t < end_t && t < total_time; t++) {
                grid[p_idx * total_time + t] = 1;
             }
        }
    }

    // 4. Print Improved Chart
    // Design:
    //            00 01 02 03
    //            |--|--|--|
    // Name       |██|  |  |
    //            +--+--+--+

    // A. Print Header (Time Ticks)
    printf("           "); // 11 spaces padding
    for (int t = 0; t <= total_time; t++) {
        printf("%02d ", t); // 3 chars width per tick
    }
    printf("\n");

    // B. Ruler / Top Separator
    printf("           ");
    for (int t = 0; t < total_time; t++) {
        printf("|--");
    }
    printf("|\n");

    // C. Rows
    for (int p = 0; p < results->process_count; p++) {
        printf("%-10s ", results->processes[p].name); // 10 chars + 1 space = 11 chars
        
        for (int t = 0; t < total_time; t++) {
            printf("|");
            
            if (grid[p * total_time + t]) {
                printf("██"); 
            } else {
                printf("  ");
            }
        }
        printf("|\n"); // Closure

        // D. Separator between rows
        printf("           ");
        for (int t = 0; t < total_time; t++) {
            printf("+--");
        }
        printf("+\n");
    }

    free(grid);
}
