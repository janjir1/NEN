#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "src/registers.hpp"
#include "mcp2515/mcp2515.h"
#include "sh1107/sh110x.hpp"


///////////////////////////////////////////////
/*
OBHAJOBA 6.5.

1-2 strany technicka dokumentace pro Levka
vyvojovy diagram, tabulka atd
1 strana EMC
Pak Novotny 
*/
///////////////////////////////////////////////

// =============== Defines =========================

// GPIO Defines
#define LED_pin 25
#define BTN_pin 24
#define NEOPIXEL_pin 23

// SPI Defines
#define SPI_PORT spi0
#define SPI_BAUD 10000000 // 10MHz
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// SH1107 Defines
#define myOLEDwidth  128
#define myOLEDheight 128
#define FULLSCREEN (myOLEDwidth * (myOLEDheight/8))
uint8_t  screenBuffer[FULLSCREEN];

SH110X myOLED(myOLEDwidth ,myOLEDheight) ; // instantiate a OLED object

// I2C settings
const uint8_t ADDR = 0x3C; // I2C address
const uint16_t SPEED = 400; // I2C clock speed in kilo hertz
const uint8_t CLK_PIN = 13;
const uint8_t DATA_PIN = 12;

// Reset only needed if Hardware reset pin is present on device and used
int _RESET_PIN = -1; // set to -1 if not used

// MCP2515 CAN defines
#define PID_COOLANT_TEMP 0x05 // PID for coolant temperature
#define PID_VAG 0x7E0 // PID for coolant temperature 2



// =============== Function prototype ================
bool SetupTest(void);
void TestLoop(void);
void EndTest(void);

// MCP2515 defines
MCP2515 can0(spi0, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUD); 
struct can_frame rx;
struct can_frame tx; // CAN frame structure for sending messages





int main()
{
    stdio_init_all();

    //Initialize CAN0
    can0.reset();
    can0.setBitrate(CAN_1000KBPS, MCP_8MHZ);
    can0.setListenOnlyMode();

    
    if(SetupTest()) TestLoop();
	//EndTest();
    
	tx.can_id = PID_COOLANT_TEMP; // CAN ID
	tx.can_dlc = 0; // Data length code
	can0.sendMessage(&tx); // Send message
	busy_wait_ms(1);
	if(can0.readMessage(&rx) == MCP2515::ERROR_OK) {
		printf("\nNew frame from ID: %10x\n", rx.can_id);
		printf("DLC: %d\n", rx.can_dlc);
		for(int i = 0; i < rx.can_dlc; i++) {
			printf("Data[%d]: %02x\n", i, rx.data[i]);
		}
	}

    while (true) {
        // printf("Hello, world!\n");
        // sleep_ms(1000);
		if(can0.readMessage(&rx) == MCP2515::ERROR_OK) {
            printf("\nNew frame from ID: %10x\n", rx.can_id);
			printf("DLC: %d\n", rx.can_dlc);
			for(int i = 0; i < rx.can_dlc; i++) {
				printf("Data[%d]: %02x\n", i, rx.data[i]);
			}
        }
    }
}




bool SetupTest() 
{
	//stdio_init_all(); // Initialize chosen serial port, default 38400 baud
	busy_wait_ms(500);
	printf("OLED SH1107 :: Start!\r\n");
	while(myOLED.OLEDbegin(myOLED.SH1107_IC, _RESET_PIN, ADDR, i2c0, 
					SPEED, DATA_PIN, CLK_PIN) != DisplayRet::Success)
	{
		printf("SetupTest ERROR : Failed to initialize OLED!\r\n");
		busy_wait_ms(1500);
	} // initialize the OLED
	if (myOLED.OLEDSetBufferPtr(myOLEDwidth, myOLEDheight, screenBuffer) != DisplayRet::Success)
	{
		printf("SetupTest : ERROR : OLEDSetBufferPtr Failed!\r\n");
		return false;
	} // Initialize the buffer
	myOLED.OLEDFillScreen(0xF0, 0); // splash screen bars
	busy_wait_ms(1000);
	return true;
}

void EndTest()
{
	myOLED.OLEDPowerDown();
	myOLED.OLED_I2C_OFF();
	printf("OLED Test End\r\n");
}

void TestLoop()
{	
	if (myOLED.OLEDSetBufferPtr(myOLEDwidth, myOLEDheight, screenBuffer) != DisplayRet::Success) return;
	myOLED.OLEDclearBuffer();
	myOLED.setFont(pFontDefault);
	myOLED.setCursor(10, 10);
	myOLED.print("Hello World.");
	myOLED.OLEDupdate();
	busy_wait_ms(5000);
}
