#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "scheduler.h"

#define MAX_TASKS 10 // Definizione del numero massimo di task

#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

// Buffer circolare per memorizzare i messaggi di debug
#define DEBUG_BUFFER_SIZE 2048
static char debug_buffer[DEBUG_BUFFER_SIZE];
static int debug_buffer_index = 0;
static absolute_time_t last_debug_flush_time;

// Selezione dei task per il debug
static bool debug_enabled_for_task[MAX_TASKS] = {false};

// Variabili per il debug dei task
static int64_t max_execution_time[MAX_TASKS] = {0};
static int64_t total_execution_time[MAX_TASKS] = {0};
static int execution_count[MAX_TASKS] = {0};

static task_t task_list[MAX_TASKS];   // Array per memorizzare i task
static int task_count = 0;            // Contatore del numero di task registrati

// Funzione per aggiungere messaggi al buffer di debug
void debug_buffer_add(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(&debug_buffer[debug_buffer_index], DEBUG_BUFFER_SIZE - debug_buffer_index, fmt, args);
    va_end(args);

    if (n > 0) {
        debug_buffer_index = (debug_buffer_index + n) % DEBUG_BUFFER_SIZE;
    }
}

// Funzione per stampare il buffer di debug
void debug_buffer_flush(void) {
    absolute_time_t current_time = get_absolute_time();
    if (absolute_time_diff_us(last_debug_flush_time, current_time) >= 5000000) { // Flush ogni 5 secondi
        printf("%s", debug_buffer);
        debug_buffer_index = 0;
        last_debug_flush_time = current_time;
    }
}

// Funzione per aggiungere un task allo scheduler
void scheduler_add_task(const char *name, task_func_t task, int priority, int64_t interval) {
    if (task_count < MAX_TASKS) {
        task_list[task_count].task = task;
        task_list[task_count].priority = priority;
        task_list[task_count].interval = interval;
        task_list[task_count].last_execution = make_timeout_time_us(0);
        task_list[task_count].total_time = 0;
        task_list[task_count].dynamic_priority = priority; // Priorità dinamica inizializzata
        task_list[task_count].name = name;
        task_count++;
    } else {
        printf("Impossibile aggiungere il task: limite massimo raggiunto.\n");
    }
}

// Funzione per trovare il prossimo task con la priorita' piu' alta
int find_highest_priority_task(void) {
    int highest_priority_index = -1;
    int highest_priority = -1;
    absolute_time_t current_time = get_absolute_time();

    for (int i = 0; i < task_count; i++) {
        // Controlla se il task è pronto per essere eseguito
        if (task_list[i].task && absolute_time_diff_us(task_list[i].last_execution, current_time) >= task_list[i].interval) {
            // Se la priorita' è più alta, seleziona questo task
            if (task_list[i].dynamic_priority > highest_priority) {
                highest_priority = task_list[i].dynamic_priority;
                highest_priority_index = i;
            }
        }
    }

    // Aumenta la priorita' dinamica dei task che non sono stati selezionati (per evitare starvation)
    for (int i = 0; i < task_count; i++) {
        if (i != highest_priority_index) {
            task_list[i].dynamic_priority++;
        }
    }

    return highest_priority_index;
}

// Funzione per abilitare o disabilitare il debug per un task specifico
void set_debug_for_task(int task_index, bool enabled) {
    if (task_index >= 0 && task_index < MAX_TASKS) {
        debug_enabled_for_task[task_index] = enabled;
    }
}

// Funzione principale dello scheduler
void scheduler_run(void) {
    last_debug_flush_time = get_absolute_time();

    while (1) {
        int task_index = find_highest_priority_task();

        // Esegui il task con la priorita' piu' alta trovato
        if (task_index != -1) {
            int i = task_index;
            absolute_time_t current_time = get_absolute_time();

            // Misura il tempo di start del task
            absolute_time_t start_time = get_absolute_time();

            // Esegui il task corrente
            task_list[i].task();

            // Misura il tempo di fine e calcola il tempo totale di esecuzione
            absolute_time_t end_time = get_absolute_time();
            int64_t task_time = absolute_time_diff_us(start_time, end_time);

            // Aggiorna il tempo totale di esecuzione del task
            task_list[i].total_time += task_time;
            task_list[i].last_execution = current_time;

            // Aggiorna la priorita' dinamica del task (reset alla priorita' originale)
            task_list[i].dynamic_priority = task_list[i].priority;

            // Aggiungi il tempo di esecuzione del task al buffer di debug se il debug è abilitato per questo task
            if (debug_enabled_for_task[i]) {
                // Verifica se il task ha mancato la deadline
                if (absolute_time_diff_us(task_list[i].last_execution, current_time) > task_list[i].interval) {
                    debug_buffer_add("Task %s: DEADLINE MISSED\n", task_list[i].name);
                }

                // Aggiorna il tempo di esecuzione massimo e medio
                if (task_time > max_execution_time[i]) {
                    max_execution_time[i] = task_time;
                }

                total_execution_time[i] += task_time;
                execution_count[i]++;
                int64_t average_execution_time = total_execution_time[i] / execution_count[i];
                
                debug_buffer_add("Task \"%s\", Prio: %d \nTempo di esecuzione = %lld us, Tempo totale = %lld us\n", task_list[i].name, task_list[i].dynamic_priority, task_time, task_list[i].total_time);
                debug_buffer_add("Tempo massimo di esecuzione = %lld us, Tempo medio di esecuzione = %lld us\n", task_list[i].name, max_execution_time[i], average_execution_time);

                // Calcola e aggiungi il carico del sistema al buffer di debug
                int64_t current_system_time = to_us_since_boot(get_absolute_time());
                double cpu_usage_percentage = ((double)task_list[i].total_time / (double)current_system_time) * 100.0;

                debug_buffer_add("Utilizzo CPU: %.2f%%, Tempo sistema totale: %lld us\n", cpu_usage_percentage, current_system_time);

                // Aggiungi la disponibilità residua al buffer di debug
                debug_buffer_add("Disponibilità residua CPU: %.2f%%\n", 100.0 - cpu_usage_percentage);
            }
        } else {
            // Se nessun task è pronto, metti la CPU in sleep per risparmiare energia
            sleep_ms(1);
        }

        // Stampa il buffer di debug periodicamente
        debug_buffer_flush();
    }
}
