#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "scheduler.h"

#include <time.h>
#include <stdio.h>

// Abilita o disabilita il debug modificando il valore di DEBUG_ENABLE
#define DEBUG_ENABLE 0

// Definisci una macro per il debug
#if DEBUG_ENABLE
    #define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) // Macro vuota, non fa nulla
#endif

static task_t task_list[MAX_TASKS];   // Array per memorizzare i task
static int task_count = 0;            // Contatore del numero di task registrati
static clock_t start_time_system;     // Tempo di inizio dell'intero sistema
static clock_t total_system_time = 0; // Tempo totale di esecuzione del sistema

// Funzione per aggiungere un task allo scheduler
void scheduler_add_task(task_func_t task, int priority, clock_t interval) {
    if (task_count < MAX_TASKS) {
        task_list[task_count].task = task;
        task_list[task_count].priority = priority;
        task_list[task_count].interval = interval;
        task_list[task_count].last_execution = 0;
        task_list[task_count].total_time = 0;
        task_count++;
    } else {
        printf("Impossibile aggiungere il task: limite massimo raggiunto.\n");
    }
}

// Funzione principale dello scheduler
void scheduler_run(void) {
    start_time_system = clock(); // Registra il tempo di avvio del sistema

    while (1) {
        int highest_priority_index = -1;
        int highest_priority = -1;

        // Trova il task con la priorità più alta che è pronto per essere eseguito
        for (int i = 0; i < task_count; i++) {
            clock_t current_time = clock();
            if (task_list[i].task && (current_time - task_list[i].last_execution >= task_list[i].interval)) {
                if (task_list[i].priority > highest_priority) {
                    highest_priority = task_list[i].priority;
                    highest_priority_index = i;
                }
            }
        }

        // Esegui il task con la priorità più alta trovato
        if (highest_priority_index != -1) {
            int i = highest_priority_index;
            clock_t current_time = clock();

            // Misura il tempo di start del task
            clock_t start_time = clock();

            // Esegui il task corrente
            task_list[i].task();

            // Misura il tempo di fine e calcola il tempo totale di esecuzione
            clock_t end_time = clock();
            clock_t task_time = end_time - start_time;

            // Aggiorna il tempo totale di esecuzione del task
            task_list[i].total_time += task_time;
            task_list[i].last_execution = current_time;

            // Aggiorna il tempo totale di esecuzione del sistema
            total_system_time += task_time;

            // Stampa il tempo di esecuzione del task
            DEBUG_PRINT("Task %d: Tempo di esecuzione = %ld ticks, Tempo totale = %ld ticks\n", i, task_time, task_list[i].total_time);

            // Calcola e stampa il carico del sistema
            #ifdef DEBUG_ENABLE
            clock_t current_system_time = clock() - start_time_system;
            double cpu_usage_percentage = ((double)total_system_time / (double)current_system_time) * 100.0;
            #endif
            DEBUG_PRINT("Utilizzo CPU: %.2f%%, Tempo sistema totale: %ld ticks\n", cpu_usage_percentage, current_system_time);

            // Stampa la disponibilità residua
            DEBUG_PRINT("Disponibilità residua CPU: %.2f%%\n", 100.0 - cpu_usage_percentage);
        }
    }
}