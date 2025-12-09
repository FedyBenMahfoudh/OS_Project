#ifndef GANTT_TEXT_H
#define GANTT_TEXT_H

#include "../engine/scheduler_engine.h"

/**
 * @brief Prints a grid-based ASCII Gantt chart to stdout.
 * 
 * Format:
 * Time:  0 1 2 3 ...
 *        |-|-|-| ...
 * P1     █ █ . . ...
 * P2     . . █ █ ...
 * 
 * @param results Pointer to the simulation results containing events and process info.
 */
void print_gantt_chart(const SimulationResult* results);

#endif
