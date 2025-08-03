#include "IO.h"

/**
 * @brief Writes logical values to all GPIO pins defined in the structure.
 *
 * @param io Pointer to the IO_t structure containing GPIO definitions.
 */
void IO_WriteAllFn(IO_t *io) {
    for (uint8_t i = 0; i < io->element_count; i++) {
        HAL_GPIO_WritePin(
            io->list[i].portpin.port,
            io->list[i].portpin.pin,
            io->list[i].value ? GPIO_PIN_SET : GPIO_PIN_RESET
        );
    }
}

/**
 * @brief Reads the current logical values from all GPIO pins defined in the structure.
 *
 * @param io Pointer to the IO_t structure containing GPIO definitions.
 */
void IO_GetAllFn(IO_t *io) {
    for (uint8_t i = 0; i < io->element_count; i++) {
        io->list[i].value = HAL_GPIO_ReadPin(
            io->list[i].portpin.port,
            io->list[i].portpin.pin
        );
    }
}
