#ifndef SCHEDULER_ENGINE_H
#define SCHEDULER_ENGINE_H

#include "../data_structures/process.h"
#include "policy_interface.h" 

#include <stdbool.h>


/**
 * @brief An event for building a Gantt chart.
 */
typedef struct {
    int time;
    char process_name[32];
} GanttEvent;


/**
 * @brief Callback function type for live simulation updates.
 * Called after each simulation tick to update UI.
 * @param current_time Current simulation time
 * @param all_processes Array of all processes
 * @param process_count Number of processes
 * @param running_process Currently running process (NULL if CPU idle)
 * @param gantt_events Current Gantt events
 * @param gantt_count Number of Gantt events
 */
typedef void (*SimulationTickCallback)(
    int current_time,
    Process* all_processes,
    int process_count,
    Process* running_process,
    GanttEvent* gantt_events,
    int gantt_count
);

/**
 * @brief Parameters for a simulation run, passed from main to the engine.
 */
typedef struct {
    const char* config_filepath;
    const char* policy_name;
    int quantum;
    bool verbose;
    SimulationTickCallback tick_callback;  // Optional: for live UI updates
} SimParameters;


/**
 * @brief All results from a completed simulation, returned to main.
 */
typedef struct {
    Process* processes;
    int process_count;
    float average_turnaround_time;
    float average_waiting_time;
    float cpu_utilization;
    GanttEvent* gantt_chart;
    int gantt_event_count;
} SimulationResult;


/**
 * @brief Initializes and executes the CPU scheduling simulation.
 * @param params A pointer to the SimParameters struct containing all simulation settings.
 * @return A pointer to a dynamically allocated SimulationResult structure, or NULL on error.
 */
SimulationResult* run_simulation(const SimParameters* params);


/**
 * @brief Frees all memory associated with a SimulationResult structure.
 * @param results The results structure to free.
 */
void free_simulation_results(SimulationResult* results);


#endif // SCHEDULER_ENGINE_H
