# Agile Development Plan - Scheduling Simulator

This document outlines the product backlog and sprint plan for the development of the scheduling simulator project.

- **Team:** 5 Developers
- **Total Duration:** 4 Weeks
- **Methodology:** Scrum
- **Sprints:** 2 x 2-week sprints

---

## 1. Product Backlog

The backlog contains all features (Epics), user stories, and technical tasks required to complete the project. Items are ordered by priority.

| ID   | Type   | Description                                                                                                   | Priority      |
|:-----|:-------|:--------------------------------------------------------------------------------------------------------------|:--------------|
| PB-1 | Epic   | **Project Foundations:** Implement the core data structures and configuration parser.                           | **Must-Have** |
| PB-2 | Epic   | **Simulation Engine:** Create the core of the simulator, capable of managing time and processes.                | **Must-Have** |
| PB-3 | Epic   | **Scheduling Policies:** Implement the various algorithms (FIFO, RR, SJF, etc.).                              | **Must-Have** |
| PB-4 | Epic   | **Command-Line Interface (CLI):** Allow the user to run and configure the simulation.                         | **Must-Have** |
| PB-5 | Epic   | **Output and Reporting:** Display the simulation results (logs, Gantt chart, metrics).                        | **Must-Have** |
| PB-6 | Epic   | **Testing and Validation:** Ensure the simulator is robust and correct.                                       | **Should-Have** |
| PB-7 | Epic   | **Documentation:** Document the code and how to use the project.                                              | **Should-Have** |
| PB-8 | Epic   | **Advanced Features:** Enhance the user interface with a TUI (ncurses) or graphical plots.                    | **Could-Have**  |

---

## 2. Sprint 1: Building the Foundation (Weeks 1-2)

**Sprint Goal:** By the end of this sprint, the team will have a functional baseline simulator that can run a non-preemptive simulation (FIFO) from a config file and output basic logs.

### Sprint 1 Backlog:

| ID    | User Story / Task                                                                                                          | Priority  |
|:------|:---------------------------------------------------------------------------------------------------------------------------|:----------|
| S1-T1 | As a developer, I want to implement the `Process`, `Queue`, and `Stack` structs to have the project's basic building blocks. | **High**  |
| S1-T2 | As a user, I want to define a list of processes in a `.conf` file to provide input for the simulation. (Parser)          | **High**  |
| S1-T3 | As a developer, I want to define a `policy_interface.h` to standardize communication between the engine and policies.     | **High**  |
| S1-T4 | As a developer, I want a basic `Makefile` to compile the entire project easily.                                            | **High**  |
| S1-T5 | As a developer, I want a simple simulation engine that manages time and process states (NEW, READY, RUNNING, TERMINATED). | **Medium**|
| S1-T6 | As a user, I want to run the **FIFO** policy to validate the non-preemptive engine's functionality.                         | **Medium**|
| S1-T7 | As a developer, I want a simple `logger` module that outputs events (arrival, start/end) with the simulation time.      | **Low**   |
| S1-T8 | As a developer, I want to start setting up the `test_runner.sh` script with an initial test case for FIFO.               | **Low**   |

---

## 3. Sprint 2: Advanced Policies, Reporting & Polish (Weeks 3-4)

**Sprint Goal:** To complete all essential features, including preemptive policies and result reporting, and to ensure the project is robust, tested, and documented for final delivery.

### Sprint 2 Backlog:

| ID    | User Story / Task                                                                                                           | Priority  |
|:------|:----------------------------------------------------------------------------------------------------------------------------|:----------|
| S2-T1 | As a developer, I want to implement a `Heap` data structure for efficient priority-based policies.                          | **High**  |
| S2-T2 | As a user, I want to use the preemptive policies **RR** (Round Robin) and **SRT** (Shortest Remaining Time).                  | **High**  |
| S2-T3 | As a developer, I need to update the simulation engine to handle preemption logic.                                          | **High**  |
| S2-T4 | As a user, I want to see an ASCII **Gantt chart** and **performance metrics** (avg. waiting/turnaround time) at the end.      | **High**  |
| S2-T5 | As a user, I want to be able to run the simulator with command-line arguments (`-f`, `-p`, `-q`). (Finalize CLI)             | **Medium**|
| S2-T6 | As a developer, I want to expand the test suite to cover all policies and edge cases (preemption, etc.).                   | **Medium**|
| S2-T7 | As a developer and user, I want clear documentation (`README.md`, user guide) to understand and use the project.            | **Medium**|
| S2-T8 | **(Stretch Goal)** As a user, I want an interactive, real-time view of the simulation via `ncurses`.                       | **Low**   |
| S2-T9 | **(Review & Debugging)** Allocate time for code reviews, bug fixing, and validation with `valgrind`.                        | **High**  |

