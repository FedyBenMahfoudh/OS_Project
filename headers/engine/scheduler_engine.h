// scheduler_engine.h
#ifndef SCHEDULER_ENGINE_H
#define SCHEDULER_ENGINE_H

// Chargement / initialisation
int sim_init(const char *config_path, const char *policy_name, const char *policy_options);

// Exécute la simulation jusqu'à la fin (bloquant).
int sim_run(void);

// Arrête proprement et libère ressources.
void sim_shutdown(void);

// Obtenir métriques après run
void sim_get_metrics(double *avg_waiting_time, double *avg_turnaround_time, int *throughput);

// Export output (optionnel si sim_run l'a déjà fait)
int sim_export_results(const char *outfile_path);


#endif
