#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h> 

#include "../headers/engine/scheduler_engine.h"


#define EPSILON 0.001f

void test_fifo_scheduler() {
    printf("--- Running Scheduler Engine Test (FIFO with test1.conf) ---\n");

    SimParameters params = {
        .config_filepath = "configs/test1.conf",
        .policy_name = "fifo",
        .quantum = 0 
    };

    SimulationResult* results = run_simulation(&params);

    assert(results != NULL);
    printf("  ✅ SimulationResult is not NULL.\n");
    assert(results->process_count == 6); 
    printf("  ✅ Correct number of processes in results (6).\n");
 
    printf("Asserting calculated metrics...\n");

    assert(fabs(results->average_turnaround_time - 12.0f) < EPSILON);
    printf("  ✅ Average Turnaround Time: %.2f (Expected: 12.00)\n", results->average_turnaround_time);

    assert(fabs(results->average_waiting_time - 7.833333f) < EPSILON); 
    printf("  ✅ Average Waiting Time: %.2f (Expected: 7.83)\n", results->average_waiting_time);

    assert(fabs(results->cpu_utilization - 100.0f) < EPSILON);
    printf("  ✅ CPU Utilization: %.2f%% (Expected: 100.00%%)\n", results->cpu_utilization);

    assert(results->gantt_chart != NULL);
    assert(results->gantt_event_count > 0);
    printf("  ✅ Gantt chart data is present.\n");

    free_simulation_results(results);
    printf("  ✅ SimulationResult memory freed.\n");

    printf("\nTEST PASSED: Scheduler Engine (FIFO) test complete.\n\n\n");
}

void test_lifo_scheduler() {
    printf("--- Running Scheduler Engine Test (LIFO with test_lifo.conf) ---\n");

    SimParameters params = {
        .config_filepath = "configs/test_lifo.conf",
        .policy_name = "lifo",
        .quantum = 0
    };

    SimulationResult* results = run_simulation(&params);

    assert(results != NULL);
    printf("  ✅ SimulationResult is not NULL.\n");
    assert(results->process_count == 3);
    printf("  ✅ Correct number of processes in results (3).\n");

    printf("Asserting calculated metrics...\n");
    assert(fabs(results->average_turnaround_time - 5.333333f) < EPSILON);
    printf("  ✅ Average Turnaround Time: %.2f (Expected: 5.33)\n", results->average_turnaround_time);
    assert(fabs(results->average_waiting_time - 2.333333f) < EPSILON);
    printf("  ✅ Average Waiting Time: %.2f (Expected: 2.33)\n", results->average_waiting_time);
    assert(results->gantt_chart != NULL);
    assert(results->gantt_event_count > 0);
    printf("  ✅ Gantt chart data is present.\n");
    
    free_simulation_results(results);
    printf("  ✅ SimulationResult memory freed.\n");

    printf("\nTEST PASSED: Scheduler Engine (LIFO) test complete.\n\n\n");
}

void test_sjf_scheduler() {
    printf("--- Running Scheduler Engine Test (SJF with test_sjf.conf) ---\n");

    SimParameters params = {
        .config_filepath = "configs/test_sjf.conf",
        .policy_name = "sjf",
        .quantum = 0
    };

    SimulationResult* results = run_simulation(&params);

    assert(results != NULL);
    printf("  ✅ SimulationResult is not NULL.\n");
    assert(results->process_count == 3);
    printf("  ✅ Correct number of processes in results (3).\n");

    printf("Asserting calculated metrics...\n");
    assert(fabs(results->average_turnaround_time - 5.333333f) < EPSILON);
    printf("  ✅ Average Turnaround Time: %.2f (Expected: 5.33)\n", results->average_turnaround_time);
    assert(fabs(results->average_waiting_time - 2.000000f) < EPSILON);
    printf("  ✅ Average Waiting Time: %.2f (Expected: 2.00)\n", results->average_waiting_time);
    assert(results->gantt_chart != NULL);
    assert(results->gantt_event_count > 0);
    printf("  ✅ Gantt chart data is present.\n");
    
    free_simulation_results(results);
    printf("  ✅ SimulationResult memory freed.\n");

    printf("\nTEST PASSED: Scheduler Engine (SJF) test complete.\n\n\n");
}

void test_priority_scheduler() {
    printf("--- Running Scheduler Engine Test (Priority with test_priority.conf) ---\n");

    SimParameters params = {
        .config_filepath = "configs/test_priority.conf",
        .policy_name = "priority",
        .quantum = 0
    };

    SimulationResult* results = run_simulation(&params);

    assert(results != NULL);
    printf("  ✅ SimulationResult is not NULL.\n");
    assert(results->process_count == 3);
    printf("  ✅ Correct number of processes in results (3).\n");

    printf("Asserting calculated metrics...\n");
    assert(fabs(results->average_turnaround_time - 7.000000f) < EPSILON);
    printf("  ✅ Average Turnaround Time: %.2f (Expected: 7.00)\n", results->average_turnaround_time);
    assert(fabs(results->average_waiting_time - 3.000000f) < EPSILON);
    printf("  ✅ Average Waiting Time: %.2f (Expected: 3.00)\n", results->average_waiting_time);
    assert(results->gantt_chart != NULL);
    assert(results->gantt_event_count > 0);
    printf("  ✅ Gantt chart data is present.\n");
    
    free_simulation_results(results);
    printf("  ✅ SimulationResult memory freed.\n");

    printf("\nTEST PASSED: Scheduler Engine (Priority) test complete.\n\n\n");
}


int main() {
    printf("--- Running All Scheduler Engine Tests ---\n\n");
    test_fifo_scheduler();
    test_lifo_scheduler();
    test_sjf_scheduler();
    test_priority_scheduler();
    printf("\nTEST PASSED: All Scheduler Engine tests completed.\n");
    return 0;
}
