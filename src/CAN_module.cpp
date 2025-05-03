#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "mcp2515/mcp2515.h"
#include <new>
#include "CAN_module.hpp"

//MCP2515* can0 = nullptr;
//no idea what this is "promises to compiler that there will be can0 instance"
alignas(MCP2515) static char can0_buf[sizeof(MCP2515)];
static MCP2515& can0 = *reinterpret_cast<MCP2515*>(can0_buf);


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
    //printf("Sending test message with ID: 0x%03X\n", txMsg.can_id);
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
    
    if (!match) {
        printf("Data mismatch between sent and received messages\n");
        return false;
    }
    
    return true;
}

bool mcp_2515_init(CAN_SPEED speed, CAN_CLOCK clock, spi_inst_t *SPI_PORT, uint8_t PIN_CS, uint8_t PIN_MOSI, uint8_t PIN_MISO, uint8_t PIN_SCK, uint32_t SPI_BAUDRATE){

	spi_init_device(SPI_PORT, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUDRATE);

    new (&can0) MCP2515(SPI_PORT, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUDRATE); 

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

    //printf("Starting MCP2515 loopback test...\n");  
    if (!test_mcp2515_loopback(can0)){
        printf("Loopback test failed\n");
        return false;
    }
    //printf("Loopback test passed\n");

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

bool get_data(uint8_t pid, struct can_frame &rxMsgOut) {

    struct can_frame txMsg;
    txMsg.can_id = 0x7DF;  // OBD-II broadcast request ID
    txMsg.can_dlc = 8;      // Always 8 bytes
    
    txMsg.data[0] = 0x02;   // Number of additional bytes
    txMsg.data[1] = 0x01;   // Mode 01 (Current data), 02 for freze frame, donno how that works, dont need
    txMsg.data[2] = pid;

    for (int i = 3; i < 8; i++) {
        txMsg.data[i] = 0x55;
    }

    MCP2515::ERROR err = can0.sendMessage(&txMsg);
    if (err != MCP2515::ERROR_OK) {
        printf("Error sending message to 0x%02X\n", pid);
        return false;
    }
    
    // Wait for the message to be received
    struct can_frame rxMsg;
    uint32_t startTime = to_ms_since_boot(get_absolute_time());
    
    // Try to receive for up to 1 second
    while (to_ms_since_boot(get_absolute_time()) - startTime < 1000) {
        err = can0.readMessage(&rxMsg);
        if (err == MCP2515::ERROR_OK) {
            if(rxMsg.can_id >= 0x7E8 && rxMsg.can_id <= 0x7EF){ //if its response to a request
                if(rxMsg.data[2] == pid){ //from what we want
                    rxMsgOut = rxMsg;
                    return true;
                }
            }
        }
        sleep_ms(10);
    }
    printf("Answer not recicved: timeout");
    return false;
}

bool decode_data(const struct can_frame& rxMsg, Result* out){

    if (!out) {
        printf("Incorect variable setup");
        return false;
    }


    out->time = to_ms_since_boot(get_absolute_time());
    out->pid = rxMsg.data[2];

    switch (rxMsg.data[2]) {
        case 0x04: // Engine load
            out->name  = "Engine Load";
            out->value = rxMsg.data[3] * 100.0f / 255.0f;
            out->unit  = "%";
            return true;

        case 0x05: // Coolant temperature
            out->name  = "Coolant Temperature";
            out->value = rxMsg.data[3] - 40.0f;
            out->unit  = "°C";
            return true;

        case 0x0C: // Engine RPM
            out->name  = "Engine RPM";
            out->value = ((rxMsg.data[3] << 8) | rxMsg.data[4]) / 4.0f;
            out->unit  = " rpm";
            return true;

        case 0x0D: // Vehicle speed
            out->name  = "Vehicle Speed";
            out->value = (float)rxMsg.data[3];
            out->unit  = " km/h";
            return true;

        case 0x0F: // Intake air temperature
            out->name  = "Intake Air Temp";
            out->value = rxMsg.data[3] - 40.0f;
            out->unit  = "°C";
            return true;

        case 0x11: // Throttle position
            out->name  = "Throttle Position";
            out->value = rxMsg.data[3] * 100.0f / 255.0f;
            out->unit  = "%";
            return true;

        case 0x2F: // Fuel level
            out->name  = "Fuel Level";
            out->value = rxMsg.data[3] * 100.0f / 255.0f;
            out->unit  = "%";
            return true;

        case 0x42: // Control module voltage
            out->name  = "Control Module Voltage";
            out->value = ((rxMsg.data[3] << 8) | rxMsg.data[4]) / 1000.0f;
            out->unit  = "V";
            return true;

        case 0x46: // Ambient air temperature
            out->name  = "Ambient Air Temp";
            out->value = rxMsg.data[3] - 40.0f;
            out->unit  = "°C";
            return true;

        case 0x0A: // Fuel pressure
            out->name  = "Fuel Pressure";
            out->value = rxMsg.data[3] * 3.0f;
            out->unit  = "kPa";
            return true;

        case 0x10: // MAF air flow rate
            out->name  = "MAF Air Flow Rate";
            out->value = ((rxMsg.data[3] << 8) | rxMsg.data[4]) / 100.0f;
            out->unit  = "g/s";
            return true;

        case 0x1F: // Runtime since engine start
            out->name  = "Runtime";
            out->value = (float)((rxMsg.data[3] << 8) | rxMsg.data[4]);
            out->unit  = "s";
            return true;

        case 0x21: // Distance with MIL on
            out->name  = "Distance w/ MIL on";
            out->value = (float)((rxMsg.data[3] << 8) | rxMsg.data[4]);
            out->unit  = "km";
            return true;

    default:
    // PID not implemented
        printf("Recived PID is unknown");
        return false;
    }
}

bool get_decoded_data(uint8_t pid, Result* out){
    struct can_frame raw_data;
    if (get_data(pid, raw_data)) {
        if (decode_data(raw_data, out)){
            return true;
        }
    }
    printf("Get_decoded_data FAILED: ");
    return false;
}