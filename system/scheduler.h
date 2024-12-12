#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>

// Livelli di log disponibili
typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} log_level_t;

// Codici di errore dello scheduler
typedef enum {
    SCHED_ERR_OK = 0,
    SCHED_ERR_FULL,
    SCHED_ERR_INVALID_INDEX,
    SCHED_ERR_INVALID_PARAMS,
    SCHED_ERR_UNKNOWN
} sched_error_t;

// Algo di scheduling
typedef enum {
    SCHED_ALGO_PRIORITY,
    SCHED_ALGO_ROUND_ROBIN,
    SCHED_ALGO_EARLIEST_DEADLINE_FIRST
} sched_algorithm_t;

// Tipo funzione per i task
typedef void (*task_func_t)(void);

typedef struct {
    const char     *name;
    task_func_t     task;
    int             priority;
    int             dynamic_priority;
    int64_t         interval;
    absolute_time_t last_execution;
    int64_t         total_time;           // tempo totale accumulato
    int64_t         max_exec_time;        // tempo massimo di esecuzione
    int64_t         total_exec_time;      // somma di tutti i tempi di esecuzione
    int             exec_count;           // numero di esecuzioni
    int             deadline_miss_count;
    bool            debug_enabled;
    // Jitter: differenza tra il tempo atteso (last_execution + interval) e l'avvio effettivo
    int64_t         max_jitter;
} task_t;

// Struttura per statistiche di un singolo task
typedef struct {
    const char *name;
    int         priority;
    int         dynamic_priority;
    int64_t     interval;
    int64_t     total_time;
    int64_t     max_exec_time;
    int64_t     avg_exec_time;
    int         exec_count;
    int         deadline_miss_count;
    int64_t     max_jitter;
} task_stats_t;

// Funzioni di configurazione e aggiunta task
sched_error_t scheduler_add_task(const char *name, task_func_t task, int priority, int64_t interval);
sched_error_t scheduler_set_task_debug(int task_index, bool enabled);
sched_error_t scheduler_set_task_priority(int task_index, int new_priority);
sched_error_t scheduler_set_task_interval(int task_index, int64_t new_interval);

// Funzioni per ottenere statistiche e impostare il logging
sched_error_t scheduler_get_task_stats(int task_index, task_stats_t *stats);
void scheduler_set_log_level(log_level_t level);
void scheduler_print_all_tasks_stats(void);
void scheduler_set_algorithm(sched_algorithm_t algorithm);

// Funzione principale dello scheduler
void scheduler_run(void);

#endif // SCHEDULER_H
