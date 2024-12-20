#ifndef DRIVER_LED_H
#define DRIVER_LED_H

#include <stdint.h>
#include <time.h>
#include <pico/types.h>

// Enum to define LED states
typedef enum {
    LED_OFF = 0,
    LED_ON = 1
} LedState;

/*
 * Struct for LED driver operations.
 * This approach mimics the class and object paradigm in object-oriented languages.
 * - The struct acts as the "class" containing attributes (fields) and methods (function pointers).
 * - Function pointers represent "methods" that operate on the instance ("object").
 */
typedef struct DriverLed {
    int led_pin; // GPIO pin for the LED
    LedState led_state; // Current state of the LED
    absolute_time_t fade_timer; // Timer for fade operations
    uint16_t fade_level; // Current brightness level
    uint16_t fade_target; // Target brightness level
    uint32_t fade_step_time; // Time per fade step (ms)
    bool fade_in_progress; // Flag for ongoing fade

    // Function pointers for LED operations
    void (*init)(struct DriverLed *driver, int gpio_pin); // Constructor-like method
    void (*on)(struct DriverLed *driver); // Turns LED on
    void (*off)(struct DriverLed *driver); // Turns LED off
    void (*toggle)(struct DriverLed *driver); // Toggles LED state
    void (*fade_in)(struct DriverLed *driver, uint32_t duration_ms); // Fades LED in
    void (*fade_out)(struct DriverLed *driver, uint32_t duration_ms); // Fades LED out
    void (*process_fade)(struct DriverLed *driver); // Processes ongoing fades
    void (*set_brightness)(struct DriverLed *driver, uint8_t brightness); // Sets specific brightness
} DriverLed;

// Initializes the LED driver
void initialize_driver_led(DriverLed *driver, int gpio_pin);

#endif // DRIVER_LED_H
