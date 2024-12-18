#include <stdio.h>
#include "hardware/uart.h"
#include "pico/stdlib.h"

#include "scheduler.h"
#include "initcalls.h"
#include "hardware_cfg.h"

int main()
{
    // Abilita la printf sulla UART0
    stdio_init_all();

    // Seleziona la configurazione hardware attuale
    select_hardware_config();

    // Itera sull'array delle initcall ed esegui ogni funzione
    initcalls();

    // Avviamo lo scheduler
    scheduler_run();

    return 0;
}
