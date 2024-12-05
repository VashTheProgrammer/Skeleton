#include "hardware_config.h"

// Definizione della configurazione per hardware V1
const HardwareConfig hardware_v1 = {
    .uart1_rts_pin = 3, // cross
    .uart1_cts_pin = 2,
    .uart1_tx_pin = 4,  //cross
    .uart1_rx_pin = 5,
    .uart1_baud = 5000000,
// ESP32 side
// AT+UART_DEF=5000000,8,1,0,3

    .uart0_rts_pin = -1,
    .uart0_cts_pin = -1,
    .uart0_tx_pin = 0,
    .uart0_rx_pin = 1,
    .uart0_baud = 115200,

    .led_pin = 25,
    .esp32_rst_pin = 6,
    .extra_gpio1 = 26,
};

// Definizione della configurazione per hardware V2
const HardwareConfig hardware_v2 = {
    .uart1_rts_pin = -1,
    .uart1_cts_pin = -1,
    .uart1_tx_pin = -1,
    .uart1_rx_pin = -1,
    .uart1_baud = -1,

    .uart0_rts_pin = -1,
    .uart0_cts_pin = -1,
    .uart0_tx_pin = -1,
    .uart0_rx_pin = -1,
    .uart0_baud = -1,

    .led_pin = -1,
    .esp32_rst_pin = -1,
    .extra_gpio1 = -1,
};

// Puntatore alla configurazione hardware attuale
const HardwareConfig* hw_config;

// Funzione per selezionare la configurazione hardware
void select_hardware_config(void) {
    #ifdef HARDWARE_DEFAULT
    hw_config = &hardware_v1;
    #elif defined(HARDWARE_V1)
    hw_config = &hardware_v2;
    #else
    #warning "Nessuna versione hardware definita!" // Warning in compilazione !
    #endif
}