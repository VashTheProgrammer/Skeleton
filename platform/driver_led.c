#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "driver_led.h"

// HAL implementation for LED on Raspberry Pi Pico

void led_init(DriverLed *driver, int gpio_pin) {
    driver->led_pin = gpio_pin;
    driver->led_state = LED_OFF;
    driver->fade_in_progress = false;

    gpio_set_function(driver->led_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(driver->led_pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, 255); // Set the PWM wrap value to 255 for 8-bit resolution
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(driver->led_pin, 0); // Start with LED off
    pwm_set_enabled(slice_num, true); // Enable the PWM output
}

void led_on(DriverLed *driver) {
    uint slice_num = pwm_gpio_to_slice_num(driver->led_pin);
    pwm_set_gpio_level(driver->led_pin, 255); // Set maximum brightness
    driver->led_state = LED_ON;
}

void led_off(DriverLed *driver) {
    uint slice_num = pwm_gpio_to_slice_num(driver->led_pin);
    pwm_set_gpio_level(driver->led_pin, 0); // Set minimum brightness
    driver->led_state = LED_OFF;
}

void led_toggle(DriverLed *driver) {
    if (driver->led_state == LED_ON) {
        led_off(driver);
    } else {
        led_on(driver);
    }
}

void led_fade_in(DriverLed *driver, uint32_t duration_ms) {
    driver->fade_in_progress = true;
    driver->fade_target = 255;
    driver->fade_level = 0;
    driver->fade_step_time = duration_ms / 255;
    driver->fade_timer = make_timeout_time_ms(driver->fade_step_time);
}

void led_fade_out(DriverLed *driver, uint32_t duration_ms) {
    driver->fade_in_progress = true;
    driver->fade_target = 0;
    driver->fade_level = 255;
    driver->fade_step_time = duration_ms / 255;
    driver->fade_timer = make_timeout_time_ms(driver->fade_step_time);
}

void led_process_fade(DriverLed *driver) {
    if (driver->fade_in_progress && time_reached(driver->fade_timer)) {
        if (driver->fade_level == driver->fade_target) {
            driver->fade_in_progress = false;
        } else {
            if (driver->fade_target > driver->fade_level) {
                driver->fade_level++;
            } else {
                driver->fade_level--;
            }
            uint slice_num = pwm_gpio_to_slice_num(driver->led_pin);
            pwm_set_gpio_level(driver->led_pin, driver->fade_level);
            driver->fade_timer = make_timeout_time_ms(driver->fade_step_time);
        }
    }
}

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