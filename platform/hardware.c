#include "hardware_cfg.h"

// Hardware configuration for version V1
const HardwareConfig hardware_v1 = {
    .uart1_rts_pin = 3, // RTS pin for UART1
    .uart1_cts_pin = 2, // CTS pin for UART1
    .uart1_tx_pin = 4,  // TX pin for UART1
    .uart1_rx_pin = 5,  // RX pin for UART1
    .uart1_baud = 5000000, // Baud rate for UART1

    .uart0_rts_pin = -1,
    .uart0_cts_pin = -1,
    .uart0_tx_pin = 0,  // TX pin for UART0
    .uart0_rx_pin = 1,  // RX pin for UART0
    .uart0_baud = 115200, // Baud rate for UART0

    .led_pin = 25, // LED GPIO pin
    .esp32_rst_pin = 6, // ESP32 reset pin
    .extra_gpio1 = 26 // Additional GPIO pin
};

// Hardware configuration for version V2
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
    .extra_gpio1 = -1
};

// Pointer to the current hardware configuration
const HardwareConfig* hw_config;

// Selects the appropriate hardware configuration based on predefined macros
void select_hardware_config(void) {
    #ifdef HARDWARE_DEFAULT
    hw_config = &hardware_v1;
    #elif defined(HARDWARE_V1)
    hw_config = &hardware_v2;
    #else
    #warning "No hardware version defined!" // Compilation warning
    #endif
}

/*
 * Note: The specific hardware configuration is selected at the CMake level
 * by defining macros (e.g., HARDWARE_DEFAULT, HARDWARE_V1). Ensure that the
 * correct macro is defined in the build system to avoid runtime issues.
 */
