#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "driver_led.h"

// Hardware Abstraction Layer (HAL) for LED control on Raspberry Pi Pico

// Initializes the LED driver with the specified GPIO pin
void led_init(DriverLed *driver, int gpio_pin) {
    driver->led_pin = gpio_pin;
    driver->led_state = LED_OFF;
    driver->fade_in_progress = false;

    gpio_set_function(driver->led_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(driver->led_pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, 255); // 8-bit resolution
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(driver->led_pin, 0); // Start with LED off
    pwm_set_enabled(slice_num, true);
}

// Turns the LED on (maximum brightness)
void led_on(DriverLed *driver) {
    uint slice_num = pwm_gpio_to_slice_num(driver->led_pin);
    pwm_set_gpio_level(driver->led_pin, 255);
    driver->led_state = LED_ON;
}

// Turns the LED off (minimum brightness)
void led_off(DriverLed *driver) {
    uint slice_num = pwm_gpio_to_slice_num(driver->led_pin);
    pwm_set_gpio_level(driver->led_pin, 0);
    driver->led_state = LED_OFF;
}

// Toggles the LED state between on and off
void led_toggle(DriverLed *driver) {
    if (driver->led_state == LED_ON) {
        led_off(driver);
    } else {
        led_on(driver);
    }
}

// Initiates a fade-in effect for the LED over the specified duration
void led_fade_in(DriverLed *driver, uint32_t duration_ms) {
    driver->fade_in_progress = true;
    driver->fade_target = 255;
    driver->fade_level = 0;
    driver->fade_step_time = duration_ms / 255;
    driver->fade_timer = make_timeout_time_ms(driver->fade_step_time);
}

// Initiates a fade-out effect for the LED over the specified duration
void led_fade_out(DriverLed *driver, uint32_t duration_ms) {
    driver->fade_in_progress = true;
    driver->fade_target = 0;
    driver->fade_level = 255;
    driver->fade_step_time = duration_ms / 255;
    driver->fade_timer = make_timeout_time_ms(driver->fade_step_time);
}

// Processes ongoing fade effects, adjusting LED brightness step by step
void led_process_fade(DriverLed *driver) {
    if (driver->fade_in_progress && time_reached(driver->fade_timer)) {
        if (driver->fade_level == driver->fade_target) {
            driver->fade_in_progress = false;
        } else {
            driver->fade_level += (driver->fade_target > driver->fade_level) ? 1 : -1;
            uint slice_num = pwm_gpio_to_slice_num(driver->led_pin);
            pwm_set_gpio_level(driver->led_pin, driver->fade_level);
            driver->fade_timer = make_timeout_time_ms(driver->fade_step_time);
        }
    }
}

// Initializes and binds LED driver functions
void initialize_driver_led(DriverLed *driver, int gpio_pin) {
    driver->init = led_init;
    driver->on = led_on;
    driver->off = led_off;
    driver->toggle = led_toggle;
    driver->fade_in = led_fade_in;
    driver->fade_out = led_fade_out;
    driver->process_fade = led_process_fade;

    driver->init(driver, gpio_pin);
}
