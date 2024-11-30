#include <stdio.h>
#include "hardware/uart.h"
#include "pico/stdlib.h"

#include "command_sequence.h"
#include "config.h"
#include "scheduler.h"

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

#define UART0_BAUD_RATE 115200
#define UART1_BAUD_RATE 5000000

// CLOCK_PER_SEC in RP2040 è 100 !

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

void task_1(void){
    led_on = !led_on;
    gpio_put(LED_PIN, led_on);
}

void task_2(void){
    
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

at_command_state_t current_state = STATE_SEND_AT_READY; // primo stato valido
int at_command_counter = 0;

void task_3(void){
        process_command_sequence(command_list, &current_state, &at_command_counter);
}

int main()
{
    // Inizializza la configurazione della UART
    stdio_init_all();
    
    // Inizializza UART0 (connessione al PC)
    uart_init(uart0, UART0_BAUD_RATE);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
    
    // Inizializza UART1 (connessione all'ESP32)
    uart_init(uart1, UART1_BAUD_RATE);

    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);

    // Configura i pin per RTS e CTS
    gpio_set_function(UART1_RTS_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_CTS_PIN, GPIO_FUNC_UART);

    // Abilita il controllo di flusso hardware
    //uart_set_hw_flow(uart1, true, true); // RTS, CTS

    uart_set_hw_flow(uart0, true, true);
    uart_set_hw_flow(uart1, true, true);

    // Set our data format
    uart_set_format(uart1, DATA_BITS, STOP_BITS, PARITY);
    uart_set_format(uart0, DATA_BITS, STOP_BITS, PARITY);

    // Configura l'interrupt per UART1
    irq_set_exclusive_handler(UART1_IRQ, on_uart1_rx);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(uart1, true, false); // Abilita interrupt per RX, disabilita per TX

    // Inizializza il pin del LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);    

    // Initialize command sequence
    initialize_command_sequence(&command_list, &iot_config);

    // Aggiungi alcuni task allo scheduler
    scheduler_add_task(task_1, 2, 50);       // Priorità 2, intervallo 50 ticks -> 500 millisec
    scheduler_add_task(task_2, 1, 0.1);      // Priorità 1, intervallo 0.1 ticks -> 1 millisec
    scheduler_add_task(task_3, 3, 500);      // Priorità 3, intervallo 500 ticks -> 5000 millisec

    // Avvia lo scheduler (loop)
    scheduler_run();

    return 0;
}
