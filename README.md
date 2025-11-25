# Linux Multi-Tasks Scheduler

**WORK IN PROGRESS:** This project is currently under development. The code is not yet fully implemented, but the architecture and specifications are well-defined. 

This project is a C-based multi-task scheduler simulator for a Linux-like environment. It allows users to experiment with different scheduling algorithms and analyze their performance based on a given set of processes.

## Features

The scheduler will support the following scheduling algorithms:

*   First-In, First-Out (FIFO)
*   Last-In, First-Out (LIFO)
*   Shortest Job First (SJF)
*   Shortest Remaining Time First (SRT)
*   Round Robin (RR)
*   Priority-based Scheduling
*   Multi-level Feedback Queues

## Project Architecture

The project is designed with a modular architecture, separating different concerns into the following components:

*   **Configuration Parser:** Reads process definitions from a configuration file.
*   **Scheduler Engine:** The core of the simulator, which manages the timeline, process states, and interacts with the scheduling policies.
*   **Policy Modules:** Each scheduling algorithm is implemented as a separate module that adheres to a common policy interface.
*   **Data Structures:** A set of generic data structures like queues, stacks, and heaps are used by the engine and policies.
*   **Output & Logging:** Generates output in the form of Gantt charts and logs to visualize the scheduling process and its results.

## Configuration File Format

The processes to be scheduled are defined in a configuration file. Each process is defined with its name, arrival time, burst time, and optionally, a priority.

**Example:**
```
process P1 {
    arrival_time = 0
    burst_time = 5
    priority = 3
}

process P2 {
    arrival_time = 2
    burst_time = 8
    priority = 1
}

process P3 {
    arrival_time = 4
    burst_time = 2
}
```

## Command-line Usage

The simulator will be executed from the command line with the following options:

```
./sim_scheduler -f <config_file> -p <policy> [--quantum <N>] [--out <dir>] [--verbose]
```

*   `-f <config_file>`: Path to the process configuration file.
*   `-p <policy>`: The scheduling policy to use (e.g., `fifo`, `sjf`, `rr`).
*   `--quantum <N>`: The time quantum for the Round Robin policy.
*   `--out <dir>`: The directory to save the output files.
*   `--verbose`: Enable verbose logging.

If the `-p` option is not provided, the simulator will display an interactive menu to choose from the available policies.

## Build and Installation

The project will be built using `make`. The `Makefile` is not yet fully implemented. The following targets will be available:

*   `make all`: Build the `sim_scheduler` executable.
*   `make clean`: Remove build artifacts.
*   `make test`: Run the test suite.

## Data Structures

The project uses a set of custom data structures:

*   **Process:** A struct to store process information (name, arrival time, burst time, priority, state, etc.).
*   **Queue:** A generic FIFO queue.
*   **Stack:** A generic LIFO stack.
*   **Heap:** A min-heap and a max-heap for implementing priority queues.

## Policy API

New scheduling policies can be added by creating a new C file in the `src/policies` directory and a corresponding header in `headers/policies`. Each policy must implement the interface defined in `headers/engine/policy_interface.h`.

## Output

The simulator will generate the following outputs:

*   **Gantt Chart:** An ASCII-based Gantt chart to visualize the execution of processes over time.
*   **Logs:** Detailed logs of scheduling events (process arrival, preemption, termination).
*   **Metrics:** Performance metrics such as average waiting time, average turnaround time, and throughput.

## Testing

The project will have a test suite to verify the correctness of the scheduling algorithms and the simulator's behavior. The tests will be run using the `make test` command.

## License

This project is licensed under the terms of the LICENSE file.
