#include "flash.h"
#include "config.h"

int flash_storage_write(const void *data, size_t size) {
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, data, size);
    restore_interrupts(ints);
    return 0;
}

int flash_storage_read(void *buffer, size_t size) {
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(buffer, flash_data, size);
    return 0;
}

size_t flash_storage_get_free_space() {
    size_t used_space = sizeof(config_param_t) * MAX_PARAMS;
    return FLASH_CAPACITY_BYTES > used_space ? FLASH_CAPACITY_BYTES - used_space : 0;
}