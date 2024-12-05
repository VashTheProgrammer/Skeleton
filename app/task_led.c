#include "task_led.h"
#include "circular_buffer.h"
#include "driver_led.h"
#include "initcalls.h"

#include "hardware_config.h"

static circular_buffer_t queue_example;

// Declare static instances for LED drivers
static DriverLed leds[2];

void task_led_init(void) {

    // i pin li leggi da hw_config

    // Initialize multiple LED drivers
    initialize_driver_led(&leds[0], hw_config->led_pin); // Assign pin 25 to led1
    initialize_driver_led(&leds[1], hw_config->extra_gpio1); // Assign pin 26 to led2

    circular_buffer_init(&queue_example);
}
REGISTER_INITCALL(task_led_init);


void task_led(void) {
    data_packet_t data;
    
    // Se riceviamo dei dati li mettiamo in coda per altri task
    circular_buffer_enqueue(&queue_example, data);

    leds[0].toggle(&leds[0]);
}