#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include "../../headers/cli/cli.h"
#include "../../headers/engine/scheduler_engine.h"
#include "../../headers/parser/config_parser.h"

// Window pointers for different panels
WINDOW *title_win;
WINDOW *process_info_win;        // Left: Basic process info
WINDOW *status_win;              // Right: Running + Ready queue
WINDOW *gantt_win;               // Center: Gantt chart
WINDOW *performance_win;         // Bottom-left: Performance metrics
WINDOW *metrics_win;             // Bottom-right: Overall metrics
WINDOW *control_win;             // Bottom: Controls

// Global state for TUI
typedef struct {
    bool paused;
    int speed_ms;
    bool should_quit;
    bool should_restart;
    int gantt_offset;
} TUIState;

TUIState tui_state = {false, 1000, false, false, 0};

// Initialize ncurses and create windows
void init_tui() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);  // Non-blocking input
    curs_set(0);  // Hide cursor
    
    // Enable colors
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Running
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Ready
        init_pair(3, COLOR_RED, COLOR_BLACK);     // Terminated
        init_pair(4, COLOR_CYAN, COLOR_BLACK);    // Header
        init_pair(5, COLOR_WHITE, COLOR_BLUE);    // Status bar
    }
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // New Layout:
    // Row 0-2:     Title (centered, 3 lines)
    // Row 3-16:    Left: Process Info (14 lines) | Right: Running/Queue (14 lines) [+5 rows]
    // Row 17-(max_y-15): Gantt Chart [-10 rows]
    // Row (max_y-14)-(max_y-4): Left: Performance (11 lines) | Right: Metrics (11 lines) [+5 rows]
    // Row (max_y-3)-(max_y-1): Controls (3 lines)
    
    int half_width = max_x / 2;
    int gantt_start = 17;
    int gantt_height = max_y - 32;  
    int perf_start = gantt_start + gantt_height;
    
    title_win = newwin(3, max_x, 0, 0);
    process_info_win = newwin(14, half_width, 3, 0);        
    status_win = newwin(14, half_width, 3, half_width);     
    gantt_win = newwin(gantt_height, max_x, gantt_start, 0);
    performance_win = newwin(11, half_width, perf_start, 0); 
    metrics_win = newwin(11, half_width, perf_start, half_width); 
    control_win = newwin(3, max_x, max_y - 3, 0);
    
    refresh();
}
// Cleanup ncurses
void cleanup_tui() {
    delwin(title_win);
    delwin(process_info_win);
    delwin(status_win);
    delwin(gantt_win);
    delwin(performance_win);
    delwin(metrics_win);
    delwin(control_win);
    endwin();
}
// Draw centered title
void draw_title(const char* policy, int current_time) {
    werase(title_win);
    box(title_win, 0, 0);
    
    int max_x;
    getmaxyx(title_win, (int){0}, max_x);
    
    char title[128];
    snprintf(title, sizeof(title), "CPU Scheduler Simulator - Policy: %s - Time: %d", 
             policy, current_time);
    
    int title_x = (max_x - strlen(title)) / 2;
    
    wattron(title_win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(title_win, 1, title_x > 0 ? title_x : 2, "%s", title);
    wattroff(title_win, COLOR_PAIR(4) | A_BOLD);
    
    wrefresh(title_win);
}
// Draw process basic info (left panel)
void draw_process_info(Process* processes, int count) {
    werase(process_info_win);
    box(process_info_win, 0, 0);
    
    mvwprintw(process_info_win, 0, 2, "[ PROCESS INFO ]");
    
    // Header
    wattron(process_info_win, A_BOLD);
    mvwprintw(process_info_win, 1, 2, "Name   Arr Bst Pri Rem Exec");
    wattroff(process_info_win, A_BOLD);
    
    // Process rows (limit to 6 visible)
    for (int i = 0; i < count && i < 6; i++) {
        int color = COLOR_PAIR(0);
        if (processes[i].state == RUNNING) color = COLOR_PAIR(1);
        else if (processes[i].state == READY) color = COLOR_PAIR(2);
        else if (processes[i].state == TERMINATED) color = COLOR_PAIR(3);
        
        int executed = processes[i].burst_time - processes[i].remaining_burst_time;
        
        wattron(process_info_win, color);
        mvwprintw(process_info_win, 2 + i, 2, 
                  "%-6s %3d %3d %3d %3d %4d",
                  processes[i].name,
                  processes[i].arrival_time,
                  processes[i].burst_time,
                  processes[i].priority,
                  processes[i].remaining_burst_time,
                  executed);
        wattroff(process_info_win, color);
    }
    
    wrefresh(process_info_win);
}
// Draw status panel (right panel - running + ready queue)
void draw_status(Process* running, Process* all_procs, int count) {
    werase(status_win);
    box(status_win, 0, 0);
    
    mvwprintw(status_win, 0, 2, "[ STATUS ]");
    
    // Running process
    wattron(status_win, A_BOLD);
    mvwprintw(status_win, 1, 2, "Running:");
    wattroff(status_win, A_BOLD);
    
    if (running) {
        wattron(status_win, COLOR_PAIR(1));
        mvwprintw(status_win, 2, 2, "%s (%d/%d)",
                  running->name,
                  running->burst_time - running->remaining_burst_time,
                  running->burst_time);
        wattroff(status_win, COLOR_PAIR(1));
    } else {
        wattron(status_win, A_DIM);
        mvwprintw(status_win, 2, 2, "CPU IDLE");
        wattroff(status_win, A_DIM);
    }
    
    // Ready queue
    wattron(status_win, A_BOLD);
    mvwprintw(status_win, 4, 2, "Ready Queue:");
    wattroff(status_win, A_BOLD);
    
    // Calculate how many rows we can display
    int max_y;
    getmaxyx(status_win, max_y, (int){0});
    int max_queue_y = max_y - 2;
    
    int y = 5;
    int ready_count = 0;
    for (int i = 0; i < count && y < max_queue_y; i++) {
        if (all_procs[i].state == READY) {
            wattron(status_win, COLOR_PAIR(2));
            mvwprintw(status_win, y++, 2, "* %s (p:%d)",
                      all_procs[i].name, all_procs[i].priority);
            wattroff(status_win, COLOR_PAIR(2));
            ready_count++;
        }
    }
    
    if (ready_count == 0) {
        wattron(status_win, A_DIM);
        mvwprintw(status_win, 5, 2, "(empty)");
        wattroff(status_win, A_DIM);
    }
    
    wrefresh(status_win);
}
// Draw Gantt chart with all processes as rows
void draw_gantt_chart(GanttEvent* events, int event_count, Process* all_procs, 
                      int proc_count, int current_time, bool is_final) {
    werase(gantt_win);
    box(gantt_win, 0, 0);
    
    mvwprintw(gantt_win, 0, 2, "[ GANTT CHART - Live Execution ]");
    
    if (proc_count == 0) return;
    
    int max_x;
    getmaxyx(gantt_win, (int){0}, max_x);
    
    // Calculate how many time units we can display
    int time_offset = 12;  // Space for process names
    int time_width = 3;    // 3 chars per time unit (handles up to 2-digit numbers + space)
    int max_display_time = (max_x - time_offset - 3) / time_width;
    int display_end = current_time < max_display_time ? current_time : max_display_time;
    for (int t = tui_state.gantt_offset; t <= display_end + tui_state.gantt_offset && t <= current_time; t++) {
        int display_pos = t - tui_state.gantt_offset;
        if (display_pos >= 0 && display_pos <= max_display_time) {
            mvwprintw(gantt_win, 1, time_offset + 6 + display_pos * time_width, "%2d ", t);
        }
    }

    for (int t = tui_state.gantt_offset; t <= display_end + tui_state.gantt_offset; t++) {
        int display_pos = t - tui_state.gantt_offset;
        if (display_pos >= 0 && display_pos <= max_display_time) {
            mvwprintw(gantt_win, 2, time_offset + 6 + display_pos * time_width, "---");
        }
    }
    // Draw each process as a row using original_index and sorting
    // Create temp array
    Process** sorted_procs = malloc(sizeof(Process*) * proc_count);
    for (int i=0; i<proc_count; i++) sorted_procs[i] = &all_procs[i];
    
    // Sort
    for (int i=0; i<proc_count-1; i++) {
        for (int j=0; j<proc_count-i-1; j++) {
            if (sorted_procs[j]->original_index > sorted_procs[j+1]->original_index) {
                Process* temp = sorted_procs[j];
                sorted_procs[j] = sorted_procs[j+1];
                sorted_procs[j+1] = temp;
            }
        }
    }
    for (int p = 0; p < proc_count && p < 14; p++) {  // Limit to available height
        int row = 3 + p;
        Process* current_p = sorted_procs[p];
        
        // Process name
        int color = COLOR_PAIR(0);
        if (current_p->state == RUNNING) color = COLOR_PAIR(1);
        else if (current_p->state == READY) color = COLOR_PAIR(2);
        else if (current_p->state == TERMINATED) color = COLOR_PAIR(3);
        
        wattron(gantt_win, color);
        mvwprintw(gantt_win, row, 2, "%-8s |", current_p->name);
        wattroff(gantt_win, color);
        
        // Draw execution timeline for this process
        for (int t = tui_state.gantt_offset; t <= display_end + tui_state.gantt_offset && t <= current_time; t++) {
            int display_pos = t - tui_state.gantt_offset;
            if (display_pos < 0 || display_pos > max_display_time) continue;
            // Check if this process was running at time t
            bool was_running = false;
            
            for (int e = 0; e < event_count; e++) {
                int event_time = events[e].time;
                int next_time = (e + 1 < event_count) ? events[e + 1].time : (is_final ? current_time : current_time + 1);
                                if (strcmp(events[e].process_name, current_p->name) == 0 &&
                    t >= event_time && t < next_time) {
                    was_running = true;
                    break;
                }
            }
            
            if (was_running) {
                wattron(gantt_win, COLOR_PAIR(1));
                mvwprintw(gantt_win, row, time_offset + 6 + display_pos * time_width, " # ");
                wattroff(gantt_win, COLOR_PAIR(1));
            } else {
                mvwprintw(gantt_win, row, time_offset + 6 + display_pos * time_width, " . ");
            }
        }
    }
    free(sorted_procs);
    
    wrefresh(gantt_win);
}


// Draw performance metrics (bottom-left panel)
void draw_performance_metrics(Process* processes, int count) {
    werase(performance_win);
    box(performance_win, 0, 0);
    
    mvwprintw(performance_win, 0, 2, "[ PROCESS PERFORMANCE ]");
    
    // Header
    wattron(performance_win, A_BOLD);
    mvwprintw(performance_win, 1, 2, "Name   Start Finish Wait  TAT   Resp");
    wattroff(performance_win, A_BOLD);
    
    // Process rows - show up to 3 processes
    int max_y;
    getmaxyx(performance_win, max_y, (int){0});
    int max_rows = max_y - 3;  // Leave space for border and header
    
    for (int i = 0; i < count && i < max_rows; i++) {
        int color = COLOR_PAIR(0);
        if (processes[i].state == RUNNING) color = COLOR_PAIR(1);
        else if (processes[i].state == READY) color = COLOR_PAIR(2);
        else if (processes[i].state == TERMINATED) color = COLOR_PAIR(3);
        
        wattron(performance_win, color);
        mvwprintw(performance_win, 2 + i, 2, 
                  "%-6s %5d %6d %4d %5d %5d",
                  processes[i].name,
                  processes[i].start_time,
                  processes[i].finish_time,
                  processes[i].waiting_time,
                  processes[i].turnaround_time,
                  processes[i].response_time);
        wattroff(performance_win, color);
    }
    
    wrefresh(performance_win);
}
// Draw overall metrics (bottom-right panel)
void draw_overall_metrics(float avg_wait, float avg_turnaround, float cpu_util) {
    werase(metrics_win);
    box(metrics_win, 0, 0);
    
    mvwprintw(metrics_win, 0, 2, "[ OVERALL METRICS ]");
    
    wattron(metrics_win, A_BOLD | COLOR_PAIR(4));
    mvwprintw(metrics_win, 2, 2, "Average Waiting Time:");
    wattroff(metrics_win, A_BOLD | COLOR_PAIR(4));
    mvwprintw(metrics_win, 2, 28, "%.2f units", avg_wait);
    
    wattron(metrics_win, A_BOLD | COLOR_PAIR(4));
    mvwprintw(metrics_win, 3, 2, "Average Turnaround Time:");
    wattroff(metrics_win, A_BOLD | COLOR_PAIR(4));
    mvwprintw(metrics_win, 3, 28, "%.2f units", avg_turnaround);
    
    wattron(metrics_win, A_BOLD | COLOR_PAIR(4));
    mvwprintw(metrics_win, 4, 2, "CPU Utilization:");
    wattroff(metrics_win, A_BOLD | COLOR_PAIR(4));
    mvwprintw(metrics_win, 4, 28, "%.2f %%", cpu_util);
    
    wrefresh(metrics_win);
}
// Draw control panel (bottom)
void draw_controls() {
    werase(control_win);
    box(control_win, 0, 0);
    
    int max_x;
    getmaxyx(control_win, (int){0}, max_x);
    
    char controls[256];
    snprintf(controls, sizeof(controls), 
                 "[SPACE] %s | [<>] Scroll | [+/-] Speed: %dms | [R] Restart | [Q] Quit",
             tui_state.paused ? "Resume" : "Pause",
             tui_state.speed_ms);
    
    int ctrl_x = (max_x - strlen(controls)) / 2;
    
    wattron(control_win, COLOR_PAIR(5));
    mvwprintw(control_win, 1, ctrl_x > 0 ? ctrl_x : 2, "%s", controls);
    wattroff(control_win, COLOR_PAIR(5));
    
    wrefresh(control_win);
}

// Handle keyboard input
void handle_input() {
    int ch = getch();
    
    switch (ch) {
        case 'q':
        case 'Q':
            tui_state.should_quit = true;
            break;
        case 'r':
        case 'R':
            tui_state.should_restart = true;
            break;
        case ' ':
            tui_state.paused = !tui_state.paused;
            break;
        case KEY_RIGHT:
            tui_state.gantt_offset += 5;
            break;
        case KEY_LEFT:
            if (tui_state.gantt_offset >= 5) tui_state.gantt_offset -= 5;
            else tui_state.gantt_offset = 0;
            break;
        case KEY_RESIZE:
             // Cleaner resize handling
             cleanup_tui();
             init_tui();
             break;
        case '+':
        case '=': // Handle +/mapped key
            if (tui_state.speed_ms > 100) tui_state.speed_ms -= 100;
            break;
        case '-':
        case '_': // Handle -/mapped key
            if (tui_state.speed_ms < 3500) tui_state.speed_ms += 100;
            break;
        case '>': // Keep old bindings for compatibility/scrolling
            tui_state.gantt_offset += 1;
            break;
        case '<':
             if (tui_state.gantt_offset > 0) tui_state.gantt_offset -= 1;
             break;
    }
}

int main(int argc, char* argv[]) {
    // Parse arguments (reuse CLI parser)
    CLIParams cli_params;
    if (parse_arguments(argc, argv, &cli_params) != 0) {
        return EXIT_FAILURE;
    }
    
    // Parse config file
    int process_count = 0;
    Process* processes = parse_config_file(cli_params.config_filepath, &process_count);
    
    if (!processes) {
        fprintf(stderr, "Failed to parse config file.\n");
        return EXIT_FAILURE;
    }
    
    // Get policy selection
    char* selected_policy = NULL;
    if (!get_policy_input(&selected_policy)) {
        free(processes);
        return EXIT_FAILURE;
    }
    
    // Get quantum if RR or MLFQ
    int quantum = 0;
    if (strcmp(selected_policy, "rr") == 0 || strcmp(selected_policy, "mlfq") == 0) {
        printf("Enter time quantum (base): ");
        scanf("%d", &quantum);
    }
    
    // Initialize TUI
    init_tui();
    
    // Main simulation loop (allows restart)
    do {
        // Reset all state flags for fresh start
        tui_state.should_restart = false;
        tui_state.should_quit = false;
        tui_state.should_quit = false;
        tui_state.paused = false;  // Ensure not paused on restart
        tui_state.gantt_offset = 0;
        
        // Reload processes from config file for fresh simulation
        int fresh_process_count = 0;
        Process* fresh_processes = parse_config_file(cli_params.config_filepath, &fresh_process_count);
        
        if (!fresh_processes) {
            cleanup_tui();
            fprintf(stderr, "Failed to reload config file.\\n");
            return EXIT_FAILURE;
        }
        
        // Live update callback for TUI
        void tui_tick_callback(int time, Process* procs, int count, Process* running, 
                               GanttEvent* events, int event_count) {
            // Handle user input
            handle_input();
            
            // Exit if requested
            if (tui_state.should_quit || tui_state.should_restart) {
                return;
            }
            
            // Wait if paused
            while (tui_state.paused && !tui_state.should_quit && !tui_state.should_restart) {
                handle_input();
                draw_controls();
                usleep(50000);
            }
            
            // Calculate current metrics (rough estimate during simulation)
            float avg_wait = 0, avg_turnaround = 0, cpu_util = 0;
            int completed = 0;
            
            for (int i = 0; i < count; i++) {
                if (procs[i].state == TERMINATED) {
                    avg_wait += procs[i].waiting_time;
                    avg_turnaround += procs[i].turnaround_time;
                    completed++;
                }
            }
            
            if (completed > 0) {
                avg_wait /= completed;
                avg_turnaround /= completed;
            }
            
            if (time > 0) {
                cpu_util = (running != NULL) ? 100.0 : 0.0;
            }
            
            // Update all displays
            draw_title(selected_policy, time);
            draw_process_info(procs, count);
            draw_status(running, procs, count);
            draw_gantt_chart(events, event_count, procs, count, time, false);
            draw_performance_metrics(procs, count);
            draw_overall_metrics(avg_wait, avg_turnaround, cpu_util);
            draw_controls();
            
            // Delay based on speed setting, but check input frequently for responsiveness
            int sleep_chunks = tui_state.speed_ms / 50;  // Break into 50ms chunks
            for (int i = 0; i < sleep_chunks && !tui_state.should_quit && !tui_state.should_restart; i++) {
                usleep(50000); 
                handle_input();
            }
        }
        
        // Configure simulation parameters
        SimParameters sim_params;
        sim_params.config_filepath = cli_params.config_filepath;
        sim_params.policy_name = selected_policy;
        sim_params.quantum = quantum;
        sim_params.verbose = false;  // Don't print logs to stdout
        sim_params.tick_callback = tui_tick_callback;
        
        // Run simulation with live updates
        SimulationResult* results = run_simulation(&sim_params);
        
        // Show final results
        if (results && !tui_state.should_quit && !tui_state.should_restart) {
            int final_time = 0;
            for (int i = 0; i < results->process_count; i++) {
                if (results->processes[i].finish_time > final_time) {
                    final_time = results->processes[i].finish_time;
                }
            }
            
            draw_title(selected_policy, final_time);
            draw_process_info(results->processes, results->process_count);
            draw_status(NULL, results->processes, results->process_count);
            draw_gantt_chart(results->gantt_chart, results->gantt_event_count,
                             results->processes, results->process_count, final_time, true);
            draw_performance_metrics(results->processes, results->process_count);
            draw_overall_metrics(results->average_waiting_time,
                                 results->average_turnaround_time,
                                 results->cpu_utilization);
            draw_controls();
            
            // Show "Simulation Complete" message
            attron(A_BOLD | COLOR_PAIR(1));
            mvprintw(LINES - 4, (COLS - 50) / 2, "Simulation Complete! Press R to restart or Q to exit");
            attroff(A_BOLD | COLOR_PAIR(1));
            refresh();
            
            // Wait for user to quit or restart
            nodelay(stdscr, FALSE);
            int ch;
            while ((ch = getch()) != EOF) {
                if (ch == 'q' || ch == 'Q') {
                    tui_state.should_quit = true;
                    break;
                } else if (ch == 'r' || ch == 'R') {
                    tui_state.should_restart = true;
                    break;
                } else if (ch == KEY_RIGHT) {
                    tui_state.gantt_offset += 5;
                    draw_gantt_chart(results->gantt_chart, results->gantt_event_count,
                             results->processes, results->process_count, final_time, true);
                } else if (ch == KEY_LEFT) {
                    if (tui_state.gantt_offset >= 5) tui_state.gantt_offset -= 5;
                    else tui_state.gantt_offset = 0;
                     draw_gantt_chart(results->gantt_chart, results->gantt_event_count,
                             results->processes, results->process_count, final_time, true);
                } else if (ch == KEY_RESIZE) {
                     cleanup_tui();
                     init_tui();
                     draw_title(selected_policy, final_time);
                     draw_process_info(results->processes, results->process_count);
                     draw_status(NULL, results->processes, results->process_count);
                     draw_gantt_chart(results->gantt_chart, results->gantt_event_count,
                                      results->processes, results->process_count, final_time, true);
                     draw_performance_metrics(results->processes, results->process_count);
                     draw_overall_metrics(results->average_waiting_time,
                                          results->average_turnaround_time,
                                          results->cpu_utilization);
                     draw_controls();
                      // Show "Simulation Complete" message
                    attron(A_BOLD | COLOR_PAIR(1));
                    mvprintw(LINES - 4, (COLS - 50) / 2, "Simulation Complete! Press R to restart or Q to exit");
                    attroff(A_BOLD | COLOR_PAIR(1));
                    refresh();
                }
            }
            nodelay(stdscr, TRUE);
        }
        
        // Cleanup this simulation run
        if (results) {
            free_simulation_results(results);
        }
        free(fresh_processes);
        
    } while (tui_state.should_restart && !tui_state.should_quit);
    
    // Cleanup
    cleanup_tui();
    free(processes);
    free(selected_policy);
    
    printf("\nTUI Scheduler terminated.\n");
    return EXIT_SUCCESS;
}
