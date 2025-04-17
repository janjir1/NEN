#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "src/registers.hpp"
#include "mcp2515/mcp2515.h"
#include "sh1107/sh110x.hpp"

// SPI Defines
#define SPI_PORT spi0
#define SPI_BAUD 10000000 // 10MHz
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// Screen Size 
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


// =============== Function prototype ================
void custom_i2c_init(i2c_inst_t* i2c_type, uint16_t CLKspeed, uint8_t  SDApin, uint8_t  SCLKpin);
bool SetupTest(void);
void TestLoop(void);
void EndTest(void);

// MCP2515 defines
MCP2515 can0(spi0, PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, SPI_BAUD); 
struct can_frame rx;




int main()
{
    stdio_init_all();

    //Initialize CAN0
    can0.reset();
    can0.setBitrate(CAN_1000KBPS, MCP_8MHZ);
    can0.setListenOnlyMode();

    
    if(SetupTest()) TestLoop();
	//EndTest();
    


    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}


void custom_i2c_init(i2c_inst_t* i2c_type, uint16_t CLKspeed, uint8_t  SDApin, uint8_t  SCLKpin)
{
	//IO_BANK0_GPIO12_CTRL_FUNCSEL_VALUE_I2C0_SDA
	volatile uint32_t* gpio_ctrl;
	gpio_ctrl = reinterpret_cast<uint32_t*>(0x40014000 + (0x64));	//IO_BANK0: 2.19.6.1
	*gpio_ctrl = (*gpio_ctrl & ~0x1F) | 2;
	gpio_ctrl = reinterpret_cast<uint32_t*>(0x40014000 + (0x64));
	*gpio_ctrl = (*gpio_ctrl & ~0x1F) | 2;

	// Enable pull-up
	volatile uint32_t* pad_ctrl;
	pad_ctrl = reinterpret_cast<uint32_t*>(0x4001c000 + 13*0x04);	//PADS_BANK0: 2.19.6.3 pg: 302
	*pad_ctrl = (*pad_ctrl & ~0x18) | (1 << 4);
	
}

void custom_set_pin_function(uint8_t pin, pin_func function)
{
	volatile uint32_t* gpio_ctrl;
	gpio_ctrl = reinterpret_cast<uint32_t*>(0x40014000 + (0x64));	//IO_BANK0: 2.19.6.1
	*gpio_ctrl = (*gpio_ctrl & ~0x1F) | GPIO_FUNC_I2C;
	
}

void custom_set_pin_pullup(uint8_t pin)
{
	volatile uint32_t* pad_ctrl;
	pad_ctrl = reinterpret_cast<uint32_t*>(0x4001c000 + (pin + 1)*0x04);	//PADS_BANK0: 2.19.6.3 pg: 302
	*pad_ctrl = (*pad_ctrl & ~0x18) | (1<<4);
}

bool SetupTest() 
{
	stdio_init_all(); // Initialize chosen serial port, default 38400 baud
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
