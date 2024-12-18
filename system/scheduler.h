#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/time.h"

// -----------------------------------------------------------------------------
// Macro e Costanti
// -----------------------------------------------------------------------------
#define RP2040_TOTAL_RAM        (264 * 1024) // 264 KB = 270336 bytes
#define STACK_FILL_VALUE        (0xAA)       // Valore per il pattern sullo stack
#define TASK_STACK_SIZE         1024
#define MAX_TASKS               10
#define PRIORITY_NORMALIZATION_INTERVAL 100 // Normalizza le priorità ogni 100 iterazioni

// -----------------------------------------------------------------------------
// Definizioni e Tipi
// -----------------------------------------------------------------------------

// Stati del task
typedef enum {
    TASK_RUNNING,
    TASK_PAUSED
} task_state_t;

// Errori dello scheduler
typedef enum {
    SCHED_ERR_OK = 0,
    SCHED_ERR_FULL,
    SCHED_ERR_INVALID_INDEX,
    SCHED_ERR_INVALID_PARAMS
} sched_error_t;

// Algoritmi di scheduling
typedef enum {
    SCHED_ALGO_PRIORITY,
    SCHED_ALGO_ROUND_ROBIN,
    SCHED_ALGO_EARLIEST_DEADLINE_FIRST,
    SCHED_ALGO_LEAST_EXECUTED,
    SCHED_ALGO_LONGEST_WAITING
} sched_algorithm_t;

// Tipo funzione per i task
typedef void (*task_func_t)(void);

// Definizione del task
typedef struct {
    const char *name;
    task_func_t task;
    int priority;
    int dynamic_priority;
    int state;
    int64_t interval;
    absolute_time_t last_execution;
    int exec_count;
    int64_t total_time;
    int64_t total_exec_time;
    int64_t max_exec_time;
    int64_t max_jitter;
    int64_t min_exec_time; 
    int64_t total_jitter;
    size_t memory_allocated;
} task_t;

// -----------------------------------------------------------------------------
// API dello Scheduler
// -----------------------------------------------------------------------------

sched_error_t scheduler_add_task(const char *name, task_func_t task, int priority, int64_t interval, task_state_t state, size_t static_memory_size);
sched_error_t scheduler_set_task_priority(int task_index, int new_priority);
sched_error_t scheduler_set_task_interval(int task_index, int64_t new_interval);
sched_error_t scheduler_pause_task(int task_index);
sched_error_t scheduler_resume_task(int task_index);
sched_error_t scheduler_set_algorithm(sched_algorithm_t algorithm);
sched_algorithm_t scheduler_get_algorithm(void);
void scheduler_run(void);
void scheduler_print_task_list(void);

#endif // SCHEDULER_H
