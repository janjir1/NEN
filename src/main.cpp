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

struct can_frame txMsg;
struct can_frame rxMsg;

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

// add some return if failed
void mcp_2515_init(CAN_SPEED speed, CAN_CLOCK clock){

	spi_init_device();

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
	err = can0.setBitrate(speed, clock);
	if(err != MCP2515::ERROR_OK) {
		printf("Bitrate error: %d\n", err);
	}
}

bool test_mcp2515_loopback(MCP2515 &can) {
    printf("Starting MCP2515 loopback test...\n");
    
    // Reset the MCP2515
    can.reset();
    sleep_ms(100); // Allow time for reset to complete
    
    mcp_2515_init(CAN_125KBPS, MCP_8MHZ);
    
    err = can.setLoopbackMode();
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
    
    // Verify the received message matches the sent message
    printf("Message received with ID: 0x%03X\n", rxMsg.can_id);
    
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
    
    printf("Data received: ");
    for (int i = 0; i < rxMsg.can_dlc; i++) {
        printf("0x%02X ", rxMsg.data[i]);
        if (rxMsg.data[i] != txMsg.data[i]) {
            match = false;
        }
    }
    printf("\n");
    
    if (!match) {
        printf("Data mismatch between sent and received messages\n");
        return false;
    }
    
    printf("Loopback test passed: sent and received messages match\n");
    return true;
}

int main() {
    stdio_init_all();
    
	sleep_ms(10000);

	if (test_mcp2515_loopback(can0)) {
        printf("Test_pass\n");
    } else {
        printf("Test_fail\n");
    }

	mcp_2515_init(CAN_500KBPS, MCP_8MHZ);

	//err = can0.setNormalMode();
	err = can0.setListenOnlyMode();
    if (err != MCP2515::ERROR_OK) {
        printf("Error setting listening mode: %d\n", err);
        return false;
    }

	while(true){
		
		MCP2515::ERROR result = can0.readMessage(&rxMsg);
        
        if(result == MCP2515::ERROR_OK) {
            // Print message metadata
            printf("\nCAN ID: 0x%08X", rxMsg.can_id);
            printf(" DLC: %d", rxMsg.can_dlc);
            
            // Detect frame type
            if(rxMsg.can_id & CAN_RTR_FLAG) {
                printf(" [RTR Frame]");
            } else if(rxMsg.can_id & CAN_EFF_FLAG) {
                printf(" [Extended Frame]");
            } else {
                printf(" [Standard Frame]");
            }
            
            // Print data payload
            printf("\nData: ");
            for(int i = 0; i < rxMsg.can_dlc; i++) {
                printf("%02X ", rxMsg.data[i]);
            }
            printf("\n");

        } else if(result != MCP2515::ERROR_NOMSG) {
            //printf("Error reading message: %d\n", result);
        }
        
        sleep_ms(10); // Prevent tight loop
		
		/*	-------------------minimal example -------------------------------------- requires normal
			if(can0.readMessage(&rx) == MCP2515::ERROR_OK) {
				printf("New frame from ID: %10x\n", rxMsg.can_id);
			}
		*/	
			/* -------------------ask rpm -------------------------------------- requires normal
			struct can_frame txMsg;
			txMsg.can_id = 0x7DF;  // OBD-II broadcast request ID
			txMsg.can_dlc = 8;      // Always 8 bytes for OBD-II requests
			
			// OBD-II RPM request (PID 0x0C) 
			txMsg.data[0] = 0x02;   // Number of additional bytes
			txMsg.data[1] = 0x01;   // Mode 01 (Current data)
			txMsg.data[2] = 0x0C;   // PID 0x0C (Engine RPM)
			txMsg.data[3] = 0x55;   // Padding
			txMsg.data[4] = 0x55;   // Padding
			txMsg.data[5] = 0x55;   // Padding
			txMsg.data[6] = 0x55;   // Padding
			txMsg.data[7] = 0x55;   // Padding

			err = can0.sendMessage(&txMsg);
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
				err = can0.readMessage(&rxMsg);
				if (err == MCP2515::ERROR_OK) {
					printf("Frame recived\nCAN ID: 0x%08X\n", rxMsg.can_id);
					received = true;
					if(rxMsg.can_id == 0x7E8 && rxMsg.data[2] == 0x0C) {
						uint16_t rpm = (rxMsg.data[3] << 8 | rxMsg.data[4]) / 4;
						printf("Engine RPM: %d\n", rpm);
					}
					break;
				}
				sleep_ms(10); // Small delay to prevent tight loop
			}
			
			if (!received) {
				printf("No message received in loopback mode\n");
				return false;
			}
				*/
	}
}