#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>

typedef enum {
    NEW,  
    READY,
    RUNNING,
    TERMINATED 
} ProcessState;

typedef struct Process {
    char name[32];

    int arrival_time;
    int burst_time;
    int priority;
    int original_index;

    ProcessState state;
    int remaining_burst_time;
    int executed_time;
   
    int start_time;
    int finish_time;
    int waiting_time;
    int turnaround_time;
    int response_time;
   
    bool is_preempted;
    int last_executed_time;
    int current_quantum_runtime;
   
    int last_active_time;        
    int current_queue_level;     
    int time_spent_at_current_level;
} Process;

#endif