#ifndef HARDWARE_CFG_H
#define HARDWARE_CFG_H

// Structure defining hardware configuration parameters
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

// Hardware configurations declarations
extern const HardwareConfig hardware_v1;
extern const HardwareConfig hardware_v2;

// Pointer to the current hardware configuration
extern const HardwareConfig* hw_config;

// Function to select the appropriate hardware configuration
void select_hardware_config(void);

/*
 * Note: The choice of which hardware configuration to use
 * is determined at the CMake level by defining specific macros
 * (e.g., HARDWARE_DEFAULT, HARDWARE_V1).
 */

#endif // HARDWARE_CFG_H
