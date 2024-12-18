#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "scheduler.h"

// -----------------------------------------------------------------------------
// Variabili Statistiche e di Stato
// -----------------------------------------------------------------------------
static task_t task_list[MAX_TASKS];
static int    task_count = 0;
static sched_algorithm_t selected_algorithm = SCHED_ALGO_ROUND_ROBIN;
static int    priority_normalization_counter = 0;  // Contatore per la normalizzazione delle priorità
static int64_t global_total_task_time = 0;          // Tempo totale accumulato dai task

// Stack di ogni task
static uint8_t task_stacks[MAX_TASKS][TASK_STACK_SIZE];

// -----------------------------------------------------------------------------
// Forward declaration delle funzioni di scheduling
// -----------------------------------------------------------------------------
static int find_highest_priority_task(absolute_time_t current_time);
static int find_round_robin_task(absolute_time_t current_time);
static int find_earliest_deadline_task(absolute_time_t current_time);
static int find_least_executed_task(absolute_time_t current_time);
static int find_longest_waiting_task(absolute_time_t current_time);

// Tabella di funzioni per gli algoritmi di scheduling
typedef int (*sched_func_t)(absolute_time_t current_time);
static sched_func_t sched_algorithms[] = {
    find_highest_priority_task,     // SCHED_ALGO_PRIORITY
    find_round_robin_task,          // SCHED_ALGO_ROUND_ROBIN
    find_earliest_deadline_task,    // SCHED_ALGO_EARLIEST_DEADLINE_FIRST
    find_least_executed_task,       // SCHED_ALGO_LEAST_EXECUTED
    find_longest_waiting_task       // SCHED_ALGO_LONGEST_WAITING
};

// -----------------------------------------------------------------------------
// Funzioni di Supporto per lo Stack Usage
// -----------------------------------------------------------------------------
static void initialize_task_stack(uint8_t *stack, size_t size) {
    // Inizializza lo stack con un valore noto per monitorare l'uso dello stack.
    memset(stack, STACK_FILL_VALUE, size);
}

static int calculate_stack_usage(const uint8_t *stack, size_t size) {
    // Calcola l'uso dello stack trovando la prima posizione diversa dal valore noto.
    int used = 0;
    while (used < (int)size && stack[used] != STACK_FILL_VALUE) {
        used++;
    }
    return used;
}

// -----------------------------------------------------------------------------
// Funzioni di Gestione dei Task
// -----------------------------------------------------------------------------
sched_error_t scheduler_add_task(const char *name, task_func_t task, int priority, int64_t interval, task_state_t state, size_t static_memory_size) {
    if (task_count >= MAX_TASKS) {
        // Errore se si supera il numero massimo di task
        return SCHED_ERR_FULL;
    }
    if (task == NULL || interval <= 0) {
        // Parametri non validi
        return SCHED_ERR_INVALID_PARAMS;
    }

    task_t *t = &task_list[task_count];
    memset(t, 0, sizeof(task_t));
    t->task = task;
    t->state = state;
    t->priority = priority;
    t->dynamic_priority = priority;
    t->interval = interval;
    t->last_execution = make_timeout_time_us(0);
    t->name = name;
    t->min_exec_time = INT64_MAX;
    t->memory_allocated = static_memory_size; // Memoria statica associata al task

    // Inizializza lo stack del task
    initialize_task_stack(task_stacks[task_count], TASK_STACK_SIZE);

    task_count++;
    return SCHED_ERR_OK;
}

sched_error_t scheduler_set_task_priority(int task_index, int new_priority) {
    if (task_index < 0 || task_index >= task_count) return SCHED_ERR_INVALID_INDEX;
    task_list[task_index].priority = new_priority;
    task_list[task_index].dynamic_priority = new_priority;
    return SCHED_ERR_OK;
}

sched_error_t scheduler_set_task_interval(int task_index, int64_t new_interval) {
    if (task_index < 0 || task_index >= task_count) return SCHED_ERR_INVALID_INDEX;
    task_list[task_index].interval = new_interval;
    return SCHED_ERR_OK;
}

sched_error_t scheduler_pause_task(int task_index) {
    if (task_index < 0 || task_index >= task_count) return SCHED_ERR_INVALID_INDEX;
    task_list[task_index].state = TASK_PAUSED;

    // Reset delle statistiche del jitter
    task_list[task_index].total_jitter = 0;
    task_list[task_index].max_jitter = 0;

    // Aggiorna il tempo dell'ultima esecuzione per evitare calcoli di jitter errati
    task_list[task_index].last_execution = get_absolute_time();

    return SCHED_ERR_OK;
}

sched_error_t scheduler_resume_task(int task_index) {
    if (task_index < 0 || task_index >= task_count) return SCHED_ERR_INVALID_INDEX;
    task_list[task_index].state = TASK_RUNNING;

    // Reset delle statistiche del jitter
    task_list[task_index].total_jitter = 0;
    task_list[task_index].max_jitter = 0;

    // Aggiorna il tempo dell'ultima esecuzione per evitare calcoli di jitter errati
    task_list[task_index].last_execution = get_absolute_time();

    return SCHED_ERR_OK;
}

sched_error_t scheduler_set_algorithm(sched_algorithm_t algorithm) {
    // Metti in pausa tutti i task
    for (int i = 0; i < task_count; i++) {
        task_list[i].state = TASK_PAUSED;
    }

    // Cambia l'algoritmo dello scheduler
    selected_algorithm = algorithm;

    // Resetta tutte le statistiche dei task
    for (int i = 0; i < task_count; i++) {
        task_t *t = &task_list[i];
        t->exec_count = 0;
        t->total_time = 0;
        t->total_exec_time = 0;
        t->max_exec_time = 0;
        t->min_exec_time = INT64_MAX;
        t->total_jitter = 0;
        t->max_jitter = 0;
        t->last_execution = get_absolute_time();
    }

    // Riporta i task in stato RUNNING
    for (int i = 0; i < task_count; i++) {
        task_list[i].state = TASK_RUNNING;
    }

    return SCHED_ERR_OK;
}

// -----------------------------------------------------------------------------
// Normalizzazione delle Priorità Dinamiche
// -----------------------------------------------------------------------------
static void normalize_dynamic_priorities(void) {
    for (int i = 0; i < task_count; i++) {
        task_list[i].dynamic_priority = task_list[i].priority;
    }
}

// -----------------------------------------------------------------------------
// Implementazioni degli Algoritmi di Scheduling
// -----------------------------------------------------------------------------

// La ALGO PRIORITY E' L'UNICA CHE USA IL MECCANISMO  DELLA PRIORITA'
static int find_highest_priority_task(absolute_time_t current_time) {
    int highest_priority_index = -1;
    int highest_priority = -1;

    for (int i = 0; i < task_count; i++) {
        if (task_list[i].state == TASK_PAUSED) continue;
        int64_t since_last = absolute_time_diff_us(task_list[i].last_execution, current_time);
        if (task_list[i].task && since_last >= task_list[i].interval) {
            if (task_list[i].dynamic_priority > highest_priority) {
                highest_priority = task_list[i].dynamic_priority;
                highest_priority_index = i;
            }
        }
    }

    // Normalizza le priorità dinamiche ogni PRIORITY_NORMALIZATION_INTERVAL esecuzioni.
    if (++priority_normalization_counter >= PRIORITY_NORMALIZATION_INTERVAL) {
        normalize_dynamic_priorities();
        priority_normalization_counter = 0;
    }

    return highest_priority_index;
}

static int find_round_robin_task(absolute_time_t current_time) {
    static int last_task_index = -1;
    for (int i = 0; i < task_count; i++) {
        int current_index = (last_task_index + 1 + i) % task_count;
        if (task_list[current_index].state == TASK_PAUSED) continue;
        int64_t since_last = absolute_time_diff_us(task_list[current_index].last_execution, current_time);
        if (task_list[current_index].task && since_last >= task_list[current_index].interval) {
            last_task_index = current_index;
            return current_index;
        }
    }
    return -1;
}

static int find_earliest_deadline_task(absolute_time_t current_time) {
    int earliest_index = -1;
    int64_t earliest_deadline = INT64_MAX;

    for (int i = 0; i < task_count; i++) {
        if (task_list[i].state == TASK_PAUSED) continue;
        int64_t deadline = to_us_since_boot(task_list[i].last_execution) + task_list[i].interval;
        int64_t since_last = absolute_time_diff_us(task_list[i].last_execution, current_time);
        if (task_list[i].task && since_last >= task_list[i].interval && deadline < earliest_deadline) {
            earliest_deadline = deadline;
            earliest_index = i;
        }
    }

    return earliest_index;
}

static int find_least_executed_task(absolute_time_t current_time) {
    (void)current_time; // Non serve in questo algoritmo
    int least_executed_index = -1;
    int min_exec_count = INT_MAX;

    for (int i = 0; i < task_count; i++) {
        if (task_list[i].state == TASK_PAUSED) continue;
        if (task_list[i].exec_count < min_exec_count) {
            min_exec_count = task_list[i].exec_count;
            least_executed_index = i;
        }
    }

    return least_executed_index;
}

static int find_longest_waiting_task(absolute_time_t current_time) {
    int longest_waiting_index = -1;
    int64_t max_wait_time = -1;

    for (int i = 0; i < task_count; i++) {
        if (task_list[i].state == TASK_PAUSED) continue;
        int64_t wait_time = absolute_time_diff_us(task_list[i].last_execution, current_time);
        if (wait_time > max_wait_time) {
            max_wait_time = wait_time;
            longest_waiting_index = i;
        }
    }

    return longest_waiting_index;
}

static int select_next_task(absolute_time_t current_time) {
    int algo_index = (int)selected_algorithm;
    if (algo_index < 0 || algo_index >= (int)(sizeof(sched_algorithms)/sizeof(sched_algorithms[0]))) {
        return -1;
    }
    return sched_algorithms[algo_index](current_time);
}

// -----------------------------------------------------------------------------
// Funzione Principale dello Scheduler
// -----------------------------------------------------------------------------
void scheduler_run(void) {
    while (1) {
        absolute_time_t current_time = get_absolute_time();
        int task_index = select_next_task(current_time);

        if (task_index != -1) {
            task_t *t = &task_list[task_index];

            int64_t since_last = absolute_time_diff_us(t->last_execution, current_time);
            
            // Calcolo del jitter
            int64_t jitter = since_last - t->interval;
            if (jitter < 0) jitter = -jitter;
            t->total_jitter += jitter;
            if (jitter > t->max_jitter) {
                t->max_jitter = jitter;
            }

            // Esegui il task
            absolute_time_t start_time = current_time;
            t->task();
            absolute_time_t end_time = get_absolute_time();

            t->last_execution = end_time;
            t->dynamic_priority = t->priority;
            t->exec_count++;

            int64_t exec_time = absolute_time_diff_us(start_time, end_time);
            t->total_time += exec_time;
            t->total_exec_time += exec_time;
            if (exec_time > t->max_exec_time) t->max_exec_time = exec_time;
            if (exec_time < t->min_exec_time) t->min_exec_time = exec_time;

            global_total_task_time += exec_time;
        } else {
            // Nessun task eseguibile, introduce un piccolo idle
            // sleep_us(100);
        }
    }
}
// -----------------------------------------------------------------------------
// Stampa informazioni sui Task e Statistiche
// -----------------------------------------------------------------------------

// Funzioni di supporto
const char* scheduler_algorithm_to_string(sched_algorithm_t algorithm) {
    switch (algorithm) {
        case SCHED_ALGO_PRIORITY: return "PRIORITY";
        case SCHED_ALGO_ROUND_ROBIN: return "ROUND_ROBIN";
        case SCHED_ALGO_EARLIEST_DEADLINE_FIRST: return "EARLIEST_DEADLINE_FIRST";
        case SCHED_ALGO_LEAST_EXECUTED: return "LEAST_EXECUTED";
        case SCHED_ALGO_LONGEST_WAITING: return "LONGEST_WAITING";
        default: return "UNKNOWN";
    }
}

static void print_task_info(int index, const task_t *task, int stack_used) {
    printf("%-5d %-10s %-10s %-10d %-10d %-10lld %-10lld %-10lld %-10lld %-10lld %-10lld %-10zu\n",
           index,
           task->name,
           task->state == TASK_RUNNING ? "RUNNING" : "PAUSED",
           task->priority,
           task->exec_count,
           task->total_time,
           (task->min_exec_time == INT64_MAX) ? 0 : task->min_exec_time,
           task->max_exec_time,
           task->exec_count > 0 ? (task->total_exec_time / task->exec_count) : 0,
           task->max_jitter,
           task->exec_count > 0 ? (task->total_jitter / task->exec_count) : 0,
           stack_used + task->memory_allocated);
}


void scheduler_print_task_list(void) {
    const char *algo_name = scheduler_algorithm_to_string(selected_algorithm);

    int64_t current_system_time = to_us_since_boot(get_absolute_time());
    double cpu_usage_percentage = ((double)global_total_task_time / (double)current_system_time) * 100.0;

    // Calcolo memoria totale utilizzata
    size_t total_memory_usage = 0;
    for (int i = 0; i < task_count; i++) {
        int stack_used = calculate_stack_usage(task_stacks[i], TASK_STACK_SIZE);
        total_memory_usage += (stack_used + task_list[i].memory_allocated);
    }
    double memory_usage_percentage = ((double)total_memory_usage / (double)RP2040_TOTAL_RAM) * 100.0;

    // Stampa statistiche globali
    printf("\n--- Global Task Statistics ---\n");
    printf("Scheduler Algorithm: %s\n", algo_name);
    printf("CPU Usage: %.2f%% (%lld us)\n", cpu_usage_percentage, global_total_task_time);
    printf("Total System Time: %lld us\n", current_system_time);
    printf("Total Memory Usage: %zu bytes (%.2f%% of total 264KB RAM)\n\n", total_memory_usage, memory_usage_percentage);

    printf("%-5s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
       "PID", "Name", "State", "Priority", "ExecCount", "TotalTime",
       "MinTime", "MaxTime", "AvgTime", "MaxJitter", "AvgJitter", "MemUsed");

    for (int i = 0; i < task_count; i++) {
        int stack_used = calculate_stack_usage(task_stacks[i], TASK_STACK_SIZE);
        print_task_info(i, &task_list[i], stack_used);
    }
    printf("\n");
}
