#include <stdio.h>
#include "string.h"

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/regs/addressmap.h"

/*
uint32_t *pico_flash_read(long int FLASH_TARGET_OFFSET, size_t len) 
{
    const uint32_t *flash_target_contents = (const uint32_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    uint32_t *stored_value = (uint32_t*)calloc(len, sizeof(uint32_t));

    for (size_t i = 0; i < len; ++i)
    {
        printf("\n STORED VALUE : %d\n", flash_target_contents[i]);
        stored_value[i] = flash_target_contents[i];
    }

    return stored_value;                                                                                //++ Returns the pointer pointing to the array
}

void pico_flash_erase(long int FLASH_TARGET_OFFSET)
{
    uint8_t *stored_value = 0;
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);

    printf("\n ERASING FLASH MEMORY......\n");

    flash_range_program(FLASH_TARGET_OFFSET, stored_value, FLASH_PAGE_SIZE);
    restore_interrupts(interrupts);   
}

void pico_flash_write(long int FLASH_TARGET_OFFSET, uint8_t flash_data[], size_t num)
{
    const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    uint32_t *stored_value = (uint32_t*)malloc(num + 1);

    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);

    printf("\n PROGRAMMING FLASH MEMORY......\n");

    flash_range_program(FLASH_TARGET_OFFSET, flash_data, FLASH_PAGE_SIZE);
    restore_interrupts(interrupts);
}

void pico_flash_write2(uint32_t flash_offset, const uint8_t flash_data[], size_t num) {
    // Check that num is exactly FLASH_PAGE_SIZE for this simple example:
    if (num != FLASH_PAGE_SIZE) {
        printf("Error: num must be %d bytes\n", FLASH_PAGE_SIZE);
        return;
    }

    // Disable interrupts for flash programming operations
    uint32_t interrupts = save_and_disable_interrupts();

    // Erase the entire sector at flash_offset
    flash_range_erase(flash_offset, FLASH_SECTOR_SIZE);
    printf("\n PROGRAMMING FLASH MEMORY......\n");

    // Write one page (256 bytes) from flash_data into flash at flash_offset
    flash_range_program(flash_offset, flash_data, num);

    // Re-enable interrupts
    restore_interrupts(interrupts);

}



Writes to flash
Disable interupts
gets FLASH_PAGE_SIZE data and adress

void flash_writer_process(uint32_t addr, const uint8_t *data[]) {


    uint32_t interrupts = save_and_disable_interrupts();

    flash_range_program(addr, (uint8_t *)data, FLASH_PAGE_SIZE);

    restore_interrupts(interrupts);

    
        
    int buf[FLASH_PAGE_SIZE/sizeof(int)];  // One page worth of 32-bit ints
    int mydata = 123456;  // The data I want to store

    buf[0] = mydata;  // Put the data into the first four bytes of buf[]

    // Erase the last sector of the flash
        

        //flash_range_erase((PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE), FLASH_SECTOR_SIZE);

    // Program buf[] into the first page of this sector
    ///flash_range_program((PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE), (uint8_t *)data, FLASH_PAGE_SIZE);

}
*/



/*
reads last sector where should be saved absolute position where data was saved last time (leveling purpouses)
checks if sector is already set all to 1, if not erase the sector (lower wear)
set flash_write buffer size multiple of 256 (check it)
rewrites to last sector begining adress, where is overflow begin

last sector frame: 32 bit beginig, 32 bit overflow begin, 192 bit empty (must write in 1 page chunks), 32 bit end

*/

#define MAX_BUFFER_SIZE 10 * FLASH_PAGE_SIZE // max 2.560kB
extern char __flash_binary_end;
uint32_t last_sector_begin;
//uint8_t buffer[96];
uint8_t *buffer = NULL;
uint32_t position = 0;
uint32_t flash_addr;
uint32_t flash_overflow_begin;

/*
* Function: is_flash_erased
* --------------------------------
* Purpose: Checks if a given block of flash memory is completely erased.
* The function iterates through each byte in the provided memory block (of
* given size) and returns true only if every byte equals 0xFF, which is
* the typical "erased" state for flash memory.
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
* Searches the last flash sector (of size FLASH_SECTOR_SIZE) for the first
* pair of pages (pages are FLASH_PAGE_SIZE in size; 2 pages are reserved for
* data) that are erased (i.e. all bytes are 0xFF). This "leveling" approach
* is intended to decrease wear by spreading writes over multiple pages.
*
* If an erased page pair is found, the function returns the absolute beginning
* offset (excluding XIP_BASE) of the first page where new data can be written.
* If no erased page pair is found, the entire sector is erased and the function
* returns the beginning offset of the sector.
*/
uint32_t last_sector_leveling(bool erase_all){

    // Loop over the flash pages in the sector.
    // The loop iterates in steps of 2 pages, leaving space for 2 pages at a time.
    for (uint8_t i = 0; i <= FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE-2; i+=2) {

        printf("leveling_i: %d\n", i);

        const uint8_t *addr = (const uint8_t *)(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE + XIP_BASE + (i * FLASH_PAGE_SIZE));

         // Check if the current flash page is completely erased (i.e. all 0xFF)
        if (is_flash_erased(addr, FLASH_PAGE_SIZE)) {

            // Return the absolute address (XIP_BASE excluded) at which new data can be written
            return (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE + (i * FLASH_PAGE_SIZE)); 

        }
    }

    //if(erase_all){
        // If no erased pages were found in the loop, all pages are considered full. 
        // Erase the flash sector.
        printf("All sectors are full, erasing\n");
        uint32_t interrupts = save_and_disable_interrupts();
        __not_in_flash_func(flash_range_erase(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE));
        restore_interrupts(interrupts); 

        // Return the beginning of the flash sector (with XIP_BASE excluded)
        return (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE);
        /*
    }
    else{
        // return 0 as there is no more space left to create new header for new file
        return 0;
    }
        */


}

uint32_t read_flash(uint32_t position) {
    volatile uint32_t *flash_ptr = (volatile uint32_t *)(position + XIP_BASE);
    return *flash_ptr;
}

uint32_t greatest_common_divisor(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}


/*
 * Function: flash_init
 * --------------------
 * Initializes flash storage by:
 *   - Reading a flash value from a fixed location.
 *   - Determining the next available write address using a leveling
 *     strategy that reduces wear by alternating between page pairs.
 *   - Erasing full flash sectors in a specified range.
 *   - Writing control data (flash_addr and flash_overflow_begin) to the
 *     last sector.
 * Argument
 * --------------------
 * input expected frame
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

    uint32_t buffer_size = FLASH_PAGE_SIZE * (total/(greatest_common_divisor(FLASH_PAGE_SIZE, total)));
    if (buffer_size > MAX_BUFFER_SIZE){
        buffer_size = MAX_BUFFER_SIZE;
    }
    printf("buffer_size: %d\n", buffer_size);

    
    buffer = (uint8_t *)malloc(buffer_size);
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return UINT32_MAX;
    }
        

    // Read a flash word from a fixed location (2 pages before the end), important for last sector erasing. If first init then random value should be ok
    flash_addr = read_flash(PICO_FLASH_SIZE_BYTES - 2* FLASH_PAGE_SIZE);

    //Get the beginning offset (excluding XIP_BASE) of the last flash sector
    //If there is no more space in header sector and erase all is disabled then will return 0 as there is no free space to write new file
    last_sector_begin = last_sector_leveling(erase_all);
    printf("last_sector_begin 0x%x\n", last_sector_begin);
    if (last_sector_begin == 0){
        printf("There is no free space\n");
        return 0;
    }
    printf("last_sector_begin 0x%x\n", last_sector_begin);

    // If the available space calculated does not leave at least 2 pages
    // before the sector's beginning relative to the whole flash size,
    // then re-read flash_addr from a lower position.

    if (!((last_sector_begin - 2* FLASH_PAGE_SIZE)<(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE))){
        flash_addr = read_flash(last_sector_begin - 2* FLASH_PAGE_SIZE);
    }

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



void flash_write(uint32_t id, uint8_t data[8], bool write_all) {

    // Copy values into the buffer
    memcpy(buffer + position, &id, sizeof(id));
    position += sizeof(id);
    memcpy(buffer + position, data, 8);
    position += 8;

    // Force folowing condition to be true, rest of array should be 0
    if (write_all == true){
        memset(buffer + position, 0, sizeof(buffer) - position);
        position = sizeof(buffer);
    }

    // Write buffer to flash
    if (position == sizeof(buffer)){
        // needs to be done 3 times
        for(uint8_t i = 0; i<3; i++){
            const uint8_t* data_ptr = &buffer[i * 32];
            printf("Writing to adr: 0x%08X\n", (unsigned int)flash_addr);
            //__not_in_flash_func(flash_writer_process(flash_addr, &data_ptr));

            // check for memory overflow
            if (flash_addr + 0x100 >= PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE){
                flash_addr = flash_overflow_begin;
            }
            else {
                flash_addr += 0x100;
            }
        }

        //Clear buffer
        memset(buffer, 0, sizeof(buffer));
        position = 0;
    }
    
}


/*
writes rest of flash_write buffer to flash
adds end position to last sector
*/
/*
void flash_end(){

    flash_write(0, {0}, true); // set to 0 for recognizable end

    uint8_t buffer[32];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &flash_addr, sizeof(flash_addr));

    const uint8_t *data_ptr = buffer;
    __not_in_flash_func(flash_writer_process(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE + 256, &data_ptr));
}
*/




