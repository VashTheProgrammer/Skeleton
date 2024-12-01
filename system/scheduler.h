#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <time.h>  // Per misurare il tempo di esecuzione

#define MAX_TASKS 10

typedef void (*task_func_t)(void);

typedef struct {
    task_func_t task;          // Funzione del task
    int priority;              // Priorità del task
    int64_t interval;          // Intervallo tra le esecuzioni in microsecondi
    absolute_time_t last_execution; // Tempo dell'ultima esecuzione
    int64_t total_time;        // Tempo totale di esecuzione
    int dynamic_priority;      // Priorità dinamica per evitare starvation
    const char *name;          // Nome del task

} task_t;

void scheduler_add_task(const char *name, task_func_t task, int priority, int64_t interval);
void scheduler_run(void);
void set_debug_for_task(int task_index, bool enabled);

#endif // SCHEDULER_H
