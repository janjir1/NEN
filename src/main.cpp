#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "mcp2515/mcp2515.h"  
#include "CAN_module.hpp"// Requires MCP2515 CAN controller library
#include "sh1107/sh110x.hpp"
#include "OLED_module.hpp" // Requires SH1107 OLED library
#include "defines.hpp" // Defines for GPIO, SPI, I2C, etc.


// Definitions
MCP2515::ERROR err;

struct can_frame txMsg;
struct can_frame rxMsg;

//MCP2515 can0;  // CAN controller instance/
//MCP2515 can0(SPI_PORT, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUDRATE); 





int main() {
    stdio_init_all();
    
	//myOLED_init(I2C_ADDR, I2C_SPEED, I2C_CLK_PIN, I2C_DATA_PIN, I2C_RESET_PIN);

	myOLED_init(); // Initialize the OLED display
	

	sleep_ms(10000);

	bool post_pass = mcp_2515_init(CAN_500KBPS, MCP_8MHZ, SPI_PORT, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUDRATE);

	if (!post_pass){
		while(true){
			printf("POST FAIL");
			sleep_ms(10000);
		}
	}

	printf("POST PASS");
	Result res;

	while(true){
		
		if (get_decoded_data(PID_ENGINE_RPM, &res)) {
			printf("%s: %u %s\n",
				res.name, res.value, res.unit);
		}
		sleep_ms(100);

		if (get_decoded_data(PID_COOLANT_TEMPERATURE, &res)) {
			printf("%s: %u %s\n",
				res.name, res.value, res.unit);
		}
		sleep_ms(100);

		if (get_decoded_data(PID_THROTTLE_POSITION, &res)) {
			printf("%s: %u %s\n",
				res.name, res.value, res.unit);
		}
		sleep_ms(100);
			
	}
}

