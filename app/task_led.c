#include <stdio.h>
#include "task_led.h"
#include "driver_led.h"
#include "initcalls.h"
#include "scheduler.h"

#include "hardware_cfg.h"

// Declare static instances for LED drivers
static DriverLed leds[2];

// Struttura dati del mio task
typedef struct {
    int buffer[256]; // 256 * 4 = 1024 byte (assuming int=4 byte)
    struct {
        char name[32];
        int value;
    } config; // 32 + 4 = 36 byte
} task_led_static_mem_t;

static task_led_static_mem_t led_data; // istanza statica della struttura

void task_led(void) {
    leds[0].toggle(&leds[0]);
    
}

void task_led_init(void) {

    // Initialize multiple LED drivers
    initialize_driver_led(&leds[0], hw_config->led_pin); // Assign pin 25 to led1
    initialize_driver_led(&leds[1], hw_config->extra_gpio1); // Assign pin 26 to led2

    // Parametri: nome, funzione, priorità, intervallo in microsecondi, uso di memoria statica
    if (scheduler_add_task("led", task_led, 0, 500 * 1000, sizeof(task_led_static_mem_t)) != SCHED_ERR_OK) {
        printf("Errore nell'aggiunta di Blink.\n");
    }
}
REGISTER_INITCALL(task_led_init);

