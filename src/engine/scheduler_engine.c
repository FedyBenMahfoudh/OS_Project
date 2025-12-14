#include "../../headers/engine/scheduler_engine.h"
#include "../../headers/parser/config_parser.h"
#include "../../headers/engine/policy_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int current_time;               
    Process* all_processes;         
    int total_process_count;        
    Process* running_process;       
    Policy* active_policy_handle;   
    int terminated_count;           
    GanttEvent* temp_gantt_chart;   
    int temp_gantt_event_count;     
    long long total_cpu_busy_time;  
    bool verbose_logging;           
} SimState;


static void initialize_sim_state(SimState* state, Process* processes, int count, Policy* policy_handle, bool verbose);
static void simulate_tick(SimState* state);
static void calculate_final_metrics(SimState* state, SimulationResult* results);
static void add_gantt_event_to_state(SimState* state, int time, const char* process_name);
static int compare_processes_by_arrival(const void* a, const void* b);

SimulationResult* run_simulation(const SimParameters* params) {
    SimulationResult* final_results = (SimulationResult*)calloc(1, sizeof(SimulationResult));
    if (!final_results) {
        perror("Scheduler Engine: Failed to allocate final_results struct");
        return NULL;
    }

    int parsed_process_count = 0;
    Process* parsed_processes = parse_config_file(params->config_filepath, &parsed_process_count);
    if (!parsed_processes || parsed_process_count == 0) {
        fprintf(stderr, "Scheduler Engine: Failed to parse config file '%s' or no processes found.\n", params->config_filepath);
        free(final_results);
        return NULL;
    }

    final_results->processes = parsed_processes;
    final_results->process_count = parsed_process_count;

    Policy* policy_handle = policy_create(params->policy_name, params->quantum);
    if (!policy_handle) {
        fprintf(stderr, "Scheduler Engine: Failed to create policy handle for '%s'.\n", params->policy_name);
        free_simulation_results(final_results);
        return NULL;
    }

    SimState state;
    memset(&state, 0, sizeof(SimState));
    initialize_sim_state(&state, parsed_processes, parsed_process_count, policy_handle, params->verbose);

    if (params->verbose) {
        printf("Scheduler Engine: Starting simulation for policy '%s' with %d processes :\n", params->policy_name, state.total_process_count);
    }

    while (state.terminated_count < state.total_process_count) {
        simulate_tick(&state);
        state.current_time++;

        if (params->tick_callback) {
            params->tick_callback(
                state.current_time,
                state.all_processes,
                state.total_process_count,
                state.running_process,
                state.temp_gantt_chart,
                state.temp_gantt_event_count
            );
        }
    }
    
    calculate_final_metrics(&state, final_results);
    
    final_results->gantt_chart = state.temp_gantt_chart;
    final_results->gantt_event_count = state.temp_gantt_event_count;

    policy_destroy(policy_handle);
    
    if (params->verbose) {
        printf("Scheduler Engine: Simulation finished at time %d.\n", state.current_time);
    }

    return final_results;
}

void free_simulation_results(SimulationResult* results) {
    if (!results) return;
    free(results->processes);
    free(results->gantt_chart);
    free(results);
}

static int compare_processes_by_arrival(const void* a, const void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;
    if (p1->arrival_time < p2->arrival_time) return -1;
    if (p1->arrival_time > p2->arrival_time) return 1;
    if (p1->original_index < p2->original_index) return -1;
    if (p1->original_index > p2->original_index) return 1;
    return 0;
}

static void initialize_sim_state(SimState* state, Process* processes, int count, Policy* policy_handle, bool verbose) {
    state->current_time = 0;
    state->all_processes = processes;
    state->total_process_count = count;
    state->verbose_logging = verbose;
    state->running_process = NULL;
    state->active_policy_handle = policy_handle;
    state->terminated_count = 0;
    state->temp_gantt_chart = NULL;
    state->temp_gantt_event_count = 0;
    state->total_cpu_busy_time = 0;

    for (int i = 0; i < count; i++) {
        processes[i].state = NEW;
        processes[i].remaining_burst_time = processes[i].burst_time;
        processes[i].current_quantum_runtime = 0; 
        processes[i].last_executed_time = 0;
    }

    qsort(state->all_processes, count, sizeof(Process), compare_processes_by_arrival);
}

static void add_gantt_event_to_state(SimState* state, int time, const char* process_name) {
    state->temp_gantt_event_count++;
    state->temp_gantt_chart = (GanttEvent*)realloc(state->temp_gantt_chart, state->temp_gantt_event_count * sizeof(GanttEvent));
    if (!state->temp_gantt_chart) {
        perror("Scheduler Engine: Failed to reallocate Gantt chart events");
        exit(EXIT_FAILURE);
    }
    GanttEvent* new_event = &state->temp_gantt_chart[state->temp_gantt_event_count - 1];
    new_event->time = time;
    strncpy(new_event->process_name, process_name, sizeof(new_event->process_name) - 1);
    new_event->process_name[sizeof(new_event->process_name) - 1] = '\0';
}

static void simulate_tick(SimState* state) {
    for (int i = 0; i < state->total_process_count; i++) {
    if (state->all_processes[i].state == NEW && state->all_processes[i].arrival_time == state->current_time) {
            state->all_processes[i].state = READY;
            policy_add_process(state->active_policy_handle, &state->all_processes[i]);
            if (state->verbose_logging) {
                printf("Time %d: Process %s arrived.\n", state->current_time, state->all_processes[i].name);
            }
        }
    }

    if (state->running_process != NULL) {
        int quantum = policy_get_quantum(state->active_policy_handle, state->running_process);
        if (quantum > 0 && state->running_process->current_quantum_runtime >= quantum) {
            if (state->verbose_logging) {
                printf("Time %d: Process %s quantum expired. Demoting.\n", state->current_time, state->running_process->name);
            }
            policy_demote_process(state->active_policy_handle, state->running_process);
            state->running_process = NULL; 
        }
    }

    bool should_reschedule = policy_needs_reschedule(state->active_policy_handle, state->running_process);
    if (should_reschedule) {
        Process* previously_running = state->running_process;
        if (previously_running != NULL) {
            previously_running->state = READY;
            policy_add_process(state->active_policy_handle, previously_running);
        }

        Process* next_process = policy_get_next_process(state->active_policy_handle);
        state->running_process = next_process;

        if (state->running_process != previously_running && state->running_process != NULL) {
            state->running_process->state = RUNNING;
            state->running_process->current_quantum_runtime = 0;

            if (state->running_process->start_time == 0 && state->current_time > 0) {
                state->running_process->start_time = state->current_time;
            }

            if (state->running_process->response_time == 0) {
                state->running_process->response_time = state->current_time - state->running_process->arrival_time;
            }
            
            if (state->verbose_logging) {
                printf("Time %d: Process %s starts running.\n", state->current_time, state->running_process->name);
            }
        }
    }

    if (state->running_process != NULL) {
        add_gantt_event_to_state(state, state->current_time, state->running_process->name);
        state->total_cpu_busy_time++;

        state->running_process->remaining_burst_time--;
        state->running_process->current_quantum_runtime++;
        state->running_process->last_executed_time = state->current_time + 1;
        
        policy_tick(state->active_policy_handle);

        if (state->running_process->remaining_burst_time == 0) {
            state->running_process->state = TERMINATED;
            state->running_process->finish_time = state->current_time + 1;
            state->running_process->turnaround_time = state->running_process->finish_time - state->running_process->arrival_time;
            state->running_process->waiting_time = state->running_process->turnaround_time - state->running_process->burst_time;
            state->terminated_count++;
            
            if (state->verbose_logging) {
                printf("Time %d: Process %s finished.\n", state->current_time + 1, state->running_process->name);
            }
            state->running_process = NULL;
        }
    } else {
        add_gantt_event_to_state(state, state->current_time, "IDLE");
    }
}

static void calculate_final_metrics(SimState* state, SimulationResult* results) {
    float total_turnaround_time = 0;
    float total_waiting_time = 0;
    int actual_completed_processes = 0;

    for (int i = 0; i < state->total_process_count; i++) {
        if (state->all_processes[i].state == TERMINATED) {
            total_turnaround_time += state->all_processes[i].turnaround_time;
            total_waiting_time += state->all_processes[i].waiting_time;
            actual_completed_processes++;
        }
    }

    if (actual_completed_processes > 0) {
        results->average_turnaround_time = total_turnaround_time / actual_completed_processes;
        results->average_waiting_time = total_waiting_time / actual_completed_processes;
    } else {
        results->average_turnaround_time = 0;
        results->average_waiting_time = 0;
    }

    if (state->current_time > 0) {
        results->cpu_utilization = (float)state->total_cpu_busy_time / state->current_time * 100.0f;
    } else {
        results->cpu_utilization = 0;
    }
}
