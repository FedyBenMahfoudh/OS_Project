#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

typedef struct {
    char* config_filepath; 
    bool verbose;
} CLIParams;

int parse_arguments(int argc, char* argv[], CLIParams* params);

bool get_policy_input(char** policy_input);

#endif
 