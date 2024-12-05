#ifndef DRIVER_LED_H
#define DRIVER_LED_H

#include <stdint.h>
#include <time.h>
#include <pico/types.h>

// CLASSE E OGGETTI IN C

// Enum to represent the state of the LED
typedef enum {
    LED_OFF = 0,
    LED_ON = 1
} LedState;

// Struct specifically for LED operations
typedef struct DriverLed {
    int led_pin;
    LedState led_state;
    absolute_time_t fade_timer;
    uint16_t fade_level;
    uint16_t fade_target;
    uint32_t fade_step_time;
    bool fade_in_progress;
    
    void (*init)(struct DriverLed *driver, int gpio_pin);
    void (*on)(struct DriverLed *driver);
    void (*off)(struct DriverLed *driver);
    void (*toggle)(struct DriverLed *driver);
    void (*fade_in)(struct DriverLed *driver, uint32_t duration_ms);
    void (*fade_out)(struct DriverLed *driver, uint32_t duration_ms);
    void (*process_fade)(struct DriverLed *driver);
    void (*set_brightness)(struct DriverLed *driver, uint8_t brightness);
} DriverLed;

void initialize_driver_led(DriverLed *driver, int gpio_pin);

#endif // DRIVER_LED_H