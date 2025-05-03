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
    
	// ==== OLED Init ====
	myOLED_init(); // Initialize the OLED display
	
	// ==== OLED Example ====
	Result res1;
	res1.name = "Test";
	res1.value = -123;
	res1.unit = "unit";

	Result res2;
	res2.name = "Test2test";
	res2.value = 10788;
	res2.unit = "u/gC%^";

	Result throttle;
	throttle.name = "Throttle";
	throttle.value = 0;
	throttle.unit = "%";

	Result rpm;
	rpm.name = "RPM";
	rpm.value = 0;
	rpm.unit = "rpm";
	
	myOLED_result(&res1, &throttle, &rpm);
	sleep_ms(2000);
	myOLED_result(&res2, &throttle, &rpm);
	sleep_ms(2000);

	for (int16_t i = 0; i < 100; i++)
	{
		throttle.value = i;
		rpm.value = i * 60;
		myOLED_result(&res2, &throttle, &rpm);
		sleep_ms(2);
	}
	

	sleep_ms(10000);

	bool post_pass = mcp_2515_init(CAN_500KBPS, MCP_8MHZ, MY_SPI_PORT, MY_PIN_CS, MY_PIN_MOSI, MY_PIN_MISO, MY_PIN_SCK, MY_SPI_BAUDRATE);

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

