#include <stdio.h>

#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

#include "hardware_cfg.h"
#include "initcalls.h"
#include "terminal.h"
#include "terminal/cmd.h"

#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

#define UART_RX_BUFFER_SIZE 128

static char uart_rx_buffer[UART_RX_BUFFER_SIZE];
static size_t uart_rx_index = 0;
static terminal_context_t terminal_context; // Global terminal context

// UART interrupt handler: Handles incoming UART data
void uart_irq_handler() {
    while (uart_is_readable(uart0)) {
        char c = uart_getc(uart0);

        // Echo received character for debugging
        uart_putc(uart0, c);

        if (c == '\n' || c == '\r') { // End of command
            uart_rx_buffer[uart_rx_index] = '\0'; // Null-terminate the string
            terminal_execute_command(&terminal_context, uart_rx_buffer); // Process the command
            uart_rx_index = 0; // Reset the buffer index
        } else if (uart_rx_index < UART_RX_BUFFER_SIZE - 1) {
            uart_rx_buffer[uart_rx_index++] = c; // Add character to buffer
        } else {
            printf("[SYSTEM][ERROR] UART0 buffer full.\n");
            uart_rx_index = 0; // Reset the buffer index
        }
    }
}

// Initializes UART and terminal functionalities
void init_task_terminal() {
    // Configure UART pins and settings
    uart_init(uart0, hw_config->uart0_baud);
    gpio_set_function(hw_config->uart0_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(hw_config->uart0_rx_pin, GPIO_FUNC_UART);
    uart_set_format(uart0, DATA_BITS, STOP_BITS, PARITY);
    uart_set_hw_flow(uart0, true, true);

    // Enable UART interrupts
    uart_set_irq_enables(uart0, true, false);
    irq_set_exclusive_handler(UART0_IRQ, uart_irq_handler);
    irq_set_enabled(UART0_IRQ, true);

    // Initialize terminal context and register commands
    terminal_init(&terminal_context);
    init_commands(&terminal_context);
}
REGISTER_INITCALL(init_task_terminal);
