#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../../headers/cli/cli.h"
#include "../../headers/engine/scheduler_engine.h"
#include "../../headers/parser/config_parser.h"
#include "../../headers/policies/policies.h"

/** @brief Main application window. */
GtkWidget *window;
/** @brief Label for displaying the current policy. */
GtkWidget *title_label;
/** @brief Label for displaying the current simulation time. */
GtkWidget *time_label;
/** @brief TreeView to display individual process information. */
GtkWidget *process_tree_view;
/** @brief Paned widget to hold status cards (running process, ready queue). */
GtkWidget *status_view;
/** @brief Drawing area for rendering the Gantt chart. */
GtkWidget *gantt_drawing_area;
/** @brief TreeView to display process performance metrics. */
GtkWidget *performance_tree_view;
/** @brief Box widget to hold overall simulation metrics. */
GtkWidget *metrics_view;
/** @brief Combo box for selecting the scheduling policy. */
GtkWidget *policy_combo;
/** @brief Entry field for specifying the time quantum. */
GtkWidget *quantum_entry;
/** @brief Button to start the simulation. */
GtkWidget *start_button;
/** @brief Button to pause/resume the simulation. */
GtkWidget *pause_button;
/** @brief Button to restart the simulation. */
GtkWidget *restart_button;
/** @brief Scale widget for adjusting simulation speed. */
GtkWidget *speed_scale;
/** @brief Label for displaying the current simulation speed. */
GtkWidget *control_label;

/**
 * @brief Represents the current state of the GUI simulation.
 * This structure holds all dynamic data related to the simulation's progress
 * and visual representation.
 */
typedef struct {
    Process *current_processes; /**< Array of current processes being simulated. */
    int process_count;          /**< Number of processes in the simulation. */
    GanttEvent *gantt_events;   /**< Array of events for the Gantt chart. */
    int gantt_event_count;      /**< Number of events in the Gantt chart. */
    int current_time;           /**< Current simulation time. */
    Process *running_process;   /**< Pointer to the process currently running on the CPU. */
    bool is_running;            /**< Flag indicating if the simulation is actively running. */
    bool is_paused;             /**< Flag indicating if the simulation is paused. */
    int speed_ms;               /**< Delay in milliseconds between each simulation tick. */
    char *selected_policy;      /**< Name of the currently selected scheduling policy. */
    int quantum;                /**< Time quantum used by quantum-based policies. */
    SimulationResult *results;  /**< Stores the final results of the simulation. */
} SimState;

/** @brief Global simulation state instance. */
SimState sim_state = {0};
/** @brief Path to the configuration file. */
char *config_filepath = NULL;
/** @brief Array of processes parsed from the config file at the start. */
Process *initial_processes = NULL;
/** @brief Number of initial processes. */
int initial_process_count = 0;

/**
 * @brief Structure to represent an RGB color.
 */
typedef struct { double r, g, b; } Color;

/**
 * @brief Predefined array of colors for processes in the Gantt chart.
 */
Color process_colors[] = {
    {0.2, 0.6, 0.9}, {0.9, 0.4, 0.4}, {0.4, 0.8, 0.4}, {0.9, 0.7, 0.3},
    {0.7, 0.4, 0.9}, {0.3, 0.9, 0.9}, {0.9, 0.5, 0.7}, {0.6, 0.6, 0.3},
};

/**
 * @brief Retrieves a color for a process based on its index.
 * @param index The index of the process.
 * @return A Color struct.
 */
Color get_process_color(int index) {
    return process_colors[index % 8];
}

/**
 * @brief Sets up the GtkTreeView for displaying process information.
 * Configures columns for process name, arrival time, burst time, priority,
 * remaining burst time, and executed time.
 */
void setup_process_tree_view() {
    GtkListStore *store = gtk_list_store_new(6,
        G_TYPE_STRING,
        G_TYPE_INT,
        G_TYPE_INT,
        G_TYPE_INT,
        G_TYPE_INT,
        G_TYPE_INT);
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(process_tree_view), GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    const char *titles[] = {"Name", "Arrival", "Burst", "Priority", "Remaining", "Executed"};
    
    for (int i = 0; i < 6; i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", i == 0 ? 0.0 : 0.5, NULL);
        
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
            titles[i], renderer, "text", i, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_column_set_sort_column_id(column, i);
        gtk_tree_view_append_column(GTK_TREE_VIEW(process_tree_view), column);
    }
}

/**
 * @brief Sets up the GtkTreeView for displaying process performance metrics.
 * Configures columns for process name, start time, finish time, waiting time,
 * turnaround time, and response time.
 */
void setup_performance_tree_view() {
    GtkListStore *store = gtk_list_store_new(6,
        G_TYPE_STRING,
        G_TYPE_INT,
        G_TYPE_INT,
        G_TYPE_INT,
        G_TYPE_INT,
        G_TYPE_INT);
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(performance_tree_view), GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    const char *titles[] = {"Name", "Start", "Finish", "Waiting Time", "Turnaround Time", "Response Time"};
    
    for (int i = 0; i < 6; i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", i == 0 ? 0.0 : 0.5, NULL);
        
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
            titles[i], renderer, "text", i, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_column_set_sort_column_id(column, i);
        gtk_tree_view_append_column(GTK_TREE_VIEW(performance_tree_view), column);
    }
}

/**
 * @brief Updates the process information in the GtkTreeView.
 * Iterates through all current processes and populates the tree view with
 * their name, arrival time, burst time, priority, remaining burst time,
 * and executed time.
 */
void update_process_info() {
    GtkListStore *store = GTK_LIST_STORE(
        gtk_tree_view_get_model(GTK_TREE_VIEW(process_tree_view)));
    gtk_list_store_clear(store);
    
    for (int i = 0; i < sim_state.process_count; i++) {
        Process *p = &sim_state.current_processes[i];
        int executed = p->burst_time - p->remaining_burst_time;
        
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, p->name,
            1, p->arrival_time,
            2, p->burst_time,
            3, p->priority,
            4, p->remaining_burst_time,
            5, executed,
            -1);
    }
}

/**
 * @brief Updates the process performance metrics in the GtkTreeView.
 * Populates the tree view with each process's start time, finish time,
 * waiting time, turnaround time, and response time.
 */
void update_performance() {
    GtkListStore *store = GTK_LIST_STORE(
        gtk_tree_view_get_model(GTK_TREE_VIEW(performance_tree_view)));
    gtk_list_store_clear(store);
    
    for (int i = 0; i < sim_state.process_count; i++) {
        Process *p = &sim_state.current_processes[i];
        
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, p->name,
            1, p->start_time,
            2, p->finish_time,
            3, p->waiting_time,
            4, p->turnaround_time,
            5, p->response_time,
            -1);
    }
}

GtkWidget *running_card_box;
GtkWidget *running_name_label;
GtkWidget *running_progress_bar;
GtkWidget *running_time_label;
GtkWidget *queue_card_box;
GtkWidget *queue_header_label;

/**
 * @brief Updates the status display, showing the running process and the ready queue.
 * This function dynamically updates the UI to reflect the current state of processes:
 * which one is running, its progress, and which processes are waiting in the ready queue.
 */
void update_status() {
    // Clearing existing queue items
    GList *children = gtk_container_get_children(GTK_CONTAINER(queue_card_box));
    int count = 0;
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        if (count >= 2) {
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        }
        count++;
    }
    g_list_free(children);
    
    // Updating running process card
    if (sim_state.running_process) {
        char name_markup[128];
        snprintf(name_markup, sizeof(name_markup),
                 "<span size='13000' weight='bold' foreground='#27ae60'>%s</span>",
                 sim_state.running_process->name);
        gtk_label_set_markup(GTK_LABEL(running_name_label), name_markup);
        
        // Updating progress bar
        double fraction = sim_state.running_process->burst_time > 0 ?
                         (double)(sim_state.running_process->burst_time - sim_state.running_process->remaining_burst_time) /
                         sim_state.running_process->burst_time : 0.0;
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(running_progress_bar), fraction);
        
        char prog_text[32];
        snprintf(prog_text, sizeof(prog_text), "%d/%d • %d%%",
                 sim_state.running_process->burst_time - sim_state.running_process->remaining_burst_time,
                 sim_state.running_process->burst_time,
                 (int)(fraction * 100));
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(running_progress_bar), prog_text);
        
        gtk_widget_show(running_card_box);
    } else {
        gtk_widget_hide(running_card_box);
    }
    
    // Counting ready processes
    int ready_count = 0;
    for (int i = 0; i < sim_state.process_count; i++) {
        if (sim_state.current_processes[i].state == READY) {
            ready_count++;
        }
    }
    
    // Updating queue header
    char queue_hdr[64];
    snprintf(queue_hdr, sizeof(queue_hdr),
             "<span foreground='#3498db' weight='bold'>⏳ Ready Queue (%d waiting)</span>",
             ready_count);
    gtk_label_set_markup(GTK_LABEL(queue_header_label), queue_hdr);
    
    // Adding mini cards for ready processes
    if (ready_count > 0) {
        for (int i = 0; i < sim_state.process_count; i++) {
            if (sim_state.current_processes[i].state == READY) {
                GtkWidget *mini_frame = gtk_frame_new(NULL);
                GtkWidget *mini_label = gtk_label_new(NULL);
                
                char mini_text[64];
                snprintf(mini_text, sizeof(mini_text),
                         "<span foreground='#3498db'>%s  •  Priority %d</span>",
                         sim_state.current_processes[i].name,
                         sim_state.current_processes[i].priority);
                gtk_label_set_markup(GTK_LABEL(mini_label), mini_text);
                gtk_container_set_border_width(GTK_CONTAINER(mini_label), 8);
                gtk_container_add(GTK_CONTAINER(mini_frame), mini_label);
                gtk_box_pack_start(GTK_BOX(queue_card_box), mini_frame, FALSE, FALSE, 3);
                gtk_widget_show_all(mini_frame);
            }
        }
    } else {
        // Displaying message for empty queue
        GtkWidget *empty_label = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(empty_label),
                            "<span foreground='#95a5a6' style='italic'>No processes waiting</span>");
        gtk_container_set_border_width(GTK_CONTAINER(empty_label), 10);
        gtk_box_pack_start(GTK_BOX(queue_card_box), empty_label, FALSE, FALSE, 0);
        gtk_widget_show(empty_label);
    }
}

GtkWidget *wait_value_label;
GtkWidget *tat_value_label;
GtkWidget *cpu_value_label;
GtkWidget *status_label_widget;

/**
 * @brief Updates the overall performance metrics displayed in the GUI.
 * Calculates and presents average waiting time, average turnaround time,
 * and CPU utilization based on the current simulation state.
 */
void update_overall_metrics() {
    float avg_wait = 0, avg_tat = 0, cpu_util = 0;
    int completed = 0;
    
    // Calculating total wait and turnaround times for completed processes
    for (int i = 0; i < sim_state.process_count; i++) {
        if (sim_state.current_processes[i].state == TERMINATED) {
            avg_wait += sim_state.current_processes[i].waiting_time;
            avg_tat += sim_state.current_processes[i].turnaround_time;
            completed++;
        }
    }
    
    // Calculating averages if processes completed
    if (completed > 0) {
        avg_wait /= completed;
        avg_tat /= completed;
    }
    
    // Calculating CPU utilization
    if (sim_state.current_time > 0) {
        cpu_util = (sim_state.running_process != NULL) ? 100.0 : 0.0;
    }
    
    // Updating wait time display
    char wait_text[128];
    snprintf(wait_text, sizeof(wait_text), "<span size='large' weight='bold' foreground='#2980b9'>%.2f</span>", avg_wait);
    gtk_label_set_markup(GTK_LABEL(wait_value_label), wait_text);
    
    // Updating TAT display
    char tat_text[128];
    snprintf(tat_text, sizeof(tat_text), "<span size='large' weight='bold' foreground='#2980b9'>%.2f</span>", avg_tat);
    gtk_label_set_markup(GTK_LABEL(tat_value_label), tat_text);
    
    // Updating CPU utilization display
    char cpu_text[128];
    snprintf(cpu_text, sizeof(cpu_text), "<span size='large' weight='bold' foreground='#2980b9'>%.2f%%</span>", cpu_util);
    gtk_label_set_markup(GTK_LABEL(cpu_value_label), cpu_text);
    
    // Updating simulation status display
    if (completed > 0) {
        char status_text[64];
        snprintf(status_text, sizeof(status_text), 
                 "<span foreground='#27ae60'>✓ %d/%d processes completed</span>",
                 completed, sim_state.process_count);
        gtk_label_set_markup(GTK_LABEL(status_label_widget), status_text);
    } else {
        gtk_label_set_markup(GTK_LABEL(status_label_widget), "<span foreground='#95a5a6'>Simulation running...</span>");
    }
}



/**
 * @brief Draws the Gantt chart representing process execution over time.
 * This function is a GTK+ drawing callback that renders the timeline of processes
 * and their CPU usage, including idle periods.
 * @param widget The GtkWidget that triggered the draw event.
 * @param cr The Cairo context for drawing.
 * @param data User data (not used in this case).
 * @return Always FALSE to indicate that the event has not been fully handled.
 */
gboolean draw_gantt_chart(GtkWidget *widget, cairo_t *cr, gpointer data) {
    // Handling initial state or no events
    if (!sim_state.gantt_events || sim_state.gantt_event_count == 0) {
        cairo_set_source_rgb(cr, 0.95, 0.95, 0.95);
        cairo_paint(cr);
        
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 14);
        cairo_move_to(cr, 50, 50);
        cairo_show_text(cr, "Click Start to begin simulation...");
        return FALSE;
    }
    
    int width = gtk_widget_get_allocated_width(widget);
    
    // Clearing background
    cairo_set_source_rgb(cr, 0.98, 0.98, 0.98);
    cairo_paint(cr);
    
    int max_time = sim_state.current_time + 1;
    if (max_time == 0) return FALSE;
    
    // Defining dimensions for drawing
    int left_margin = 80;
    int top_margin = 40;
    int time_width = 35;
    int row_height = 30;
    
    // Drawing chart title
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    cairo_move_to(cr, 10, 20);
    cairo_show_text(cr, "Gantt Chart - Process Execution Timeline");
    
    // Drawing time header
    cairo_set_font_size(cr, 10);
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    for (int t = 0; t <= max_time; t++) {
        int x = left_margin + t * time_width;
        cairo_move_to(cr, x + 12, top_margin - 8);
        char time_str[10];
        snprintf(time_str, sizeof(time_str), "%d", t);
        cairo_show_text(cr, time_str);
    }
    
    // Drawing grid lines
    cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 0.3);
    cairo_set_line_width(cr, 1);
    for (int t = 0; t <= max_time; t++) {
        int x = left_margin + t * time_width;
        cairo_move_to(cr, x, top_margin);
        cairo_line_to(cr, x, top_margin + sim_state.process_count * row_height);
        cairo_stroke(cr);
    }
    
    // Drawing each process row
    // Creating a temporary array of pointers to sort by original_index
    Process** sorted_procs = malloc(sizeof(Process*) * sim_state.process_count);
    for (int i=0; i<sim_state.process_count; i++) sorted_procs[i] = &sim_state.current_processes[i];
    
    // Performing simple bubble sort by original_index
    for (int i=0; i<sim_state.process_count-1; i++) {
        for (int j=0; j<sim_state.process_count-i-1; j++) {
            if (sorted_procs[j]->original_index > sorted_procs[j+1]->original_index) {
                Process* temp = sorted_procs[j];
                sorted_procs[j] = sorted_procs[j+1];
                sorted_procs[j+1] = temp;
            }
        }
    }

    for (int p = 0; p < sim_state.process_count && p < 12; p++) {
        int y = top_margin + p * row_height;
        Process* current_p = sorted_procs[p];
        
        // Drawing process name with background
        Color proc_color = get_process_color(current_p->original_index); // Using index for consistent color
        cairo_rectangle(cr, 5, y + 4, 70, row_height - 8);
        cairo_set_source_rgba(cr, proc_color.r, proc_color.g, proc_color.b, 0.2);
        cairo_fill(cr);
        
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 11);
        cairo_move_to(cr, 10, y + 18);
        cairo_show_text(cr, current_p->name);
        
        // Drawing execution timeline
        for (int t = 0; t <= max_time; t++) {
            bool was_running = false;
            
            for (int e = 0; e < sim_state.gantt_event_count; e++) {
                int event_time = sim_state.gantt_events[e].time;
                int next_time = (e + 1 < sim_state.gantt_event_count) ?
                               sim_state.gantt_events[e + 1].time : 
                               (sim_state.is_running ? sim_state.current_time + 1 : sim_state.current_time);
                
                if (strcmp(sim_state.gantt_events[e].process_name,
                          current_p->name) == 0 &&
                    t >= event_time && t < next_time) {
                    was_running = true;
                    break;
                }
            }
            
            int x = left_margin + t * time_width;
            
            if (was_running) {
                // Drawing filled block with gradient effect
                cairo_rectangle(cr, x + 3, y + 5, time_width - 6, row_height - 10);
                cairo_set_source_rgb(cr, proc_color.r, proc_color.g, proc_color.b);
                cairo_fill_preserve(cr);
                cairo_set_source_rgb(cr, proc_color.r * 0.7, proc_color.g * 0.7, proc_color.b * 0.7);
                cairo_set_line_width(cr, 1.5);
                cairo_stroke(cr);
            }
        }
    }
    free(sorted_procs);
    
    return FALSE;
}


/**
 * @brief Orchestrates the update of all relevant GUI displays.
 * This function calls individual update functions for process information,
 * status, performance metrics, and queues, then triggers a redraw of the Gantt chart.
 */
void update_displays() {
    char title[128];
    snprintf(title, sizeof(title), "Policy: %s", sim_state.selected_policy);
    gtk_label_set_text(GTK_LABEL(title_label), title);
    
    char time_text[64];
    snprintf(time_text, sizeof(time_text), "Time: %d", sim_state.current_time);
    gtk_label_set_text(GTK_LABEL(time_label), time_text);
    
    update_process_info();
    update_status();
    update_performance();
    update_overall_metrics();
    update_overall_metrics();
    
    // Dynamically resizing for Gantt chart scrolling
    int required_width = 80 + (sim_state.current_time + 5) * 35;
    if (required_width < 1200) required_width = 1200;
    gtk_widget_set_size_request(gantt_drawing_area, required_width, 400);
    
    gtk_widget_queue_draw(gantt_drawing_area);
}


/**
 * @brief Callback function executed at each simulation tick for GUI updates.
 * This function is registered with the scheduler engine to receive real-time
 * updates about the simulation progress and update the GUI accordingly.
 * It also handles GUI event processing and simulation speed control.
 *
 * @param time Current simulation time.
 * @param procs Array of all processes.
 * @param count Number of processes.
 * @param running Pointer to the currently running process.
 * @param events Array of Gantt chart events.
 * @param event_count Number of Gantt chart events.
 */
void gui_tick_callback(int time, Process* procs, int count, Process* running,
                       GanttEvent* events, int event_count) {
    sim_state.current_time = time;
    sim_state.current_processes = procs;
    sim_state.process_count = count;
    sim_state.running_process = running;
    sim_state.gantt_events = events;
    sim_state.gantt_event_count = event_count;
    
    update_displays();
    
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
    
    g_usleep(sim_state.speed_ms * 1000);
    
    // Handling pause state
    while (sim_state.is_paused && sim_state.is_running) {
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
        g_usleep(50000);
    }
}


/**
 * @brief Initiates a new simulation run.
 * Resets the simulation state, configures parameters, and calls the scheduler
 * engine to start the simulation, receiving real-time updates via `gui_tick_callback`.
 */
void start_simulation() {
    if (sim_state.is_running) return;
    
    // Freeing previous results if any
    if (sim_state.results) {
        free_simulation_results(sim_state.results);
        sim_state.results = NULL;
    }
    
    // Initializing simulation state
    sim_state.is_running = true;
    sim_state.is_paused = false;
    gtk_widget_set_sensitive(start_button, FALSE);
    gtk_widget_set_sensitive(pause_button, TRUE);
    gtk_widget_set_sensitive(restart_button, TRUE);
    
    // Setting up simulation parameters
    SimParameters sim_params;
    sim_params.config_filepath = config_filepath;
    sim_params.policy_name = sim_state.selected_policy;
    sim_params.quantum = sim_state.quantum;
    sim_params.verbose = FALSE;
    sim_params.tick_callback = gui_tick_callback;
    
    // Running the simulation and storing results
    sim_state.results = run_simulation(&sim_params);
    
    // Processing and displaying final results
    if (sim_state.results) {
        sim_state.current_time = 0;
        for (int i = 0; i < sim_state.results->process_count; i++) {
            if (sim_state.results->processes[i].finish_time > sim_state.current_time) {
                sim_state.current_time = sim_state.results->processes[i].finish_time;
            }
        }
        sim_state.current_processes = sim_state.results->processes;
        sim_state.process_count = sim_state.results->process_count;
        sim_state.gantt_events = sim_state.results->gantt_chart;
        sim_state.gantt_event_count = sim_state.results->gantt_event_count;
        update_displays();  // Final update
    }
    
    // Resetting simulation running state
    sim_state.is_running = false;
    gtk_widget_set_sensitive(start_button, TRUE);
    gtk_widget_set_sensitive(pause_button, FALSE);
}


/**
 * @brief Callback function for the "Start" button click event.
 * Validates selected policy and quantum, then initiates the simulation.
 * @param button The GtkButton widget that triggered the event.
 * @param data User data (not used in this case).
 */
void on_start_clicked(GtkButton *button, gpointer data) {
    // Getting fresh policy selection
    gchar *policy = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(policy_combo));
    if (!policy) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Please select a scheduling policy");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Freeing old policy and updating with new selection
    if (sim_state.selected_policy) {
        g_free(sim_state.selected_policy);
    }
    sim_state.selected_policy = policy;  // Taking ownership of the string
    
    const char *quantum_text = gtk_entry_get_text(GTK_ENTRY(quantum_entry));
    sim_state.quantum = atoi(quantum_text);
    
    // Validating quantum for RR and MLFQ policies
    if ((strcmp(sim_state.selected_policy, "rr") == 0 || strcmp(sim_state.selected_policy, "RR") == 0 ||
         strcmp(sim_state.selected_policy, "mlfq") == 0 || strcmp(sim_state.selected_policy, "MLFQ") == 0) &&
        sim_state.quantum <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Please enter a valid time quantum for Round Robin / MLFQ");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    start_simulation();
}

/**
 * @brief Callback function for the "Pause/Resume" button click event.
 * Toggles the `is_paused` state of the simulation and updates the button label.
 * @param button The GtkButton widget that triggered the event.
 * @param data User data (not used in this case).
 */
void on_pause_clicked(GtkButton *button, gpointer data) {
    sim_state.is_paused = !sim_state.is_paused;
    gtk_button_set_label(button, sim_state.is_paused ? "▶ Resume" : "⏸ Pause");
}

/**
 * @brief Callback function for the "Restart" button click event.
 * Forces the current simulation to stop and then starts a new one with
 * the currently selected policy and quantum.
 * @param button The GtkButton widget that triggered the event.
 * @param data User data (not used in this case).
 */
void on_restart_clicked(GtkButton *button, gpointer data) {
    // Forcing current simulation to stop
    sim_state.is_running = false;
    g_usleep(100000);
    
    // Getting fresh policy selection
    gchar *policy = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(policy_combo));
    if (policy) {
        if (sim_state.selected_policy) {
            g_free(sim_state.selected_policy);
        }
        sim_state.selected_policy = policy;
        
        const char *quantum_text = gtk_entry_get_text(GTK_ENTRY(quantum_entry));
        sim_state.quantum = atoi(quantum_text);
    }
    
    start_simulation();
}

/**
 * @brief Callback function for when the simulation speed slider value changes.
 * Updates the simulation speed (`speed_ms`) and its corresponding display label.
 * @param range The GtkRange widget (speed_scale) that triggered the event.
 * @param data User data (not used in this case).
 */
void on_speed_changed(GtkRange *range, gpointer data) {
    sim_state.speed_ms = (int)gtk_range_get_value(range);
    char label[64];
    snprintf(label, sizeof(label), "%dms", sim_state.speed_ms);
    gtk_label_set_text(GTK_LABEL(control_label), label);
}


/**
 * @brief Callback function for when the scheduling policy selection changes.
 * Enables or disables the quantum entry field based on whether the selected
 * policy is quantum-based (e.g., Round Robin, MLFQ).
 * @param combo The GtkComboBox widget (policy_combo) that triggered the event.
 * @param data User data (not used in this case).
 */
void on_policy_changed(GtkComboBox *combo, gpointer data) {
    gchar *policy = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
    if (policy) {
        // Enabling quantum for Round Robin and MLFQ
        gboolean uses_quantum = (strcmp(policy, "rr") == 0 || strcmp(policy, "RR") == 0 ||
                                 strcmp(policy, "mlfq") == 0 || strcmp(policy, "MLFQ") == 0);
        gtk_widget_set_sensitive(quantum_entry, uses_quantum);
        g_free(policy);
    }
}


void create_window() {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "CPU Scheduler Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 1300, 950);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 15);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);
    
    // Title (centered)
    GtkWidget *title_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(main_vbox), title_box, FALSE, FALSE, 0);
    
    GtkWidget *main_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(main_title),
        "<span size='22000' weight='bold' foreground='#2c3e50'>CPU Scheduler Simulator</span>");
    gtk_widget_set_halign(main_title, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(title_box), main_title, FALSE, FALSE, 0);
    
    GtkWidget *subtitle_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(subtitle_hbox, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(title_box), subtitle_hbox, FALSE, FALSE, 0);
    
    title_label = gtk_label_new("Select a policy and click Start");
    gtk_box_pack_start(GTK_BOX(subtitle_hbox), title_label, FALSE, FALSE, 0);
    
    time_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(time_label), "<span weight='bold' foreground='#e74c3c'>Time: 0</span>");
    gtk_box_pack_start(GTK_BOX(subtitle_hbox), time_label, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    
    // Top row: Process Info | Status
    GtkWidget *top_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_vbox), top_paned, FALSE, FALSE, 0);
    
    GtkWidget *proc_frame = gtk_frame_new(NULL);
    GtkWidget *proc_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(proc_label), "<b>Process Information</b>");
    gtk_frame_set_label_widget(GTK_FRAME(proc_frame), proc_label);
    gtk_widget_set_size_request(proc_frame, -1, 220);
    
    GtkWidget *proc_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(proc_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(proc_frame), proc_scroll);
    
    process_tree_view = gtk_tree_view_new();
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(process_tree_view), GTK_TREE_VIEW_GRID_LINES_BOTH);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(process_tree_view), TRUE);
    gtk_container_add(GTK_CONTAINER(proc_scroll), process_tree_view);
    setup_process_tree_view();
    gtk_paned_add1(GTK_PANED(top_paned), proc_frame);
    
    GtkWidget *status_frame = gtk_frame_new(NULL);
    GtkWidget *status_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(status_label), "<b>Execution Status</b>");
    gtk_frame_set_label_widget(GTK_FRAME(status_frame), status_label);
    
    GtkWidget *status_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(status_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(status_frame), status_scroll);
    
    // Horizontal paned for side-by-side layout
    status_view = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(status_scroll), status_view);
    
    // LEFT: Running process card
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(left_box), 5);
    
    GtkWidget *running_frame = gtk_frame_new(NULL);
    running_card_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(running_card_box), 8);
    gtk_container_add(GTK_CONTAINER(running_frame), running_card_box);
    
    GtkWidget *running_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(running_header), "<span foreground='#27ae60' weight='bold'>▶ Running Process</span>");
    gtk_widget_set_halign(running_header, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(running_card_box), running_header, FALSE, FALSE, 0);
    
    running_name_label = gtk_label_new("");
    gtk_widget_set_halign(running_name_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(running_card_box), running_name_label, FALSE, FALSE, 0);
    
    running_progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(running_progress_bar), TRUE);
    gtk_box_pack_start(GTK_BOX(running_card_box), running_progress_bar, FALSE, FALSE, 0);
    
    // Expand to fill vertical space
    gtk_box_pack_start(GTK_BOX(left_box), running_frame, TRUE, TRUE, 0);
    gtk_paned_add1(GTK_PANED(status_view), left_box);
    
    // RIGHT: Ready queue card
    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(right_box), 5);
    
    GtkWidget *queue_frame = gtk_frame_new(NULL);
    GtkWidget *queue_frame_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(queue_frame_label), "<b>Ready Queue</b>");
    gtk_frame_set_label_widget(GTK_FRAME(queue_frame), queue_frame_label);
    
    queue_card_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(queue_card_box), 12);
    gtk_container_add(GTK_CONTAINER(queue_frame), queue_card_box);
    
    queue_header_label = gtk_label_new("");
    gtk_widget_set_halign(queue_header_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(queue_card_box), queue_header_label, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(queue_card_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(right_box), queue_frame, TRUE, TRUE, 0);
    gtk_paned_add2(GTK_PANED(status_view), right_box);
    
    gtk_paned_set_position(GTK_PANED(status_view), 250);
    
    gtk_paned_add2(GTK_PANED(top_paned), status_frame);
    
    gtk_paned_set_position(GTK_PANED(top_paned), 650);
    
    // Gantt chart
    GtkWidget *gantt_frame = gtk_frame_new(NULL);
    GtkWidget *gantt_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(gantt_label), "<b>Gantt Chart</b>");
    gtk_frame_set_label_widget(GTK_FRAME(gantt_frame), gantt_label);
    gtk_widget_set_size_request(gantt_frame, -1, 300);
    gtk_box_pack_start(GTK_BOX(main_vbox), gantt_frame, TRUE, TRUE, 0);
    
    GtkWidget *gantt_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gantt_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(gantt_frame), gantt_scroll);
    
    gantt_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(gantt_drawing_area, 1200, 400);
    g_signal_connect(gantt_drawing_area, "draw", G_CALLBACK(draw_gantt_chart), NULL);
    gtk_container_add(GTK_CONTAINER(gantt_scroll), gantt_drawing_area);
    
    // Bottom row: Performance | Metrics
    GtkWidget *bottom_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_vbox), bottom_paned, FALSE, FALSE, 0);
    
    GtkWidget *perf_frame = gtk_frame_new(NULL);
    GtkWidget *perf_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(perf_label), "<b>Process Performance</b>");
    gtk_frame_set_label_widget(GTK_FRAME(perf_frame), perf_label);
    gtk_widget_set_size_request(perf_frame, -1, 280);
    
    GtkWidget *perf_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(perf_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(perf_frame), perf_scroll);
    
    performance_tree_view = gtk_tree_view_new();
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(performance_tree_view), GTK_TREE_VIEW_GRID_LINES_BOTH);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(performance_tree_view), TRUE);
    gtk_container_add(GTK_CONTAINER(perf_scroll), performance_tree_view);
    setup_performance_tree_view();
    gtk_paned_add1(GTK_PANED(bottom_paned), perf_frame);
    
    GtkWidget *metrics_frame = gtk_frame_new(NULL);
    GtkWidget *metrics_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(metrics_label), "<b>Overall Metrics</b>");
    gtk_frame_set_label_widget(GTK_FRAME(metrics_frame), metrics_label);
    
    GtkWidget *metrics_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(metrics_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(metrics_frame), metrics_scroll);
    
    // Main metrics container
    metrics_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(metrics_view), 10);
    gtk_container_add(GTK_CONTAINER(metrics_scroll), metrics_view);
    
    // Title
    GtkWidget *metrics_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(metrics_title), "<span size='12000' weight='bold' foreground='#2c3e50'>📊 Performance Metrics</span>");
    gtk_widget_set_halign(metrics_title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(metrics_view), metrics_title, FALSE, FALSE, 0);
    
    // 2x2 Grid for metrics
    GtkWidget *metrics_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(metrics_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(metrics_grid), 10);
    gtk_grid_set_row_homogeneous(GTK_GRID(metrics_grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(metrics_grid), TRUE);
    gtk_box_pack_start(GTK_BOX(metrics_view), metrics_grid, FALSE, FALSE, 5);
    
    // Card 1: Average Wait Time (top-left)
    GtkWidget *wait_card = gtk_frame_new(NULL);
    GtkWidget *wait_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(wait_box), 10);
    gtk_container_add(GTK_CONTAINER(wait_card), wait_box);
    
    GtkWidget *wait_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(wait_header), "<span weight='bold' foreground='#34495e'>⏱ Average Waiting Time</span>");
    gtk_widget_set_halign(wait_header, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(wait_box), wait_header, FALSE, FALSE, 0);
    
    wait_value_label = gtk_label_new("0.00");
    gtk_widget_set_halign(wait_value_label, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(wait_box), wait_value_label, TRUE, TRUE, 0);
    
    GtkWidget *wait_unit = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(wait_unit), "<span foreground='#7f8c8d' size='8000'>time units</span>");
    gtk_widget_set_halign(wait_unit, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(wait_box), wait_unit, FALSE, FALSE, 0);
    
    gtk_grid_attach(GTK_GRID(metrics_grid), wait_card, 0, 0, 1, 1);
    
    // Card 2: Average TAT (top-right)
    GtkWidget *tat_card = gtk_frame_new(NULL);
    GtkWidget *tat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(tat_box), 10);
    gtk_container_add(GTK_CONTAINER(tat_card), tat_box);
    
    GtkWidget *tat_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(tat_header), "<span weight='bold' foreground='#34495e'>🔄 Average Turnaround Time</span>");
    gtk_widget_set_halign(tat_header, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(tat_box), tat_header, FALSE, FALSE, 0);
    
    tat_value_label = gtk_label_new("0.00");
    gtk_widget_set_halign(tat_value_label, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(tat_box), tat_value_label, TRUE, TRUE, 0);
    
    GtkWidget *tat_unit = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(tat_unit), "<span foreground='#7f8c8d' size='8000'>time units</span>");
    gtk_widget_set_halign(tat_unit, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(tat_box), tat_unit, FALSE, FALSE, 0);
    
    gtk_grid_attach(GTK_GRID(metrics_grid), tat_card, 1, 0, 1, 1);
    
    // Card 3: CPU Utilizationization (bottom-left)
    GtkWidget *cpu_card = gtk_frame_new(NULL);
    GtkWidget *cpu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(cpu_box), 10);
    gtk_container_add(GTK_CONTAINER(cpu_card), cpu_box);
    
    GtkWidget *cpu_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(cpu_header), "<span weight='bold' foreground='#34495e'>💻 CPU Utilization</span>");
    gtk_widget_set_halign(cpu_header, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(cpu_box), cpu_header, FALSE, FALSE, 0);
    
    cpu_value_label = gtk_label_new("0.00%");
    gtk_widget_set_halign(cpu_value_label, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(cpu_box), cpu_value_label, TRUE, TRUE, 0);
    
    gtk_grid_attach(GTK_GRID(metrics_grid), cpu_card, 0, 1, 1, 1);
    
    // Card 4: Status (bottom-right)
    GtkWidget *status_card = gtk_frame_new(NULL);
    GtkWidget *status_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(status_box), 10);
    gtk_container_add(GTK_CONTAINER(status_card), status_box);
    
    GtkWidget *status_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(status_header), "<span weight='bold' foreground='#34495e'>📈 Status</span>");
    gtk_widget_set_halign(status_header, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(status_box), status_header, FALSE, FALSE, 0);
    
    status_label_widget = gtk_label_new("");
    gtk_widget_set_halign(status_label_widget, GTK_ALIGN_CENTER);
    gtk_label_set_line_wrap(GTK_LABEL(status_label_widget), TRUE);
    gtk_box_pack_start(GTK_BOX(status_box), status_label_widget, TRUE, TRUE, 0);
    
    gtk_grid_attach(GTK_GRID(metrics_grid), status_card, 1, 1, 1, 1);
    
    gtk_paned_add2(GTK_PANED(bottom_paned), metrics_frame);
    
    gtk_paned_set_position(GTK_PANED(bottom_paned), 650);
    
    gtk_box_pack_start(GTK_BOX(main_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    
    // Control panel
    GtkWidget *control_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_halign(control_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_vbox), control_box, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(control_box), gtk_label_new("Policy:"), FALSE, FALSE, 0);
    policy_combo = gtk_combo_box_text_new();
    
    // Dynamically populate the combo box with registered policies
    int policy_count = 0;
    const char** policy_names = get_available_policies(&policy_count);
    for (int i = 0; i < policy_count; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(policy_combo), policy_names[i]);
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(policy_combo), 0);
    g_signal_connect(policy_combo, "changed", G_CALLBACK(on_policy_changed), NULL);
    gtk_box_pack_start(GTK_BOX(control_box), policy_combo, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(control_box), gtk_label_new("  Quantum:"), FALSE, FALSE, 0);
    quantum_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(quantum_entry), "2");
    gtk_entry_set_width_chars(GTK_ENTRY(quantum_entry), 5);
    gtk_widget_set_sensitive(quantum_entry, FALSE);  // Initially disabled (FIFO selected)
    gtk_box_pack_start(GTK_BOX(control_box), quantum_entry, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(control_box), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 10);
    
    start_button = gtk_button_new_with_label("▶ Start");
    gtk_widget_set_size_request(start_button, 100, 35);
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(control_box), start_button, FALSE, FALSE, 0);
    
    pause_button = gtk_button_new_with_label("⏸ Pause");
    gtk_widget_set_size_request(pause_button, 100, 35);
    g_signal_connect(pause_button, "clicked", G_CALLBACK(on_pause_clicked), NULL);
    gtk_widget_set_sensitive(pause_button, FALSE);
    gtk_box_pack_start(GTK_BOX(control_box), pause_button, FALSE, FALSE, 0);
    
    restart_button = gtk_button_new_with_label("↻ Restart");
    gtk_widget_set_size_request(restart_button, 100, 35);
    g_signal_connect(restart_button, "clicked", G_CALLBACK(on_restart_clicked), NULL);
    gtk_widget_set_sensitive(restart_button, FALSE);
    gtk_box_pack_start(GTK_BOX(control_box), restart_button, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(control_box), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 10);
    
    gtk_box_pack_start(GTK_BOX(control_box), gtk_label_new("Speed:"), FALSE, FALSE, 0);
    speed_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 100, 2000, 100);
    gtk_range_set_value(GTK_RANGE(speed_scale), 500);
    gtk_widget_set_size_request(speed_scale, 150, -1);
    gtk_scale_set_draw_value(GTK_SCALE(speed_scale), FALSE);
    g_signal_connect(speed_scale, "value-changed", G_CALLBACK(on_speed_changed), NULL);
    gtk_box_pack_start(GTK_BOX(control_box), speed_scale, FALSE, FALSE, 0);
    
    control_label = gtk_label_new("500ms");
    gtk_box_pack_start(GTK_BOX(control_box), control_label, FALSE, FALSE, 0);
    
    
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css,
        "button { font-size: 11pt; padding: 8px; }"
        "frame { border: 2px solid #bdc3c7; border-radius: 5px; background: #ecf0f1; }"
        "textview { background: white; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    CLIParams cli_params;
    if (parse_arguments(argc, argv, &cli_params) != 0) {
        return EXIT_FAILURE;
    }
    
    config_filepath = cli_params.config_filepath;
    
    initial_processes = parse_config_file(config_filepath, &initial_process_count);
    if (!initial_processes) {
        fprintf(stderr, "Failed to parse config file.\n");
        return EXIT_FAILURE;
    }
    
    sim_state.speed_ms = 900;
    sim_state.current_processes = initial_processes;
    sim_state.process_count = initial_process_count;
    
    gtk_init(&argc, &argv);

    // Register all available scheduling policies before creating the window
    register_all_policies();
    
    create_window();
    
    update_process_info();
    update_overall_metrics();
    
    gtk_main();
    
    if (initial_processes) free(initial_processes);
    if (sim_state.selected_policy) g_free(sim_state.selected_policy);
    if (sim_state.results) free_simulation_results(sim_state.results);
    
    return EXIT_SUCCESS;
}
