#ifndef UTILS_H
#define UTILS_H

#include "../data_structures/process.h"

/**
 * @brief Prints a formatted table of processes to stdout.
 * @param processes Array of processes to display.
 * @param count Number of processes in the array.
 */
void print_process_table(const Process* processes, int count);

#endif
