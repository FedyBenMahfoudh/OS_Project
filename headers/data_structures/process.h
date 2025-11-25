#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>

// Enumeration of the different states of the process
typedef enum {
    NEW,  
    READY,
    RUNNING,
    TERMINATED 
} ProcessState;


typedef struct Process {
    // Process Identifier (Name) 
    char name[32];

    // Process Initial Parameters
    int arrival_time;
    int burst_time;
    int priority;

    // Process Followup Parameters (During Execution)
    ProcessState state;
    int remaining_burst_time;
    int executed_time;

    // Process Performance Metrics
    int start_time; // Start time of the first execution
    int finish_time; // End time of the execution
    int waiting_time; // Total time in the READY state
    int turnaround_time; // Total time in the system (Finish - Arrival)
    int response_time; // Total time in the system till the start (Start - arrival)

    // Process Preemption Tracking
    bool is_preempted;
    int last_executed_time;
} Process;

#endif