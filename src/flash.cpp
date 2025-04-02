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
