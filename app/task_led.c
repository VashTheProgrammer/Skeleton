#include <stdio.h>
#include "task_led.h"
#include "driver_led.h"
#include "initcalls.h"
#include "scheduler.h"
#include "debug.h"

#include "hardware_cfg.h"

// Static instances for LED drivers
static DriverLed leds[2];

// Static memory structure for the LED task
typedef struct {
    int buffer[256]; // 1 KB buffer for internal use
    struct {
        char name[32];
        int value;
    } config; // Configuration data (36 bytes)
} task_led_static_mem_t;

static task_led_static_mem_t led_data; // Static instance of LED task data

// Task function: Toggles LEDs and logs execution
void task_led(void) {
    leds[0].toggle(&leds[0]);
    DEBUG_LOG_TASK(0, "LED task executed.");
}

// Initializes the LED task and registers it with the scheduler
void task_led_init(void) {
    // Initialize LED drivers with hardware configurations
    initialize_driver_led(&leds[0], hw_config->led_pin); // Pin 25 for LED 1
    initialize_driver_led(&leds[1], hw_config->extra_gpio1); // Pin 26 for LED 2

    // Add task to scheduler: name, function, priority, interval, state, and memory usage
    if (scheduler_add_task("led01", task_led, 0, (1 * 1000 * 1000), TASK_RUNNING, sizeof(task_led_static_mem_t)) != SCHED_ERR_OK) {
        printf("[LED TASK][ERROR] Failed to add Blink task.\n");
    }
}

// Registers the LED task initialization function to run at startup
REGISTER_INITCALL(task_led_init);
