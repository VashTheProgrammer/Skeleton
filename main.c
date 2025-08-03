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



    // IO EXAMPLE
    #include "IO.h"

    /* Define two separate GPIO structures */
    IO_DEFINE(relays);
    IO_DEFINE(sensors);

    void vMyTask(void *pvParameters) {
        /* Task variables */
        uint8_t var1 = 1;
        uint8_t var2 = 0;

        for (;;) {
            /* Initialize GPIO structure "relays" with 2 elements */
            IO_Init(relays, 2);
            IO_SetElement(relays, 0, (GPIO_PortPin_t){GPIOA, GPIO_PIN_5}, var1);
            IO_SetElement(relays, 1, (GPIO_PortPin_t){GPIOB, GPIO_PIN_3}, var2);

            /* Write values to GPIO pins */
            IO_WriteAll(relays);

            /* Initialize GPIO structure "sensors" with 2 elements for reading inputs */
            IO_Init(sensors, 2);
            IO_SetElement(sensors, 0, (GPIO_PortPin_t){GPIOC, GPIO_PIN_1}, 0);
            IO_SetElement(sensors, 1, (GPIO_PortPin_t){GPIOC, GPIO_PIN_2}, 0);

            /* Read values from GPIO pins */
            IO_GetAll(sensors);

            /* Access sensor values */
            uint8_t sensor1_val = IO_Get_sensors()->list[0].value;
            uint8_t sensor2_val = IO_Get_sensors()->list[1].value;

            /* Implement your logic with sensor values */
            (void)sensor1_val;
            (void)sensor2_val;

            /* Delay for task scheduling */
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }





    return 0; // Although unreachable, it's a standard practice
}
