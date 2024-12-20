#include "flash.h"
#include "config.h"

// Writes data to the flash storage
int flash_storage_write(const void *data, size_t size) {
    uint32_t ints = save_and_disable_interrupts(); // Disable interrupts to safely access flash
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE); // Erase target flash range
    flash_range_program(FLASH_TARGET_OFFSET, data, size); // Write data to flash
    restore_interrupts(ints); // Restore interrupts
    return 0;
}

// Reads data from the flash storage
int flash_storage_read(void *buffer, size_t size) {
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET); // Flash address base
    memcpy(buffer, flash_data, size); // Copy data from flash to buffer
    return 0;
}

// Calculates the remaining free space in flash storage
size_t flash_storage_get_free_space() {
    size_t used_space = sizeof(config_param_t) * MAX_PARAMS; // Calculate used space
    return FLASH_CAPACITY_BYTES > used_space ? FLASH_CAPACITY_BYTES - used_space : 0; // Return free space
}
