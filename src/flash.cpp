#include <stdio.h>
#include "string.h"

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/regs/addressmap.h"
#include "malloc.h"
#include "src/flash.hpp"

#define MAX_BUFFER_SIZE 10 * FLASH_PAGE_SIZE // max 2.560kB
extern char __flash_binary_end;
uint32_t last_sector_begin;
uint8_t *buffer = NULL;
uint32_t buffer_size = 0;
uint32_t position = 0;
uint32_t flash_addr;
uint32_t flash_overflow_begin;

/*
 * Function: is_flash_erased
 * -------------------------
 * Purpose:
 *   Checks if a given block of flash memory is completely erased.
 *
 * Description:
 *   This function iterates through each byte in the provided memory block (of the specified size)
 *   and verifies whether every byte is in the erased state (0xFF). In flash memory, a byte in the erased
 *   state typically has all bits set (0xFF). If any byte deviates from 0xFF, the function returns false,
 *   indicating that part of the flash is programmed. Otherwise, it returns true.
 *
 * Arguments:
 *   memory_block - Pointer to the first byte of the flash memory block to be checked.
 *   size         - The number of bytes to check within the flash memory block.
 *
 * Return Value:
 *   Returns true if all bytes in the specified memory block are 0xFF (erased state).
 *   Returns false if any byte is not 0xFF, indicating that the flash block is not fully erased.
 */
bool is_flash_erased(const uint8_t *memory_block, uint32_t size) {

    for (uint32_t i = 0; i < size; ++i) {

        if (memory_block[i] != 0xFF) {
            return false; 
        }
    }
    return true;
}

/*
 * Function: last_sector_leveling
 * --------------------------------
 * Purpose:
 *   Finds the first available pair of erased flash pages in the last sector 
 *   (sector size = FLASH_SECTOR_SIZE, page size = FLASH_PAGE_SIZE) to reduce wear.
 *
 * Behavior:
 *   - Iterates over pages in pairs; if an erased page is found, returns its flash offset 
 *     (XIP_BASE excluded).
 *   - If no erased pages are found and erase_all is true, erases the sector and returns its offset.
 *   - If erase_all is false, returns 0 to indicate no free space.
 *
 * Arguments:
 *   erase_all - When true, forces an erase of the sector if no free page pair is found.
 *
 * Return Value:
 *   The flash offset (excluding XIP_BASE) where new data can be written, or 0 if no space is available.
 */
uint32_t last_sector_leveling(bool erase_all){

    // Loop over the flash pages in the sector.
    // The loop iterates in steps of 1 page
    for (uint8_t i = 0; i <= FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE-1; i+=1) {

        printf("leveling_i: %d\n", i);

        const uint8_t *addr = (const uint8_t *)(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE + XIP_BASE + (i * FLASH_PAGE_SIZE));

         // Check if the current flash page is completely erased (i.e. all 0xFF)
        if (is_flash_erased(addr, FLASH_PAGE_SIZE)) {

            // Return the absolute address (XIP_BASE excluded) at which new data can be written
            return (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE + (i * FLASH_PAGE_SIZE)); 

        }
    }

    if(erase_all){
        // If no erased pages were found in the loop, all pages are considered full. 
        // Erase the flash sector.
        printf("All sectors are full, erasing\n");
        uint32_t interrupts = save_and_disable_interrupts();
        __not_in_flash_func(flash_range_erase(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE));
        restore_interrupts(interrupts); 

        // Return the beginning of the flash sector (with XIP_BASE excluded)
        return (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE);

    }
    else{
        // return 0 as there is no more space left to create new header for new file
        return 0;
    }

}

/*
 * Function: read_flash
 * --------------------
 * Reads a 32-bit word from flash memory at the specified offset.
 *
 * Arguments:
 *   position - Flash memory offset (excluding XIP_BASE).
 *
 * Returns:
 *   32-bit value read from flash.
 */
uint32_t read_flash(uint32_t position) {
    volatile uint32_t *flash_ptr = (volatile uint32_t *)(position + XIP_BASE);
    return *flash_ptr;
}

/*
 * Function: greatest_common_divisor
 * ---------------------------------
 * Computes the greatest common divisor (GCD) of two non-negative integers
 * using the Euclidean algorithm.
 *
 * Arguments:
 *   a, b - Non-negative integers.
 *
 * Returns:
 *   The greatest common divisor of a and b.
 */
uint32_t greatest_common_divisor(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

uint32_t find_last_EOF(uint32_t start){

    // Loop over the flash pages in the sector.
    // The loop iterates in steps of 1 page
    for (uint32_t i = start; i <= PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE; i+=FLASH_SECTOR_SIZE) {

        printf("finding start: 0x%x\n", i);

        const uint8_t *addr = (const uint8_t *)(i + XIP_BASE);

         // Check if the current flash page is completely erased (i.e. all 0xFF)
        if (is_flash_erased(addr, FLASH_PAGE_SIZE)) {

            // Return the absolute address (XIP_BASE excluded) at which new data can be written
            return i; 

        }
    }

    printf("Not found until end of flash\n");

    for (uint32_t i = 0; i <= start; i+=FLASH_SECTOR_SIZE) {

        printf("finding start: 0x%x\n", i);

        const uint8_t *addr = (const uint8_t *)(i + XIP_BASE);

         // Check if the current flash page is completely erased (i.e. all 0xFF)
        if (is_flash_erased(addr, FLASH_PAGE_SIZE)) {

            // Return the absolute address (XIP_BASE excluded) at which new data can be written
            return i; 

        }
    }

    return UINT32_MAX;

}


/*
 * Function: flash_init
 * --------------------
 * Initializes flash storage by performing the following steps:
 *   1. Reads a flash word from a fixed location (2 pages before the end) to acquire control data,
 *      which is used for flash management.
 *   2. Sums the sizes specified in the input array and computes a buffer size (in multiples of FLASH_PAGE_SIZE)
 *      that is perfectly divisible by the total of these sizes. The computed buffer size is capped by MAX_BUFFER_SIZE.
 *   3. Allocates a buffer of the computed size dynamically.
 *   4. Determines the next available write address using a wear-leveling strategy which alternates between page pairs.
 *      If there is no free space, the function returns 0.
 *   5. Computes flash overflow begin address based on the firmware's end and reserves extra space.
 *   6. Adjusts the flash write address (flash_addr) to ensure data is not written into program memory or outside
 *      the available flash region.
 *   7. If the 'erase_all' flag is true, erases full flash sectors between flash_overflow_begin and the last available sector.
 *   8. Counts the amount of free (erased) flash space in the designated region and prints it.
 *   9. Constructs a header in an init_buffer that contains:
 *         - The current flash write address (flash_addr)
 *         - The flash overflow begin address (flash_overflow_begin)
 *         - The list of sizes provided in the input array
 *      This header is then written into the last flash sector.
 *
 * Arguments:
 *   sizes:    An array of uint32_t values representing expected frame sizes (or parameters) that will be incorporated
 *             into the flash header.
 *   count:    The number of elements in the 'sizes' array.
 *   erase_all:Boolean flag indicating whether all sectors in the designated range should be erased.
 *
 * Return value:
 *   Returns the total amount of free flash space available (in bytes) if successful.
 *   In case of an error (such as memory allocation failure or excessive parameters), returns UINT32_MAX or 0 as appropriate.
 */
uint32_t flash_init(uint32_t sizes[], uint8_t count, bool erase_all) {

    if (count*sizeof(uint32_t) >= FLASH_PAGE_SIZE - (2*sizeof(uint32_t))){
        printf("This will not work too many parameters\n");
        return UINT32_MAX;
    }

    uint8_t total = 0;
    for (uint8_t i = 0; i < count; ++i) {
        total += sizes[i];
    }

    buffer_size = FLASH_PAGE_SIZE * (total/(greatest_common_divisor(FLASH_PAGE_SIZE, total)));
    if (buffer_size > MAX_BUFFER_SIZE){
        buffer_size = MAX_BUFFER_SIZE;
    }
    printf("buffer_size: %d\n", buffer_size);

    
    buffer = (uint8_t *)malloc(buffer_size);
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return UINT32_MAX;
    }

    //Get the beginning offset (excluding XIP_BASE) of the last flash sector
    //If there is no more space in header sector and erase all is disabled then will return 0 as there is no free space to write new file
    last_sector_begin = last_sector_leveling(erase_all);
    if (last_sector_begin == 0){
        printf("There is no free space\n");
        return 0;
    }
    printf("last_sector_begin 0x%x\n", last_sector_begin);

    // If the available space calculated does not leave at least 2 pages
    // before the sector's beginning relative to the whole flash size,
    // then re-read flash_addr from a lower position.

    uint32_t start = read_flash(last_sector_begin - FLASH_PAGE_SIZE);
    printf("looking for free space begin 0x%x\n", last_sector_begin);
    flash_addr = find_last_EOF(start);


    printf("Flash begin: 0x%08X\n", flash_addr);
 

    // Determine where overflow (additional flash storage) begins.
    // __flash_binary_end marks the end of the firmware binary in flash.
    // Rounding up to the next 4KB boundary and adding 5 sectors gives some extra space (random number).

    uint32_t program_end = (uintptr_t)&__flash_binary_end - XIP_BASE;
    flash_overflow_begin = (program_end + 4095) & ~4095;
    flash_overflow_begin += 5*4096;

    printf("Overflow begin: 0x%08X\n", flash_overflow_begin);

    // Adjust flash_addr if it would cause data to be written into program memory or outside available flash.
    if (flash_addr < flash_overflow_begin){
        printf("Data would be saved to program memory -> setting beginig to: 0x%08X\n", flash_overflow_begin);
        flash_addr = flash_overflow_begin;
        printf("fash_addr: 0x%08X\n", flash_addr);
    }
    else if (flash_addr > (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)){
        printf("Data would be saved outside of avalible flash -> setting beginig to: 0x%08X\n", flash_overflow_begin);
        flash_addr = flash_overflow_begin;
    }

    if (erase_all){
    // Erase full sectors in the region between flash_overflow_begin and the last available sector
        for (uint32_t pos = flash_overflow_begin; pos < (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE); pos += FLASH_SECTOR_SIZE) {
        
            const uint8_t *sector_addr = (const uint8_t *)(pos+XIP_BASE);
            if (!is_flash_erased(sector_addr, FLASH_SECTOR_SIZE)) {
                
                printf("Erasing at adr: 0x%08X\n", pos);
                uint32_t interrupts = save_and_disable_interrupts();
                __not_in_flash_func(flash_range_erase(pos, FLASH_SECTOR_SIZE));
                restore_interrupts(interrupts); 

            }                   
        }

        printf("Memory erased\n");
    }

    

    //Count avalible space
    uint32_t space_avalible = 0;

    for (uint32_t pos = flash_overflow_begin; pos < (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE); pos += FLASH_SECTOR_SIZE) {
        
        const uint8_t *sector_addr = (const uint8_t *)(pos+XIP_BASE);
        if (is_flash_erased(sector_addr, FLASH_SECTOR_SIZE)) {
            
            space_avalible += FLASH_SECTOR_SIZE;

        }                   
    }
    
    printf("There is %d kB of free space\n", space_avalible/1000);
    
    
    // Write control data to the last sector for tracking purpouse.
    // Fill the buffer with 0xFF (commonly used as the erased state)
    uint8_t init_buffer[FLASH_PAGE_SIZE];
    memset(init_buffer, 0xFF, sizeof(init_buffer));
    uint32_t offset = 0;

    memcpy(init_buffer + offset, &flash_addr, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(init_buffer + offset, &flash_overflow_begin, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    for (uint8_t i = 0; i < count; ++i) {
        memcpy(init_buffer + offset, &sizes[i], sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }

    printf("Writing header to adr: 0x%x\n", last_sector_begin);

    uint32_t interrupts = save_and_disable_interrupts();
    __not_in_flash_func(flash_range_program(last_sector_begin, (uint8_t *)init_buffer, FLASH_PAGE_SIZE));
    restore_interrupts(interrupts); 

    printf("Init finished\n");

    return space_avalible;
}

/*
expecting 32bit device identifier (CAN does 11 or 29 bit), and 8byte data (recived could be less, must be padded by 0 before entering this function)
Function has a buffer of size (3*96%256 = 0 ->) 768bits, once buffer is full writes data to flash -> change to flash offset
Writing to flash starts where ended last time 
if flash offset is larger then (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE) (last sector reserved for leveling) overflow to size of current program + something (its often writen in here - leveling)
can be forced to write not fullz complete buffer to flash

*/


// how to use flash_write(&value16, sizeof(value16), false);
/*
 * Function: flash_write
 * --------------------
 * Writes data to a buffer and, when the buffer is full, programs it to flash memory.
 * The function implements a circular buffer that automatically wraps around to a
 * designated overflow area when reaching the end of the flash memory.
 *
 * Arguments:
 *   data - Pointer to the data that needs to be written to flash.
 *   data_size - Size of the data in bytes.
 *   write_all - If true, fills the rest of the buffer with zeros and forces a write
 *               operation regardless of buffer fullness.
 *
 * Returns:
 *   true if all flash write operations were successful, false if any verification failed.
 *
 * Notes:
 *   - The function uses global buffer, position, and flash_addr variables that must be
 *     initialized before calling this function.
 *   - Interrupts are disabled during the actual flash programming to ensure timing
 *     requirements are met.
 *   - Each write operation is verified by comparing the flash contents with the source data.
 */
bool flash_write(const void* data, uint8_t data_size, bool write_all) {

    if (write_all == true){
        memset(buffer + position, 0, buffer_size - position);
        position = buffer_size;
    }

    bool pass = true;

    if (position + data_size >= buffer_size){
        for(uint32_t i = 0; i<buffer_size; i+= FLASH_PAGE_SIZE){

            printf("writing data %d\n", i);

            if (flash_addr + FLASH_PAGE_SIZE >= PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE){
                flash_addr = flash_overflow_begin;
            }

            const uint8_t* data_ptr = &buffer[i];
            printf("Writing to adr: 0x%08X\n", (unsigned int)flash_addr);

            uint32_t interrupts = save_and_disable_interrupts();
            __not_in_flash_func(flash_range_program(flash_addr, data_ptr, FLASH_PAGE_SIZE));
            restore_interrupts(interrupts); 

            // check if it was written correctly
            uint8_t *flash_ptr = (uint8_t *)(XIP_BASE + flash_addr);
            if (memcmp(flash_ptr, data_ptr, FLASH_PAGE_SIZE) != 0){
                pass = false;
            }

            flash_addr += FLASH_PAGE_SIZE;

        }

        memset(buffer, 0xFF, buffer_size);
        position = 0;
        

    }

    // Copy values into the buffer
    memcpy(buffer + position, data, data_size);
    position += data_size;

    return pass;
    
}


/*
 * Function: flash_end
 * -------------------
 * Finalizes the flash writing process by forcing a final write of the buffer to flash memory.
 * This is typically used to ensure that any remaining unwritten data in the buffer is flushed
 * to flash, even if the buffer is not full.
 *
 * Arguments:
 *   None.
 *
 * Returns:
 *   true if the final flash write operation was successful, false otherwise.
 */
bool flash_end(void){

    uint8_t empty_data = 0;
    bool pass = flash_write(&empty_data, 8, true);
    free(buffer);
    
    return pass;

}
