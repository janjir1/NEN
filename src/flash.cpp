#include <stdio.h>
#include "string.h"

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"


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


/*
Writes to flash
Disable interupts
gets FLASH_PAGE_SIZE data and adress
*/
void flash_writer_process(uint32_t addr, const uint8_t *data[]) {


    uint32_t interrupts = save_and_disable_interrupts();

    flash_range_program(addr, (uint8_t *)data, FLASH_PAGE_SIZE);

    restore_interrupts(interrupts);

    
        /*
    int buf[FLASH_PAGE_SIZE/sizeof(int)];  // One page worth of 32-bit ints
    int mydata = 123456;  // The data I want to store

    buf[0] = mydata;  // Put the data into the first four bytes of buf[]

    // Erase the last sector of the flash
        */

        //flash_range_erase((PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE), FLASH_SECTOR_SIZE);

    // Program buf[] into the first page of this sector
    ///flash_range_program((PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE), (uint8_t *)data, FLASH_PAGE_SIZE);

}

/*
expecting 32bit device identifier (CAN does 11 or 29 bit), and 8byte data (recived could be less, must be padded by 0 before entering this function)
Function has a buffer of size (3*96%256 = 0 ->) 768bits, once buffer is full writes data to flash -> change to flash offset
Writing to flash starts where ended last time 
if flash offset is larger then (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE) (last sector reserved for leveling) overflow to size of current program + something (its often writen in here - leveling)
can be forced to write not fullz complete buffer to flash

*/

uint8_t buffer[96];
uint16_t position = 0;
uint32_t flash_addr;
uint32_t flash_overflow_begin;

void flash_write(uint32_t id, uint8_t data[8], bool write_all) {

    // Copy values into the buffer
    memcpy(buffer + position, &id, sizeof(id));
    position += sizeof(id);
    memcpy(buffer + position, data, sizeof(data));
    position += sizeof(data);

    // Force folowing condition to be true, rest of array should be 0
    if (write_all == true) position = sizeof(buffer);

    // Write buffer to flash
    if (position = sizeof(buffer)){
        // needs to be done 3 times
        for(uint8_t i = 0; i<4; i++){

            __not_in_flash_func(flash_writer_process(flash_addr, &buffer[i*32]));

            // check for memory overflow
            if (flash_addr + 0x100 >= PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE) flash_addr = flash_overflow_begin;
            else flash_addr += 0x100;
        }

        //Clear buffer
        memset(buffer, 0, sizeof(buffer));
        position = 0;
    }
    

}
/*
reads last sector where should be saved absolute position where data was saved last time (leveling purpouses)
checks if sector is already set all to 1, if not erase the sector (lower wear)
set flash_write buffer size multiple of 256 (check it)
rewrites to last sector begining adress, where is overflow begin

last sector frame: 32 bit beginig, 32 bit overflow begin, 192 bit empty (must write in 1 page chunks), 32 bit end

*/
extern char __flash_binary_end;

bool flash_init(uint32_t buffer_size) {

    // Read where to begin
    const int *read_data = (const int *)(XIP_BASE + (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE));
    flash_addr = read_data[0];

    // Determine overflow_begin
    uint32_t program_end = (uint32_t)&__flash_binary_end;
    flash_overflow_begin = (program_end + 255) & ~255;

    // Empty full cache chunks
    for(uint32_t position = flash_overflow_begin; position < (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE); position += 4096)
        if (is_flash_sector_erased(*position) = false) flash_range_erase(*position, FLASH_SECTOR_SIZE);
    
    // write data to last sector
    uint8_t buffer[32];
    memset(buffer, 1, sizeof(buffer));
    memcpy(buffer, &flash_addr, sizeof(flash_addr));
    memcpy(buffer + sizeof(flash_addr), &flash_overflow_begin, sizeof(flash_overflow_begin));
    __not_in_flash_func(flash_writer_process(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE, &buffer[0]));


}

bool is_flash_sector_erased(const uint8_t *memory_block) {
    for (size_t i = 0; i < FLASH_SECTOR_SIZE; ++i) {
        if (memory_block[i] != 0xFF) {
            return false; 
        }
    }
    return true;
}
/*
writes rest of flash_write buffer to flash
adds end position to last sector
*/
bool flash_end(){

    flash_write(0, {0}, true); // set to 0 for recognizable end

    uint8_t buffer[32];
    memset(buffer, 1, sizeof(buffer));
    memcpy(buffer, &flash_addr, sizeof(flash_addr));
    __not_in_flash_func(flash_writer_process(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE + 256, &buffer[0]));

}



   /*
    stored_value = pico_flash_read(FLASH_TARGET_OFFSET, num);

    if (strcmp(*stored_value,flash_data)==0)                                                             //++ Compares the stored and written data 
        {
            printf("\n COMMIT SUCCESSFULL \n");
            return 0;
        }
    else
        {
            printf("\n COMMIT UNSUCCESSFULL \n");
            return 1;
        }
        */
