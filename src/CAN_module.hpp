#ifndef CAN_MODULE_HPP
#define CAN_MODULE_HPP

#include <stdint.h>
#include "hardware/spi.h"
#include "mcp2515/mcp2515.h"

// PIDs for OBD-II communication
#define PID_ENGINE_LOAD 0x04                // Engine load
#define PID_COOLANT_TEMPERATURE 0x05        // Engine coolant temperature
#define PID_ENGINE_RPM 0x0C                 // Engine RPM
#define PID_VEHICLE_SPEED 0x0D              // Vehicle speed
#define PID_INTAKE_AIR_TEMPERATURE 0x0F     // Intake air temperature
#define PID_MAF_AIR_FLOW_RATE 0x10          // MAF air flow rate
#define PID_THROTTLE_POSITION 0x11          // Throttle position
#define PID_RUNTIME_SINCE_ENGINE_START 0x1F // Runtime since engine start (seconds)
#define PID_DISTANCE_WITH_MIL_ON 0x21       // Distance traveled with MIL on (km)
#define PID_FUEL_PRESSURE 0x0A              // Fuel pressure (kPa)
#define PID_FUEL_LEVEL 0x2F                 // Fuel level input
#define PID_CONTROL_MODULE_VOLTAGE 0x42     // Control module voltage
#define PID_AMBIENT_AIR_TEMPERATURE 0x46    // Ambient air temperature

// Result structure for decoded CAN data
typedef struct {
    uint8_t pid;
    uint32_t time;
    const char* name;  // pointer to static string
    int16_t value;    // decoded value
    const char* unit;  // pointer to static string
} Result;

// External reference to the global CAN controller
// extern MCP2515& can0;

// Function declarations
void spi_init_device(spi_inst_t *SPI_PORT, uint8_t PIN_CS, uint8_t PIN_MOSI, 
                     uint8_t PIN_MISO, uint8_t PIN_SCK, uint32_t SPI_BAUDRATE);

bool test_mcp2515_loopback(MCP2515 &can);

bool mcp_2515_init(CAN_SPEED speed, CAN_CLOCK clock, spi_inst_t *SPI_PORT, 
                  uint8_t PIN_CS, uint8_t PIN_MOSI, uint8_t PIN_MISO, 
                  uint8_t PIN_SCK, uint32_t SPI_BAUDRATE);

bool get_data(uint8_t pid, struct can_frame &rxMsgOut);

bool decode_data(const struct can_frame& rxMsg, Result* out);

bool get_decoded_data(uint8_t pid, Result* out);

#endif // CAN_MODULE_HPP
