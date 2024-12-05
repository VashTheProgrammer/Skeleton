#include "task_led.h"
#include "circular_buffer.h"
#include "driver_led.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "initcalls.h"

#include "hardware_config.h"

#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

#define BUFFER_SIZE_CB 2048 // Dimensione del buffer circolare

static circular_buffer_t queue_example;

// Buffer circolare
uint8_t buffer[BUFFER_SIZE_CB];
volatile int head = 0; // Indice della testa del buffer
volatile int tail = 0; // Indice della coda del buffer

// Funzione per aggiungere un dato al buffer
void buffer_put(uint8_t data) {
    int next_head = (head + 1) % BUFFER_SIZE_CB;
    if (next_head != tail) { // Verifica che il buffer non sia pieno
        buffer[head] = data;
        head = next_head;
    }
}

// Funzione per leggere un dato dal buffer
int buffer_get(uint8_t *data) {
    if (head == tail) {
        return 0; // Buffer vuoto
    }
    *data = buffer[tail];
    tail = (tail + 1) % BUFFER_SIZE_CB;
    return 1; // Dato letto con successo
}

// Gestore di interrupt per UART1
void on_uart1_rx() {
    while (uart_is_readable(uart1)) { 
        uint8_t data = uart_getc(uart1); 
        buffer_put(data); // Aggiungi il dato al buffer 
    }
}

void task_serial_bridge_init(void) {
    uart_init(uart0, hw_config->uart0_baud);
    gpio_set_function(hw_config->uart0_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(hw_config->uart0_rx_pin, GPIO_FUNC_UART);
    uart_set_format(uart0, DATA_BITS, STOP_BITS, PARITY);
    uart_set_hw_flow(uart0, true, true);

    uart_init(uart1, hw_config->uart1_baud);
    gpio_set_function(hw_config->uart1_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(hw_config->uart1_rx_pin, GPIO_FUNC_UART);
    uart_set_format(uart1, DATA_BITS, STOP_BITS, PARITY);
    gpio_set_function(hw_config->uart1_rts_pin, GPIO_FUNC_UART);
    gpio_set_function(hw_config->uart1_cts_pin, GPIO_FUNC_UART);
    uart_set_hw_flow(uart1, true, true);

    circular_buffer_init(&queue_example);

    // Configura l'interrupt per UART1
    irq_set_exclusive_handler(UART1_IRQ, on_uart1_rx);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(uart1, true, false); // Abilita interrupt per RX, disabilita per TX
}
REGISTER_INITCALL(task_serial_bridge_init);

void task_serial_bridge(void){

    data_packet_t data;
    
    // Se riceviamo dei dati li mettiamo in coda per altri task
    circular_buffer_enqueue(&queue_example, data);
    
    // Leggi dal buffer e invia alla UART0
    if (uart_is_writable(uart0)) {
        uint8_t data;
        if (head != tail) {
            data = buffer[tail];
            tail = (tail + 1) % BUFFER_SIZE_CB;
            uart_putc(uart0, data);
        }
    }

    // Se c'è un dato disponibile sulla UART0 (PC), invialo alla UART1 (ESP32)
    if (uart_is_readable(uart0)) {
        uint8_t data = uart_getc(uart0);
        if (uart_is_writable(uart1)) { // Controlla CTS prima di scrivere
            uart_putc(uart1, data);
        }
    }
}