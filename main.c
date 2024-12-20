#include <stdio.h>
#include "hardware/uart.h"
#include "pico/stdlib.h"

#include "scheduler.h"
#include "initcalls.h"
#include "hardware_cfg.h"
#include "config.h"

// Main entry point of the program
int main() {
    // Initialize UART0 for printf functionality
    // This allows debug output and communication over UART
    stdio_init_all();

    // Select the active hardware configuration
    // Based on predefined macros, selects hardware_v1 or hardware_v2
    select_hardware_config();

    // Execute all registered initialization functions
    // Uses the initcalls mechanism to set up drivers, hardware, and other modules
    initcalls();

    // Initialize configuration parameters from `secret.c`
    // Sets default system parameters for runtime operation
    init_params();

    // Start the scheduler to manage and execute tasks
    // This enters an infinite loop to run scheduled tasks based on the selected algorithm
    scheduler_run();

    return 0; // Although unreachable, it's a standard practice
}
