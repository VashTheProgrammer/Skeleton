#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <time.h>  // Per misurare il tempo di esecuzione

#define MAX_TASKS 10

typedef void (*task_func_t)(void); // Definizione di tipo per un puntatore a funzione task

typedef struct {
    task_func_t task;
    int priority;
    clock_t interval;
    clock_t last_execution;
    clock_t start_time;
    clock_t total_time;
} task_t;

void scheduler_add_task(task_func_t task, int priority, clock_t interval);
void scheduler_run(void);

#endif // SCHEDULER_H
