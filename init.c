#include "initcalls.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "hardware_config.h"

#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

void init_gpio_led(void) {
    gpio_init(hw_config->led_pin);
    gpio_set_dir(hw_config->led_pin, GPIO_OUT);
}
REGISTER_INITCALL(init_gpio_led);


void init_system(void) {
    stdio_init_all();
}
REGISTER_INITCALL(init_system);

void init_uart0(void) {
    uart_init(uart0, hw_config->uart0_baud);
    gpio_set_function(hw_config->uart0_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(hw_config->uart0_rx_pin, GPIO_FUNC_UART);
    uart_set_format(uart0, DATA_BITS, STOP_BITS, PARITY);

    uart_set_hw_flow(uart0, true, true);
}
REGISTER_INITCALL(init_uart0);

void init_uart1(void) {
    uart_init(uart1, hw_config->uart1_baud);
    gpio_set_function(hw_config->uart1_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(hw_config->uart1_rx_pin, GPIO_FUNC_UART);
    uart_set_format(uart1, DATA_BITS, STOP_BITS, PARITY);

    // Configura i pin per RTS e CTS
    gpio_set_function(hw_config->uart1_rts_pin, GPIO_FUNC_UART);
    gpio_set_function(hw_config->uart1_cts_pin, GPIO_FUNC_UART);

    // Abilita il controllo di flusso RTS, CTS
    uart_set_hw_flow(uart1, true, true);
}
REGISTER_INITCALL(init_uart1);

void init_gpio_esp32_rst(void) {
    gpio_init(hw_config->esp32_rst_pin);
    gpio_set_dir(hw_config->esp32_rst_pin, GPIO_OUT);  
}
REGISTER_INITCALL(init_gpio_esp32_rst);