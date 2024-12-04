#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// Definizione della struttura con i parametri hardware
typedef struct {
    int uart1_rts_pin;
    int uart1_cts_pin;
    int uart1_tx_pin;
    int uart1_rx_pin;
    int uart1_baud;

    int uart0_rts_pin;
    int uart0_cts_pin;
    int uart0_tx_pin;
    int uart0_rx_pin;
    int uart0_baud;

    int led_pin;
    int esp32_rst_pin; 
    
    int extra_gpio1;
} HardwareConfig;

// Dichiarazione delle configurazioni hardware
extern const HardwareConfig hardware_v1;
extern const HardwareConfig hardware_v2;

// Puntatore alla configurazione hardware attuale
extern const HardwareConfig* hw_config;

// Funzione per selezionare la configurazione hardware
void select_hardware_config(void);

#endif // HARDWARE_CONFIG_H