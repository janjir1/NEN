#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "mcp2515/mcp2515.h"

// SPI Defines
#define SPI_PORT spi0
#define SPI_BAUD 10000000 // 10MHz
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19



// MCP2515 defines
MCP2515 can0(spi0, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUD); 
struct can_frame rx;




int main()
{
    stdio_init_all();

    //Initialize CAN0
    can0.reset();
    can0.setBitrate(CAN_1000KBPS, MCP_8MHZ);
    can0.setListenOnlyMode();


    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
