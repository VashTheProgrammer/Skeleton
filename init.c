#include "initcalls.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#define UART0_TX_PIN 0
#define UART0_RX_PIN 1

#define UART1_RTS_PIN 3 //CROSS
#define UART1_CTS_PIN 2
#define UART1_TX_PIN 4   //CROSS
#define UART1_RX_PIN 5

#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

#define LED_PIN 25

#define GPIO6_ESP32_RST 6

#define UART0_BAUD_RATE 115200
#define UART1_BAUD_RATE 5000000

// ESP32 side
// AT+UART_DEF=5000000,8,1,0,3

void init_gpio_led(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}
REGISTER_INITCALL(init_gpio_led);


void init_system(void) {
    stdio_init_all();
}
REGISTER_INITCALL(init_system);

void init_uart0(void) {
    uart_init(uart0, UART0_BAUD_RATE);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(uart0, DATA_BITS, STOP_BITS, PARITY);

    uart_set_hw_flow(uart0, true, true);
}
REGISTER_INITCALL(init_uart0);

void init_uart1(void) {
    uart_init(uart1, UART1_BAUD_RATE);
    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(uart1, DATA_BITS, STOP_BITS, PARITY);

    // Configura i pin per RTS e CTS
    gpio_set_function(UART1_RTS_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_CTS_PIN, GPIO_FUNC_UART);

    // Abilita il controllo di flusso RTS, CTS
    uart_set_hw_flow(uart1, true, true);
}
REGISTER_INITCALL(init_uart1);

void init_gpio_esp32_rst(void) {
    gpio_init(GPIO6_ESP32_RST);
    gpio_set_dir(GPIO6_ESP32_RST, GPIO_OUT);  
}
REGISTER_INITCALL(init_gpio_esp32_rst);