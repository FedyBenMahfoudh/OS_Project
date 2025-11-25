#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "../data_structures/data_structures.h"

// Params:
// @filepath: Path to the configuration file
// @process_count: Pointer to save the number of parsed processes

// Return:
// Dynamically-allocated table of parsed processes OTHERWISE NULL 
Process* parse_config_file(const char* filepath, int* process_count);

#endif
