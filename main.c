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

    // Aggiungi alcuni task allo scheduler
    // (name, function, prio, usec)
    scheduler_add_task("led", task_led, 2, 500 * 1000);                         // 0 
    scheduler_add_task("serial_bridge", task_serial_bridge, 1, 1 * 1000);       // 1
    scheduler_add_task("cmd seq", task_at_sequence, 3, 5 * 1000 * 1000);        // 2

    // Scheduler debug 
    // (task number, bool)
    set_debug_for_task(2, false);

    // Avvia lo scheduler (loop)
    scheduler_run();

    return 0;
}
