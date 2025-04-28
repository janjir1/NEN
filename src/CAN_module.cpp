#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "mcp2515/mcp2515.h"

MCP2515* can0 = nullptr;


void spi_init_device(spi_inst_t *SPI_PORT, uint8_t PIN_CS, uint8_t PIN_MOSI, uint8_t PIN_MISO, uint8_t PIN_SCK, uint32_t SPI_BAUDRATE) {

    spi_init(SPI_PORT, SPI_BAUDRATE);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
}

// add some return if failed

bool test_mcp2515_loopback(MCP2515 &can) {
    
    MCP2515::ERROR err = can.setLoopbackMode();
    if (err != MCP2515::ERROR_OK) {
        printf("Error setting loopback mode: %d\n", err);
        return false;
    }
    
    // Create a test message
    struct can_frame txMsg;
    txMsg.can_id = 0x123;  // Test ID
    txMsg.can_dlc = 8;     // 8 bytes of data
    
    // Fill with test pattern
    for (int i = 0; i < 8; i++) {
        txMsg.data[i] = 0x10 + i;  // 0x10, 0x11, 0x12, etc.
    }
    
    // Send the message
    printf("Sending test message with ID: 0x%03X\n", txMsg.can_id);
    err = can.sendMessage(&txMsg);
    if (err != MCP2515::ERROR_OK) {
        printf("Error sending message: %d\n", err);
        return false;
    }
    
    // Wait for the message to be received
    struct can_frame rxMsg;
    uint32_t startTime = to_ms_since_boot(get_absolute_time());
    bool received = false;
    
    // Try to receive for up to 1 second
    while (to_ms_since_boot(get_absolute_time()) - startTime < 1000) {
        err = can.readMessage(&rxMsg);
        if (err == MCP2515::ERROR_OK) {
            received = true;
            break;
        }
        sleep_ms(10); // Small delay to prevent tight loop
    }
    
    if (!received) {
        printf("No message received in loopback mode\n");
        return false;
    }
    
    bool match = true;
    if (rxMsg.can_id != txMsg.can_id) {
        printf("Error: Received ID 0x%03X does not match sent ID 0x%03X\n", 
               rxMsg.can_id, txMsg.can_id);
        match = false;
    }
    
    if (rxMsg.can_dlc != txMsg.can_dlc) {
        printf("Error: Received DLC %d does not match sent DLC %d\n", 
               rxMsg.can_dlc, txMsg.can_dlc);
        match = false;
    }
    
    for (int i = 0; i < rxMsg.can_dlc; i++) {
        if (rxMsg.data[i] != txMsg.data[i]) {
            match = false;
        }
    }
    printf("\n");
    
    if (!match) {
        printf("Data mismatch between sent and received messages\n");
        return false;
    }
    
    return true;
}

bool mcp_2515_init(CAN_SPEED speed, CAN_CLOCK clock, spi_inst_t *SPI_PORT, uint8_t PIN_CS, uint8_t PIN_MOSI, uint8_t PIN_MISO, uint8_t PIN_SCK, uint32_t SPI_BAUDRATE){

	spi_init_device(SPI_PORT, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUDRATE);

    MCP2515 can0(SPI_PORT, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUDRATE); 

	MCP2515::ERROR err = can0.reset();
    sleep_ms(100);
	if(err != MCP2515::ERROR_OK) {
		printf("Reset error: %d\n", err);
        return false;
	}
	
	// Set config mode
	err = can0.setConfigMode();
	if(err != MCP2515::ERROR_OK) {
		printf("Config mode error: %d\n", err);
        return false;
	}
	
	// Verify CANSTAT (bit 7-5 = 100 for config mode)
	uint8_t canstat = can0.readRegister(MCP2515::MCP_CANSTAT);
	if((canstat & 0xE0) != 0x80) {
		printf("Not in config mode! CANSTAT=0x%02X\n", canstat);
        return false;
	}
	
	// Set bitrate (critical for internal timing)
	err = can0.setBitrate(speed, clock);
	if(err != MCP2515::ERROR_OK) {
		printf("Bitrate error: %d\n", err);
        return false;
	}

    printf("Starting MCP2515 loopback test...\n");  
    if (!test_mcp2515_loopback(can0)){
        printf("Loopback test failed\n");
        return false;
    }

    err = can0.reset();
    sleep_ms(100);
	if(err != MCP2515::ERROR_OK) {
		printf("Reset error: %d\n", err);
        return false;
	}
	
	// Set bitrate (critical for internal timing)
	err = can0.setBitrate(speed, clock);
	if(err != MCP2515::ERROR_OK) {
		printf("Bitrate error: %d\n", err);
        return false;
	}

    err = can0.setNormalMode();
    if (err != MCP2515::ERROR_OK) {
        printf("Error setting normal mode: %d\n", err);
        return false;
    }

    return true;


}