#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "../../headers/parser/config_parser.h"
#include "../../headers/data_structures/process.h"


#define INITIAL_CAPACITY 16

typedef enum {
    IDLE,
    IN_PROCESS,
    IN_COMMENT_BLOCK
} ParserState;

char* trim_whitespaces_from_string(char* str) {
    char* end;

    while (isspace((unsigned char) *str)) str++;

    if (*str == 0) return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end)) end--;
    *(end + 1) = 0;

    return str;
}

Process* parse_config_file(const char* filepath, int* process_count) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open config file '%s'.\n", filepath);
        return NULL;
    }

    char line[256];
    int line_number = 0;
    int capacity = INITIAL_CAPACITY;

    Process* processes = malloc(sizeof(Process) * capacity);
    if (!processes) {
        fprintf(stderr, "Error: Memory allocation failed for processes array.\n");
        return NULL;
    }

    *process_count = 0;
    Process* current_process = NULL;
    ParserState state = IDLE;

    while(fgets(line, sizeof(line), file)) {
        line_number++;
        char* trimmed_line = trim_whitespaces_from_string(line);        

        char* comment_position = strchr(trimmed_line, '#');
        if (comment_position != NULL) {
            *comment_position = '\0';
            trimmed_line = trim_whitespaces_from_string(trimmed_line);
        }

        if (strlen(trimmed_line) == 0) {
            continue;
        }

        if (strncmp(trimmed_line, "\"\"\"", 3) == 0) {
            if (state == IDLE) state = IN_COMMENT_BLOCK;
            else if (state == IN_COMMENT_BLOCK) state = IDLE;
            continue;
        }

        switch(state) {
            case IDLE: {
                char process_name[32];
                if (sscanf(trimmed_line, "process %31s {", process_name) == 1) {
                }

                if (*process_count >= capacity) {
                    capacity *= 2;
                    Process* new_processes = realloc(processes, sizeof(Process) * capacity);
                    if (!new_processes) {
                        fprintf(stderr, "Error line %d: Memory reallocation failed.\n", line_number);
                        free(processes);
                        fclose(file);
                        return NULL;
                    }

                    processes = new_processes;
                }
                
                current_process = &processes[*process_count];
                strncpy(current_process->name, process_name, sizeof(current_process->name) - 1);
                current_process->name[sizeof(current_process->name) - 1] = '\0';

                current_process->arrival_time = -1; //
                current_process->burst_time = -1; 
                current_process->priority = 0; 
                current_process->original_index = *process_count; 

                current_process->start_time = 0;
                current_process->finish_time = 0;
                current_process->waiting_time = 0;
                current_process->turnaround_time = 0;
                current_process->response_time = 0;
                current_process->remaining_burst_time = 0;
                current_process->executed_time = 0;
                current_process->last_executed_time = 0;
                current_process->is_preempted = false;
                current_process->current_quantum_runtime = 0;
                
                current_process->last_active_time = 0;
                current_process->current_queue_level = 0;
                current_process->time_spent_at_current_level = 0;

                state = IN_PROCESS;
            }
                break;

            case IN_PROCESS: {
                if (strcmp(trimmed_line, "}") == 0) {
                    if ((current_process->arrival_time < 0) || (current_process->burst_time <= 0)) {
                        fprintf(stderr, "Error parsing process %s: missing or invalid 'arrival_time' or 'burst_time'.\n", current_process->name);
                        free(processes);
                        fclose(file);
                        return NULL;
                    }

                    (*process_count)++;
                    current_process = NULL;
                    state = IDLE;
                    break;
                }

                char* key = trimmed_line;
                char* value_str = strchr(trimmed_line, '=');

                if (value_str == NULL) {
                    fprintf(stderr, "Error line %d: Invalid syntax in process block: '%s'. Expected 'key = value'.\n", line_number, trimmed_line);
                    free(processes);
                    fclose(file);
                    return NULL;
                }

                *value_str = '\0'; 
                value_str++; 

                key = trim_whitespaces_from_string(key);
                value_str = trim_whitespaces_from_string(value_str);
                int value = atoi(value_str);

                if (strcmp(key, "arrival_time") == 0) {
                    current_process->arrival_time = value;
                    if (value < 0) {
                        fprintf(stderr, "Error line %d: 'arrival_time' value cannot be negative for process '%s'.\n", line_number, current_process->name);
                        free(processes);
                        fclose(file);
                        return NULL;
                    }
                } else if (strcmp(key, "burst_time") == 0) {
                    if (value <= 0) {
                        fprintf(stderr, "Error line %d: 'burst_time' value must be positive for process '%s'.\n", line_number, current_process->name);
                        free(processes);
                        fclose(file);
                        return NULL;
                    }
                    current_process->burst_time = value;
                    current_process->remaining_burst_time = value;
                } else if (strcmp(key, "priority") == 0) {
                    if (value < 0) {
                        fprintf(stderr, "Error line %d: 'priority' value cannot be negative for process '%s'.\n", line_number, current_process->name);
                        free(processes);
                        fclose(file);
                        return NULL;
                    }
                    current_process->priority = value;
                } else {
                    fprintf(stderr, "Error line %d: Unknown key '%s' for process '%s'.\n", line_number, key, current_process->name);
                    free(processes);
                    fclose(file);
                    return NULL;                    
                }

            }
            break;
            case IN_COMMENT_BLOCK:
                continue;
        }
    }

    if (state == IN_PROCESS) {
        fprintf(stderr, "Error: Unexpected end of file while parsing process '%s'.\n", current_process->name);
        free(processes);
        fclose(file);
        return NULL;
    }

    if (state == IN_COMMENT_BLOCK) {
        fprintf(stderr, "Error: Unexpected end of file while in multi-line comment block. Missing '\"\"\"' \n");
        free(processes);
        fclose(file);
        return NULL;
    }

    return processes;
}