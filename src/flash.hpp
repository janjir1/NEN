#ifndef FLASH_HPP
#define FLASH_HPP

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"

/**
 * Initializes flash storage system, allocates buffer, and sets up flash addresses.
 * 
 * @param sizes Array of expected frame sizes
 * @param count Number of elements in the sizes array
 * @param erase_all Whether to erase all sectors in the designated range
 * @return Available flash space in bytes, or UINT32_MAX/0 on error
 */
uint32_t flash_init(uint32_t sizes[], uint8_t count, bool erase_all);

/**
 * Writes data to buffer and programs to flash when buffer is full.
 * 
 * @param data Pointer to data to write
 * @param data_size Size of data in bytes
 * @param write_all If true, forces a write operation regardless of buffer fullness
 * @return true if all operations succeeded, false otherwise
 */
bool flash_write(const void* data, uint8_t data_size, bool write_all);

/**
 * Empty wtite buffer
 *
 * @return true if all operations succeeded, false otherwise
 */
bool flash_end(void);

#endif // FLASH_HPP