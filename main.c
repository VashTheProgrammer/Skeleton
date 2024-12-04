#include <stdio.h>
#include "hardware/uart.h"
#include "pico/stdlib.h"

#include "command_sequence.h"
#include "config.h"
#include "scheduler.h"
#include "initcalls.h"
#include "hardware_config.h"

#define BUFFER_SIZE 2048 // Dimensione del buffer circolare

// Buffer circolare
uint8_t buffer[BUFFER_SIZE];
volatile int head = 0; // Indice della testa del buffer
volatile int tail = 0; // Indice della coda del buffer

// Funzione per aggiungere un dato al buffer
void buffer_put(uint8_t data) {
    int next_head = (head + 1) % BUFFER_SIZE;
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
    tail = (tail + 1) % BUFFER_SIZE;
    return 1; // Dato letto con successo
}

// Gestore di interrupt per UART1
void on_uart1_rx() {
    while (uart_is_readable(uart1)) { 
        uint8_t data = uart_getc(uart1); 
        buffer_put(data); // Aggiungi il dato al buffer 
    }
}

bool led_on = false;

void task_0(void){
    led_on = !led_on;
    gpio_put(25, led_on);
}

void task_1(void){
    
    // Leggi dal buffer e invia alla UART0
    if (uart_is_writable(uart0)) {
        uint8_t data;
        if (head != tail) {
            data = buffer[tail];
            tail = (tail + 1) % BUFFER_SIZE;
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

// Set initial state
CommandNode *command_list = NULL;

at_command_state_t current_state = STATE_SEND_RST; // primo stato valido
int at_command_counter = 0;

void task_2(void){
    process_command_sequence(command_list, &current_state, &at_command_counter);
}

int main()
{

    // Seleziona la configurazione hardware attuale
    select_hardware_config();

    // Itera sull'array delle initcall ed esegui ogni funzione
    for (int i = 0; i < initcall_count; i++) {
        initcall_list[i].init_func();
    }
    
    // Configura l'interrupt per UART1
    irq_set_exclusive_handler(UART1_IRQ, on_uart1_rx);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(uart1, true, false); // Abilita interrupt per RX, disabilita per TX

    // Initialize command sequence
    initialize_command_sequence(&command_list, &iot_config);

    // Aggiungi alcuni task allo scheduler
    // (name, function, prio, usec)
    scheduler_add_task("led", task_0, 2, 500 * 1000);             // 0 
    scheduler_add_task("uart bridge", task_1, 1, 1 * 1000);       // 1
    scheduler_add_task("cmd seq", task_2, 3, 5 * 1000 * 1000);    // 2

    // Scheduler debug 
    // (task number, bool)
    set_debug_for_task(2, false);

    // Avvia lo scheduler (loop)
    scheduler_run();

    return 0;
}
