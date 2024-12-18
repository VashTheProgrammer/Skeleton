#include <stdio.h>
#include "hardware/uart.h"
#include "pico/stdlib.h"

#include "scheduler.h"
#include "initcalls.h"
#include "hardware_cfg.h"

#include "task_led.h"

int main()
{
    // Abilita la printf sulla UART0
    stdio_init_all();

    // Seleziona la configurazione hardware attuale
    select_hardware_config();

    // Itera sull'array delle initcall ed esegui ogni funzione
    initcalls();

    // Impostiamo l'algoritmo di scheduling che più si adatta alla natura dei nostri task 
    scheduler_set_algorithm(SCHED_ALGO_ROUND_ROBIN); // ROUND_ROBIN sempre una garanzia

    // Avviamo lo scheduler
    scheduler_run();

    return 0;
}
