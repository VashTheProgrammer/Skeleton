#ifndef FLASH_H
#define FLASH_H

#include <stddef.h>
#include <stdint.h>

#include <string.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#define FLASH_CAPACITY_BYTES (FLASH_SECTOR_SIZE) // Capacità totale del settore riservato
#define FLASH_TARGET_OFFSET (256 * 1024) // Offset per i dati

int flash_storage_write(const void *data, size_t size);
int flash_storage_read(void *buffer, size_t size);
size_t flash_storage_get_free_space();

#endif // FLASH_STORAGE_H
