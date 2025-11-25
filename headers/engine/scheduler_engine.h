// scheduler_engine.h
#ifndef SCHEDULER_ENGINE_H
#define SCHEDULER_ENGINE_H

// Loading / Initialization
int sim_init(const char *config_path, const char *policy_name, const char *policy_options);

// Executes the simulation till the finish (Can block).
int sim_run(void);

// Stops properly and frees up resources.
void sim_shutdown(void);

// Gives metrics after execution finish
void sim_get_metrics(double *avg_waiting_time, double *avg_turnaround_time, int *throughput);

// Exports the output (Optional)
int sim_export_results(const char *outfile_path);


#endif
