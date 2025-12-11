#include "../../headers/engine/scheduler_engine.h"
#include "../../headers/parser/config_parser.h"
#include "../../headers/engine/policy_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/**
 * @brief Internal structure to maintain the simulation's current state.
 */
typedef struct {
    int current_time;                   /**< The current simulated time. */
    Process* all_processes;             /**< Array of all processes in the simulation. */
    int total_process_count;            /**< Total number of processes. */
    Process* running_process;           /**< Pointer to the process currently executing on the CPU. */
    Policy* active_policy_handle;       /**< Handle to the active scheduling policy. */
    int terminated_count;               /**< Number of processes that have completed execution. */
    GanttEvent* temp_gantt_chart;       /**< Dynamically allocated array for Gantt chart events. */
    int temp_gantt_event_count;         /**< Current number of events in the Gantt chart. */
    long long total_cpu_busy_time;      /**< Total time the CPU has been busy (not idle). */
    bool verbose_logging;               /**< Flag to enable/disable verbose output during simulation. */
} SimState;


/**
 * @brief Helper Function Prototypes.
 * Doxygen for these functions are with their definitions.
 */
static void initialize_sim_state(SimState* state, Process* processes, int count, Policy* policy_handle, bool verbose);
static void simulate_tick(SimState* state);
static void calculate_final_metrics(SimState* state, SimulationResult* results);
static void add_gantt_event_to_state(SimState* state, int time, const char* process_name);
static int compare_processes_by_arrival(const void* a, const void* b);


/**
 * @brief Runs the CPU scheduling simulation based on provided parameters.
 *
 * This is the main entry point for the simulation. It parses the configuration,
 * initializes the chosen scheduling policy, simulates the execution tick by tick,
 * and calculates the final performance metrics.
 *
 * @param params A pointer to a SimParameters structure containing simulation configuration.
 * @return A pointer to a SimulationResult structure containing detailed results, or NULL if an error occurs.
 */
SimulationResult* run_simulation(const SimParameters* params) {
    // Allocating the final SimulationResult structure early
    SimulationResult* final_results = (SimulationResult*)calloc(1, sizeof(SimulationResult));
    if (!final_results) {
        perror("Scheduler Engine: Failed to allocate final_results struct");
        return NULL;
    }

    // Parsing Configuration File
    int parsed_process_count = 0;
    Process* parsed_processes = parse_config_file(params->config_filepath, &parsed_process_count);
    if (!parsed_processes || parsed_process_count == 0) {
        fprintf(stderr, "Scheduler Engine: Failed to parse config file '%s' or no processes found.\n", params->config_filepath);
        free(final_results);
        return NULL;
    }
    
    // Linking parsed processes to the final_results
    final_results->processes = parsed_processes;
    final_results->process_count = parsed_process_count;

    // Creating the policy instance
    Policy* policy_handle = policy_create(params->policy_name, params->quantum);
    if (!policy_handle) {
        fprintf(stderr, "Scheduler Engine: Failed to create policy handle for '%s'.\n", params->policy_name);
        free_simulation_results(final_results);
        return NULL;
    }

    // Initializing the simulation state
    SimState state;
    memset(&state, 0, sizeof(SimState));
    initialize_sim_state(&state, parsed_processes, parsed_process_count, policy_handle, params->verbose);

    if (params->verbose) {
        printf("Scheduler Engine: Starting simulation for policy '%s' with %d processes :\n", params->policy_name, state.total_process_count);
    }

    while (state.terminated_count < state.total_process_count) {
        simulate_tick(&state);
        state.current_time++;
        
        // Calling the live update callback if provided (For the TUI and GUI versions)
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

/**
 * @brief Frees all dynamically allocated memory within a SimulationResult structure.
 *
 * This function is crucial for preventing memory leaks after a simulation run.
 * It frees the array of processes, the Gantt chart events, and the result structure itself.
 *
 * @param results A pointer to the SimulationResult structure to be freed.
 */
void free_simulation_results(SimulationResult* results) {
    if (!results) return;
    free(results->processes);
    free(results->gantt_chart);
    free(results);
}


/**
 * @brief Comparison function for sorting processes based on their arrival time.
 *
 * Used by `qsort` to ensure processes are handled in the order they arrive.
 * Ties are broken by the original index to maintain a stable sort.
 *
 * @param a Pointer to the first Process.
 * @param b Pointer to the second Process.
 * @return Negative if a comes before b, positive if a comes after b, zero if equal.
 */
static int compare_processes_by_arrival(const void* a, const void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;
    if (p1->arrival_time < p2->arrival_time) return -1;
    if (p1->arrival_time > p2->arrival_time) return 1;
    // Tie-break with original index to maintain config order
    if (p1->original_index < p2->original_index) return -1;
    if (p1->original_index > p2->original_index) return 1;
    return 0;
}

/**
 * @brief Initializes the simulation state before execution begins.
 *
 * Sets up initial values for current time, process counts, policy handle,
 * and resets all process-specific metrics. It also sorts processes by arrival time.
 *
 * @param state A pointer to the SimState structure to initialize.
 * @param processes An array of all processes loaded from the configuration.
 * @param count The number of processes in the array.
 * @param policy_handle A handle to the active scheduling policy.
 * @param verbose A boolean indicating whether verbose logging is enabled.
 */
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

    // Initializing all processes (NEW state + remaining burst time + current quantum runtime + last executed time)
    for (int i = 0; i < count; i++) {
        processes[i].state = NEW;
        processes[i].remaining_burst_time = processes[i].burst_time;
        processes[i].current_quantum_runtime = 0; 
        processes[i].last_executed_time = 0;
    }

    // Sorting processes by arrival time
    qsort(state->all_processes, count, sizeof(Process), compare_processes_by_arrival);
}

/**
 * @brief Adds a new event to the dynamically growing Gantt chart.
 *
 * This helper function reallocates memory for the Gantt chart and
 * stores the process name and time at which it was running.
 *
 * @param state A pointer to the SimState structure.
 * @param time The current simulation time for the event.
 * @param process_name The name of the process running at this time (or "IDLE").
 */
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


/**
 * @brief Simulates a single tick of the CPU.
 *
 * This function encapsulates the core logic for advancing the simulation by one time unit.
 * It handles process arrivals, quantum expiry, preemption logic, process execution,
 * and state updates.
 *
 * @param state A pointer to the SimState structure to update.
 */
static void simulate_tick(SimState* state) {
    // 1. Handle Process Arrivals
    for (int i = 0; i < state->total_process_count; i++) {
    if (state->all_processes[i].state == NEW && state->all_processes[i].arrival_time == state->current_time) {
            state->all_processes[i].state = READY;
            policy_add_process(state->active_policy_handle, &state->all_processes[i]);
            if (state->verbose_logging) {
                printf("Time %d: Process %s arrived.\n", state->current_time, state->all_processes[i].name);
            }
        }
    }

    // 2. Handle Quantum Expiry Preemption
    if (state->running_process != NULL) {
        int quantum = policy_get_quantum(state->active_policy_handle, state->running_process);
        if (quantum > 0 && state->running_process->current_quantum_runtime >= quantum) {
            if (state->verbose_logging) {
                printf("Time %d: Process %s quantum expired. Demoting.\n", state->current_time, state->running_process->name);
            }
            policy_demote_process(state->active_policy_handle, state->running_process);
            // CPU becomes free
            state->running_process = NULL; 
        }
    }

    // 3. Handle Priority-Based Preemption or Select New Process
    bool should_reschedule = policy_needs_reschedule(state->active_policy_handle, state->running_process);
    if (should_reschedule) {
        Process* previously_running = state->running_process;
        if (previously_running != NULL) {
            previously_running->state = READY;
            policy_add_process(state->active_policy_handle, previously_running);
        }

        // Selecting the next process to run in the CPU
        Process* next_process = policy_get_next_process(state->active_policy_handle);
        state->running_process = next_process;

        if (state->running_process != previously_running && state->running_process != NULL) {
            state->running_process->state = RUNNING;
            state->running_process->current_quantum_runtime = 0;
            
            // Setting the start_time when process first starts executing
            if (state->running_process->start_time == 0 && state->current_time > 0) {
                state->running_process->start_time = state->current_time;
            }
            // Setting the response_time when process first gets CPU (first time running)
            if (state->running_process->response_time == 0) {
                state->running_process->response_time = state->current_time - state->running_process->arrival_time;
            }
            
            if (state->verbose_logging) {
                printf("Time %d: Process %s starts running.\n", state->current_time, state->running_process->name);
            }
        }
    }

    // 4. Execute Tick for the Running Process
    if (state->running_process != NULL) {
        add_gantt_event_to_state(state, state->current_time, state->running_process->name);
        state->total_cpu_busy_time++;
        
        // Updating the process properties
        state->running_process->remaining_burst_time--;
        state->running_process->current_quantum_runtime++;
        state->running_process->last_executed_time = state->current_time + 1;
        
        policy_tick(state->active_policy_handle);

        // Checking if the process has finished
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

/**
 * @brief Calculates and populates final simulation metrics into the results structure.
 *
 * This function computes average turnaround time, average waiting time, and CPU utilization
 * after the simulation has completed.
 *
 * @param state A pointer to the final SimState structure.
 * @param results A pointer to the SimulationResult structure to populate with metrics.
 */
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
