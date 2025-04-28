#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "mcp2515/mcp2515.h"  
#include "src/CAN_module.cpp"// Requires MCP2515 CAN controller library

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
//MCP2515 can0(SPI_PORT, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUDRATE); 



int main() {
    stdio_init_all();
    
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