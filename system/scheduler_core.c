#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "scheduler.h"

// -----------------------------------------------------------------------------
// Configurazioni e Limiti
// -----------------------------------------------------------------------------
#define MAX_TASKS               10
#define DEBUG_BUFFER_SIZE       2048
#define DEBUG_FLUSH_INTERVAL_US 5000000  // Flush ogni 5 secondi
#define DEADLINE_MISS_THRESHOLD 10       // Ignora ritardi inferiori a 10µs
#define LOG_LEVEL_DEFAULT       LOG_LEVEL_INFO

// -----------------------------------------------------------------------------
// Variabili Statistiche e di Stato
// -----------------------------------------------------------------------------
static task_t task_list[MAX_TASKS];
static int    task_count = 0;

static char   debug_buffer[DEBUG_BUFFER_SIZE];
static int    debug_buffer_index = 0;
static absolute_time_t last_debug_flush_time;

static log_level_t global_log_level = LOG_LEVEL_DEFAULT;
static int64_t global_total_task_time = 0;
static sched_algorithm_t selected_algorithm = SCHED_ALGO_PRIORITY;
static int priority_normalization_counter = 0; // Contatore per normalizzazione

// -----------------------------------------------------------------------------
// Funzioni di Logging e Debug
// -----------------------------------------------------------------------------
static void debug_buffer_add_raw(const char *fmt, va_list args) {
    int n = vsnprintf(&debug_buffer[debug_buffer_index], DEBUG_BUFFER_SIZE - debug_buffer_index, fmt, args);
    if (n > 0 && debug_buffer_index + n < DEBUG_BUFFER_SIZE) {
        debug_buffer_index += n;
    } else {
        debug_buffer_index = 0; // Reset in caso di overflow
    }
}

static void debug_log(log_level_t level, const char *fmt, ...) {
    if (level > global_log_level) return;
    absolute_time_t now = get_absolute_time();
    int64_t ts_us = to_us_since_boot(now);

    char prefix[64];
    int p_len = snprintf(prefix, sizeof(prefix), "[%lldus][%d] ", ts_us, level);
    if (p_len > 0 && p_len < DEBUG_BUFFER_SIZE - debug_buffer_index) {
        memcpy(&debug_buffer[debug_buffer_index], prefix, p_len);
        debug_buffer_index += p_len;
    }

    va_list args;
    va_start(args, fmt);
    debug_buffer_add_raw(fmt, args);
    va_end(args);
}

static void debug_buffer_flush(void) {
    absolute_time_t current_time = get_absolute_time();
    if (absolute_time_diff_us(last_debug_flush_time, current_time) >= DEBUG_FLUSH_INTERVAL_US) {
        if (debug_buffer_index > 0) {
            printf("%.*s", debug_buffer_index, debug_buffer);
            debug_buffer_index = 0;
        }
        last_debug_flush_time = current_time;
    }
}

// -----------------------------------------------------------------------------
// Funzioni di Gestione dei Task
// -----------------------------------------------------------------------------
sched_error_t scheduler_add_task(const char *name, task_func_t task, int priority, int64_t interval) {
    if (task_count >= MAX_TASKS) {
        debug_log(LOG_LEVEL_ERROR, "Impossibile aggiungere il task: limite massimo raggiunto.\n");
        return SCHED_ERR_FULL;
    }
    if (task == NULL || interval <= 0) {
        debug_log(LOG_LEVEL_ERROR, "Impossibile aggiungere il task: parametri non validi.\n");
        return SCHED_ERR_INVALID_PARAMS;
    }

    task_t *t = &task_list[task_count];
    memset(t, 0, sizeof(task_t));
    t->task = task;
    t->priority = priority;
    t->interval = interval;
    t->last_execution = make_timeout_time_us(0);
    t->name = name;

    task_count++;
    return SCHED_ERR_OK;
}

sched_error_t scheduler_set_task_debug(int task_index, bool enabled) {
    if (task_index < 0 || task_index >= task_count) return SCHED_ERR_INVALID_INDEX;
    task_list[task_index].debug_enabled = enabled;
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

sched_error_t scheduler_get_task_stats(int task_index, task_stats_t *stats) {
    if (task_index < 0 || task_index >= task_count || stats == NULL) return SCHED_ERR_INVALID_INDEX;

    task_t *t = &task_list[task_index];
    *stats = (task_stats_t){
        .name = t->name,
        .priority = t->priority,
        .dynamic_priority = t->dynamic_priority,
        .interval = t->interval,
        .total_time = t->total_time,
        .max_exec_time = t->max_exec_time,
        .exec_count = t->exec_count,
        .deadline_miss_count = t->deadline_miss_count,
        .max_jitter = t->max_jitter,
        .avg_exec_time = (t->exec_count > 0) ? (t->total_exec_time / t->exec_count) : 0
    };
    return SCHED_ERR_OK;
}

void scheduler_set_log_level(log_level_t level) {
    global_log_level = level;
}

void scheduler_set_algorithm(sched_algorithm_t algorithm) {
    selected_algorithm = algorithm;
}

// -----------------------------------------------------------------------------
// Normalizzazione delle Priorità Dinamiche
// -----------------------------------------------------------------------------
/**
 * Normalizza le priorità dinamiche dei task riportandole al valore iniziale.
 * Questo previene possibili overflow numerici o squilibri a lungo termine
 * nella gestione dei task quando si usa l'algoritmo a priorità.
 */
static void normalize_dynamic_priorities(void) {
    for (int i = 0; i < task_count; i++) {
        task_list[i].dynamic_priority = task_list[i].priority;
    }
}

// -----------------------------------------------------------------------------
// Implementazioni degli Algoritmi di Scheduling
// -----------------------------------------------------------------------------
static int find_highest_priority_task(void) {
    int highest_priority_index = -1;
    int highest_priority = -1;
    absolute_time_t current_time = get_absolute_time();

    for (int i = 0; i < task_count; i++) {
        int64_t since_last = absolute_time_diff_us(task_list[i].last_execution, current_time);
        if (task_list[i].task && since_last >= task_list[i].interval) {
            if (task_list[i].dynamic_priority > highest_priority) {
                highest_priority = task_list[i].dynamic_priority;
                highest_priority_index = i;
            }
        }
    }

    if (highest_priority_index != -1) {
        for (int i = 0; i < task_count; i++) {
            if (i != highest_priority_index) task_list[i].dynamic_priority++;
        }
    }

    // Incrementa il contatore e normalizza se necessario
    if (++priority_normalization_counter >= 100) { // Normalizza ogni 100 iterazioni
        normalize_dynamic_priorities();
        priority_normalization_counter = 0;
    }

    return highest_priority_index;
}

static int find_round_robin_task(void) {
    static int last_task_index = -1;
    for (int i = 0; i < task_count; i++) {
        int current_index = (last_task_index + 1 + i) % task_count;
        int64_t since_last = absolute_time_diff_us(task_list[current_index].last_execution, get_absolute_time());
        if (task_list[current_index].task && since_last >= task_list[current_index].interval) {
            last_task_index = current_index;
            return current_index;
        }
    }
    return -1;
}

static int find_earliest_deadline_task(void) {
    int earliest_index = -1;
    int64_t earliest_deadline = INT64_MAX;
    absolute_time_t current_time = get_absolute_time();

    for (int i = 0; i < task_count; i++) {
        int64_t deadline = to_us_since_boot(task_list[i].last_execution) + task_list[i].interval;
        int64_t since_last = absolute_time_diff_us(task_list[i].last_execution, current_time);

        if (task_list[i].task && since_last >= task_list[i].interval && deadline < earliest_deadline) {
            earliest_deadline = deadline;
            earliest_index = i;
        }
    }

    return earliest_index;
}

static int select_next_task(void) {
    switch (selected_algorithm) {
        case SCHED_ALGO_PRIORITY:
            return find_highest_priority_task();
        case SCHED_ALGO_ROUND_ROBIN:
            return find_round_robin_task();
        case SCHED_ALGO_EARLIEST_DEADLINE_FIRST:
            return find_earliest_deadline_task();
        default:
            return -1;
    }
}

// -----------------------------------------------------------------------------
// Funzione Principale dello Scheduler
// -----------------------------------------------------------------------------
void scheduler_run(void) {
    last_debug_flush_time = get_absolute_time();

    while (1) {
        int task_index = select_next_task();

        if (task_index != -1) {
            task_t *t = &task_list[task_index];
            absolute_time_t start_time = get_absolute_time();

            int64_t expected_start = to_us_since_boot(t->last_execution) + t->interval;
            int64_t actual_start = to_us_since_boot(start_time);
            int64_t jitter = actual_start - expected_start;
            if (jitter < 0) jitter = 0;
            if (jitter > t->max_jitter) t->max_jitter = jitter;

            t->task();

            absolute_time_t end_time = get_absolute_time();
            int64_t task_time = absolute_time_diff_us(start_time, end_time);

            t->total_time += task_time;
            t->last_execution = end_time;
            t->dynamic_priority = t->priority;
            if (task_time > t->max_exec_time) t->max_exec_time = task_time;
            t->total_exec_time += task_time;
            t->exec_count++;

            if ((actual_start - expected_start) > DEADLINE_MISS_THRESHOLD) {
                t->deadline_miss_count++;
            }

            global_total_task_time += task_time;

            if (t->debug_enabled) {
                int64_t current_system_time = to_us_since_boot(get_absolute_time());
                double cpu_usage_percentage = ((double)t->total_time / (double)current_system_time) * 100.0;
                double global_cpu_usage = ((double)global_total_task_time / (double)current_system_time) * 100.0;

                debug_log(LOG_LEVEL_DEBUG, "Task \"%s\" eseguito.\n", t->name);
                debug_log(LOG_LEVEL_INFO,
                          "Task \"%s\" Prio:%d ExecTime:%lld us TotTime:%lld us MaxExec:%lld us AvgExec:%lld us JitterMax:%lld us\n",
                          t->name, t->dynamic_priority, task_time, t->total_time, t->max_exec_time, t->total_exec_time / t->exec_count, t->max_jitter);
                debug_log(LOG_LEVEL_INFO,
                          "CPU Usage Task:%s=%.2f%% CPU Usage Globale=%.2f%%\n",
                          t->name, cpu_usage_percentage, global_cpu_usage);
            }
        } else {
            // sleep_ms(1); // Pausa breve se nessun task è pronto, lo tolgo cosi abbasso il jitter ma aumenteranno i consumi (mW)
        }

        debug_buffer_flush();
    }
}

// -----------------------------------------------------------------------------
// Funzioni di Servizio
// -----------------------------------------------------------------------------
void scheduler_print_all_tasks_stats(void) {
    for (int i = 0; i < task_count; i++) {
        task_stats_t stats;
        if (scheduler_get_task_stats(i, &stats) == SCHED_ERR_OK) {
            debug_log(LOG_LEVEL_INFO,
                      "Task \"%s\" prio:%d dyn_prio:%d interval:%lld us tot_time:%lld us max_exec:%lld us avg_exec:%lld us exec_count:%d deadline_miss:%d max_jitter:%lld\n",
                      stats.name, stats.priority, stats.dynamic_priority, stats.interval, stats.total_time,
                      stats.max_exec_time, stats.avg_exec_time, stats.exec_count, stats.deadline_miss_count, stats.max_jitter);
        }
    }
    debug_buffer_flush();
}
