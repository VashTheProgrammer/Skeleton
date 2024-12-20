#ifndef FLASH_H
#define FLASH_H

#include <stddef.h>
#include <stdint.h>

#include <string.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

// Reserved flash sector capacity
#define FLASH_CAPACITY_BYTES (FLASH_SECTOR_SIZE)

// Offset for application data storage
#define FLASH_TARGET_OFFSET (256 * 1024)

// Writes data to flash storage
int flash_storage_write(const void *data, size_t size);

// Reads data from flash storage
int flash_storage_read(void *buffer, size_t size);

// Retrieves the available free space in flash storage
size_t flash_storage_get_free_space();

#endif // FLASH_H
