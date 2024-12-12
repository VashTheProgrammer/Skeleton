#include <stdio.h>
#include "hardware/uart.h"
#include "pico/stdlib.h"

#include "scheduler.h"
#include "initcalls.h"
#include "hardware_config.h"

#include "task_led.h"
#include "task_serial_bridge.h"
#include "task_at_sequence.h"

int main()
{
    // Abilita la printf sulla UART0
    stdio_init_all();

    // Seleziona la configurazione hardware attuale
    select_hardware_config();

    // Itera sull'array delle initcall ed esegui ogni funzione
    initcalls();

    // Impostiamo un livello di log globale a INFO, così vedremo messaggi importanti
    scheduler_set_log_level(LOG_LEVEL_INFO);

    /*
    SCHED_ALGO_PRIORITY: Priorità statica/dinamica (default).
    Ideale per task con priorità chiaramente definite e differenze significative nell'importanza.
   
    SCHED_ALGO_ROUND_ROBIN: Rotazione equa dei task.
    Utile per sistemi multitasking semplici dove tutti i task hanno uguale priorità.
    
    SCHED_ALGO_EARLIEST_DEADLINE_FIRST: Deadline più vicina.
    Adatto per sistemi real-time con scadenze stringenti.
    */
    // Impostiamo l'algoritmo di scheduling che più si adatta alla natura dei nostri task 
    scheduler_set_algorithm(SCHED_ALGO_ROUND_ROBIN); // ROUND_ROBIN sempre una garanzia

    // Aggiungiamo alcuni task
    // Parametri: nome, funzione, priorità, intervallo in microsecondi
    // Supponiamo che task_blink sia relativamente frequente (500 ms) e abbia priorità 2
    if (scheduler_add_task("led", task_led, 2, 500 * 1000) != SCHED_ERR_OK) {
        printf("Errore nell'aggiunta di Blink.\n");
    }
/*
    // Aggiungiamo un task sensori con priorità più alta (3) e intervallo corto (1 ms)
    if (scheduler_add_task("serial_bridge", task_serial_bridge, 3, 1 * 1000) != SCHED_ERR_OK) {
        printf("Errore nell'aggiunta di serial_bridge.\n");
    }

    // Aggiungiamo un task di comunicazione con priorità (1) e intervallo 5s
    if (scheduler_add_task("at_sequence", task_at_sequence, 1, 5 * 1000 * 1000) != SCHED_ERR_OK) {
        printf("Errore nell'aggiunta di task_at_sequence.\n");
    }
*/
    // Abilitiamo il debug per il task "led" (che è il secondo task aggiunto, indice 1)
    // Di default è già false
    scheduler_set_task_debug(0, false);
    scheduler_set_task_debug(1, false);
    scheduler_set_task_debug(2, false);

    // Modifichiamo la priorità del task "at_sequence" (indice 2) a 2
    scheduler_set_task_priority(2, 2);

    // Modifichiamo l'intervallo del led (indice 0) a 300 ms invece di 500 ms
    scheduler_set_task_interval(0, 300 * 1000);

    // Stampiamo le statistiche attuali (probabilmente vuote, dato che non abbiamo ancora eseguito nulla)
    // printf("Statistiche prima dell'avvio dello scheduler:\n");
    // scheduler_print_all_tasks_stats();

    // Avviamo lo scheduler
    scheduler_run();

    return 0;
}
