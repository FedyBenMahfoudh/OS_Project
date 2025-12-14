#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "../data_structures/data_structures.h"

Process* parse_config_file(const char* filepath, int* process_count);

#endif
