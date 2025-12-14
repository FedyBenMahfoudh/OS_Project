#ifndef SCHEDULER_ENGINE_H
#define SCHEDULER_ENGINE_H

#include "../data_structures/process.h"
#include "policy_interface.h" 

#include <stdbool.h>

typedef struct {
    int time;
    char process_name[32];
} GanttEvent;

typedef void (*SimulationTickCallback)(
    int current_time,
    Process* all_processes,
    int process_count,
    Process* running_process,
    GanttEvent* gantt_events,
    int gantt_count
);

typedef struct {
    const char* config_filepath;
    const char* policy_name;
    int quantum;
    bool verbose;
    SimulationTickCallback tick_callback; 
} SimParameters;

typedef struct {
    Process* processes;
    int process_count;
    float average_turnaround_time;
    float average_waiting_time;
    float cpu_utilization;
    GanttEvent* gantt_chart;
    int gantt_event_count;
} SimulationResult;

SimulationResult* run_simulation(const SimParameters* params);

void free_simulation_results(SimulationResult* results);

#endif
