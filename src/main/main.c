#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../headers/cli/cli.h"
#include "../../headers/engine/scheduler_engine.h"
#include "../../headers/output/gantt_text.h"
#include "../../headers/parser/config_parser.h"
#include "../../headers/utils/utils.h"

int main(int argc, char* argv[]) {
    // 1. Parse command-line arguments
    CLIParams cli_params;
    if (parse_arguments(argc, argv, &cli_params) != 0) {
        return EXIT_FAILURE;
    }

    // printf("\n");
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║       Linux Multi-Tasks Scheduler Simulator       ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n");

    // 1.5 Parse and display processes
    int process_count = 0;
    Process* processes = parse_config_file(cli_params.config_filepath, &process_count);
    
    if (processes) {
        print_process_table(processes, process_count);
        
        // We free the processes here because run_simulation will parse them again
        // (Optimally we would pass them, but keeping it simple for now as per current architecture)
        free(processes);
    } else {
        // parsing failed, error message printed by parser
        return EXIT_FAILURE;
    }

    // 2. Get policy selection from user interactively
    char* selected_policy = NULL;
    if (!get_policy_input(&selected_policy)) {
        // Error message already printed by get_policy_input
        return EXIT_FAILURE;
    }

    // 3. Handle Round Robin quantum input
    int quantum = 0;
    if (strcmp(selected_policy, "rr") == 0) {
        printf("\nRound Robin selected.\n");
        printf("Enter time quantum (integer > 0): ");
        if (scanf("%d", &quantum) != 1 || quantum <= 0) {
            fprintf(stderr, "Error: Invalid quantum value. Must be a positive integer.\n");
            free(selected_policy);
            return EXIT_FAILURE;
        }
        // Consume newline left in buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF); 
    }

    // 4. Configure Simulation Parameters
    SimParameters sim_params;
    sim_params.config_filepath = cli_params.config_filepath;
    sim_params.policy_name = selected_policy;
    sim_params.quantum = quantum;
    sim_params.verbose = cli_params.verbose;
    sim_params.tick_callback = NULL;  // No live updates for CLI
    
    printf("\n");
    printf("--> Starting Simulation...\n");
    printf("    Config : %s\n", sim_params.config_filepath);
    printf("    Policy : %s\n", sim_params.policy_name);
    if (quantum > 0) printf("    Quantum: %d\n", sim_params.quantum);
    printf("-----------------------------------------------------\n\n");

    // 5. Run Simulation
    SimulationResult* results = run_simulation(&sim_params);

    if (results == NULL) {
        fprintf(stderr, "❌ Simulation failed to run (returned NULL).\n");
        free(selected_policy);
        return EXIT_FAILURE;
    }

    // 6. Display Results
    printf("✅ Simulation Completed!\n\n");
    printf("📊 Performance Metrics:\n");
    printf("   - Average Waiting Time    : %.2f units\n", results->average_waiting_time);
    printf("   - Average Turnaround Time : %.2f units\n", results->average_turnaround_time);
    printf("   - CPU Utilization         : %.2f %%\n", results->cpu_utilization);
    
    // Display Gantt chart (Always, unless empty)
    if (results->gantt_chart) {
        printf("\n📈 Gantt Chart:\n");
        print_gantt_chart(results);
    }

    // 7. Cleanup
    free_simulation_results(results);
    free(selected_policy);

    return EXIT_SUCCESS;
}