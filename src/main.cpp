#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "mcp2515/mcp2515.h"  
#include "CAN_module.hpp"// Requires MCP2515 CAN controller library
#include "sh1107/sh110x.hpp"
#include "OLED_module.hpp" // Requires SH1107 OLED library
#include "defines.hpp" // Defines for GPIO, SPI, I2C, etc.
#include "PIO_button.hpp"
#include "flash.hpp"
#include "hardware/watchdog.h"


const uint8_t monitored_aditional_PIDs[] = {PID_VEHICLE_SPEED};
const uint8_t PID_array_length = sizeof(monitored_aditional_PIDs) / sizeof(monitored_aditional_PIDs[0]);

const PIO pio = pio0;

Result throttle;
Result rpm;
Result res;

uint32_t sizes[] = {sizeof(rpm.time), sizeof(rpm.pid), sizeof(rpm.value)};
const uint8_t count = sizeof(sizes) / sizeof(sizes[0]);

bool record = false;
bool record_init = false;
bool clear_flash = false;
uint8_t display_show = 0;
uint8_t write_error = 0;
uint32_t last_button_press_time = 0;


void flash_write_CAN(Result *out){
	if(record){
		if (!flash_write(&out->time, sizeof(out->time), false)){
			write_error++;
			printf("Write error count: %d\n", write_error);
		}
		if (!flash_write(&out->pid, sizeof(out->pid), false)){
			write_error++;
			printf("Write error count: %d\n", write_error);
		}
		if (!flash_write(&out->value, sizeof(out->value), false)){
			write_error++;
			printf("Write error count: %d\n", write_error);
		}
	}
}


int main() {

	stdio_init_all();


	// ==== Inits ====
	
	myOLED_init(); // Initialize the OLED display
	init_pio_button(pio, BTN_PIN);


	bool post_pass = mcp_2515_init(CAN_500KBPS, MCP_8MHZ, MY_SPI_PORT, MY_PIN_CS, MY_PIN_MOSI, MY_PIN_MISO, MY_PIN_SCK, MY_SPI_BAUDRATE);

	if (!post_pass){
		while(true){
			printf("MCP2515 POST FAIL\n");
			sleep_ms(10000);
		}
	}
	sleep_ms(2000);
	if (was_button_pressed(pio)) {
        clear_flash = true;
		printf("Flash will be cleared");
    }


	printf("POST PASS");
    
	rpm.name = "RPM";
	rpm.value = 0;
	rpm.unit = "rpm";

	throttle.name = "Throttle";
	throttle.value = 0;
	throttle.unit = "%";

	res.name = "Unknown";
	throttle.value = 0;
	throttle.unit = "Unknown";

	myOLED_result(&res, &throttle, &rpm);
	

	while(true){

		if (record_init){
			//TODO handle full/empty storage
			uint32_t free_space = flash_init(sizes, count, clear_flash);
			if(clear_flash) printf("Flash clear");
			clear_flash = false;
			record_init = false;
			//print free avalible space to display
			Result message;
			message.name = "There is";
			message.value = (int16_t)free_space;
			message.unit = "kB";
			myOLED_result(&message, &throttle, &rpm);
			sleep_ms(2000);
			myOLED_result(&res, &throttle, &rpm);
		}
		
		if (get_decoded_data(PID_ENGINE_RPM, &rpm)) {
			printf("%s: %u %s\n", rpm.name, rpm.value, rpm.unit);
			flash_write_CAN(&rpm);
		}
		sleep_ms(POLL_PERIOD_MS);

		if (get_decoded_data(PID_THROTTLE_POSITION, &throttle)) {
			printf("%s: %u %s\n", throttle.name, throttle.value, throttle.unit);
			flash_write_CAN(&throttle);
		}
		sleep_ms(POLL_PERIOD_MS);

		for (uint8_t i = 0; i < PID_array_length; i++) {
			if (get_decoded_data(PID_THROTTLE_POSITION, &res)) {
				printf("%s: %u %s\n", res.name, res.value, res.unit);
				flash_write_CAN(&res);
				if(display_show == i){
					myOLED_result(&res, &throttle, &rpm);
				}
				else{
					sleep_ms(POLL_PERIOD_MS);
				}
			}
			else{
				sleep_ms(POLL_PERIOD_MS);
			}
			
		}

		if(was_button_pressed(pio)){
			if((to_ms_since_boot(get_absolute_time()) - last_button_press_time < 1000)){
				printf("\nDouble tap\n");
				if(record == false){
					Result message;
					message.name = "Log start";
					message.value = 0;
					message.unit = " ";
					myOLED_result(&message, &throttle, &rpm);
					sleep_ms(2000);
					if(was_button_pressed(pio)){
						record_init = true;
						record = true;
						printf("Log start\n");
					}
					else{
						myOLED_result(&res, &throttle, &rpm);
					}
				}
				else{
					Result message;
					message.name = "Log end?";
					message.value = 0;
					message.unit = " ";
					myOLED_result(&message, &throttle, &rpm);
					sleep_ms(2000);
					if(was_button_pressed(pio)){
						record = false;
						flash_end();
						printf("Log end\n");
						Result message;
						message.name = "Log ended";
						message.value = 0;
						message.unit = " ";
						myOLED_result(&message, &throttle, &rpm);
						sleep_ms(2000);
						myOLED_result(&res, &throttle, &rpm);
					}
					else{
						myOLED_result(&res, &throttle, &rpm);
					}
				}
				
				display_show = display_show - 1;
				if(display_show > 254){
					display_show = PID_array_length;
				}
				
			}
			else{
				display_show++;
				if(display_show >= PID_array_length){
					display_show = 0;
				}
			}
			last_button_press_time = to_ms_since_boot(get_absolute_time());

		}
		





			
	}
}

