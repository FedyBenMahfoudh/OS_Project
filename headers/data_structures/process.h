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
    int arrival_time; // Time at which the process arrives
    int burst_time; // Time required to complete the process
    int priority; // Priority of the process
    int original_index; // To preserve config file order

    // Process Followup Parameters (During Execution)
    ProcessState state;
    int remaining_burst_time; // Time remaining to complete the process
    int executed_time; // Time executed so far

    // Process Performance Metrics
    int start_time; // Start time of the first execution
    int finish_time; // End time of the execution
    int waiting_time; // Total time in the READY state
    int turnaround_time; // Total time in the system (Finish - Arrival)
    int response_time; // Total time in the system till the start (Start - arrival)

    // Process Preemption Tracking
    bool is_preempted;
    int last_executed_time; // Time at which the process was last executed
    int current_quantum_runtime; // Time running in the current time slice
    
    // MLFQ Tracking
    int last_active_time;         // Time at which the process was last active (Used for tracking aging)
    int current_queue_level;      // Current priority queue index
    int time_spent_at_current_level; // For anti-gaming allotment
} Process;

#endif