# Linux Multi-Tasks Scheduler

**PROJECT STATUS:** Core engine and scheduling algorithms are implemented and tested. Three interface versions are complete: CLI (command-line), TUI (terminal UI with ncurses), and GUI (graphical interface with GTK+3). The architecture and implementation are well-defined.

This project is a C-based multi-task scheduler simulator for a Linux-like environment. It allows users to experiment with different scheduling algorithms and analyze their performance with multiple visualization options.

## Features

### Available Interface Versions
*   **CLI Version:** Command-line interface with text output
*   **TUI Version:** Interactive terminal UI with real-time visualization using ncurses
*   **GUI Version:** Graphical interface with cards and charts using GTK+3

### Scheduling Algorithms

*   First-In, First-Out (FIFO)
*   Last-In, First-Out (LIFO)
*   Shortest Job First (SJF)
*   Priority-based Scheduling
*   Shortest Remaining Time First (SRT)
*   Round Robin (RR)
*   Multi-level Feedback Queues (MLFQ)

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

### CLI Version (Command-Line Interface)

The basic command-line interface provides text-based output with Gantt charts and metrics.

```bash
./scheduler -c <config_file> [--verbose]
```

**Arguments:**
*   `-c, --config <config_file>`: Path to the process configuration file (REQUIRED)
*   `--verbose`: Enable verbose logging and detailed Gantt chart display (OPTIONAL)
*   `-h, --help`: Display help message

**Interactive Policy Selection:**
When you run the program, an interactive menu will be displayed allowing you to choose from the available scheduling policies:
- FIFO (First In, First Out)
- LIFO (Last In, First Out)
- SJF (Shortest Job First)
- Priority-based Scheduling
- SRT (Shortest Remaining Time)
- RR (Round Robin) - will prompt for quantum value

**Example Usage:**
```bash
./scheduler -c configs/test1.conf
./scheduler -c configs/test1.conf --verbose
```

### TUI Version (Terminal User Interface)

The TUI version provides an interactive, real-time visualization using ncurses with the following features:

*   **Real-time Process Visualization:** Live view of running processes and ready queue
*   **Dynamic Gantt Chart:** Scrollable timeline showing process execution
*   **Performance Metrics:** Live updates of waiting time, turnaround time, and CPU utilization
*   **Interactive Controls:** 
    - `SPACE`: Pause/Resume simulation
    - `>`: Increase simulation speed
    - `<`: Decrease simulation speed
    - `R`: Restart simulation
    - `Q`: Quit

```bash
./tui_scheduler -c <config_file>
```

**Example Usage:**
```bash
./tui_scheduler -c configs/test1.conf
```

### GUI Version (Graphical User Interface)

The GUI version provides a modern GTK+3 interface with card-based layout:

*   **Process Information Table:** Detailed view of all processes with their states and metrics
*   **Execution Status Panel:** Visual cards showing active process and ready queue
*   **Gantt Chart:** Interactive timeline visualization
*   **Overall Metrics Dashboard:** 2x2 grid with performance statistics
*   **Control Buttons:** Start, Pause, Resume, Restart, and speed controls

```bash
./gui_scheduler -c <config_file>
```

**Example Usage:**
```bash
./gui_scheduler -c configs/test1.conf
```

## Build and Installation

The project is built using `make`. The following targets are available:

### Build Targets

*   `make all` or `make`: Build the `scheduler` executable (CLI version)
*   `make tui`: Build the `tui_scheduler` executable (TUI version with ncurses)
*   `make gui`: Build the `gui_scheduler` executable (GUI version with GTK+3)

### Test and Utility Targets

*   `make test`: Run the full test suite
*   `make clean`: Remove all build artifacts and executables
*   `make rebuild`: Clean and rebuild the CLI version
*   `make run`: Build and run the CLI version

### Quick Start

**CLI Version:**
```bash
make all          # Build CLI version
./scheduler -c configs/test1.conf
```

**TUI Version:**
```bash
make tui          # Build TUI version
./tui_scheduler -c configs/test1.conf
```

**GUI Version:**
```bash
make gui          # Build GUI version
./gui_scheduler -c configs/test1.conf
```

### Prerequisites

*   **For CLI version:** GCC compiler, standard C libraries
*   **For TUI version:** ncurses library (`libncurses-dev` on Debian/Ubuntu)
*   **For GUI version:** GTK+3 development libraries (`libgtk-3-dev` on Debian/Ubuntu)

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
