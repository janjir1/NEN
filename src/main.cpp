#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "mcp2515/mcp2515.h"  // Requires MCP2515 CAN controller library

#define SPI_PORT spi0
#define SPI_BAUDRATE 10000000  // 10 MHz recommended for MCP2515

MCP2515::ERROR err;

// GPIO Definitions
#define PIN_MISO  0
#define PIN_CS    1
#define PIN_SCK   2
#define PIN_MOSI  3

struct can_frame tx;
struct can_frame rx;

//MCP2515 can0;  // CAN controller instance/
MCP2515 can0(spi0, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUDRATE); 

uint8_t read_canstat() {
    uint8_t rx_data[3] = {0};  // Receive buffer
    uint8_t tx_buf[] = {0x03, 0x0E, 0x00}; // READ cmd + reg address + dummy byte

    gpio_put(PIN_CS, 0);
    spi_write_read_blocking(SPI_PORT, tx_buf, rx_data, sizeof(tx_buf));
    gpio_put(PIN_CS, 1);
    
    return rx_data[2]; // Received data is in third byte
}

void spi_init_device() {
    // Initialize SPI at 10 MHz
    spi_init(SPI_PORT, SPI_BAUDRATE);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Configure GPIOs
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
}

int main() {
    stdio_init_all();
    spi_init_device();
	//sleep_ms(10000);

	tx.can_id = 0x123;
	tx.can_dlc = 1;
	tx.data[0] = 0xDE;

	while(true){
    // Initialize MCP2515 CAN controller
		can0.reset();
		can0.setConfigMode();
		printf("CANSTAT config: 0x%02X\n", read_canstat());
		//printf("CANSTAT config: 0x%02X\n", can0.readRegister(MCP2515::MCP_RXB0CTRL));
		printf("END\n");

		err = can0.reset();
		if(err != MCP2515::ERROR_OK) {
			printf("Reset error: %d\n", err);
		}
		
		// Set config mode
		err = can0.setConfigMode();
		if(err != MCP2515::ERROR_OK) {
			printf("Config mode error: %d\n", err);
		}
		
		// Verify CANSTAT (bit 7-5 = 100 for config mode)
		uint8_t canstat = can0.readRegister(MCP2515::MCP_CANSTAT);
		if((canstat & 0xE0) != 0x80) {
			printf("Not in config mode! CANSTAT=0x%02X\n", canstat);
		}
		
		// Set bitrate (critical for internal timing)
		err = can0.setBitrate(CAN_500KBPS, MCP_8MHZ);
		if(err != MCP2515::ERROR_OK) {
			printf("Bitrate error: %d\n", err);
		}
		
		
		// Set loopback mode
		err = can0.setLoopbackMode();
		if(err != MCP2515::ERROR_OK) {
			printf("Loopback mode error: %d\n", err);
		}
		else{
			printf("Loopback mode success: %d\n", err);
		}

		sleep_ms(5000);
	}
}