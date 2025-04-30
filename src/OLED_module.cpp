#include <stdio.h>
#include "OLED_module.hpp"

SH110X myOLED(myOLEDwidth ,myOLEDheight); // instantiate a OLED object
uint8_t screenBuffer[FULLSCREEN/8];

void myOLED_init(void)
{
	myOLED.OLEDbegin(SH110X::SH1107_IC, I2C_RESET_PIN, I2C_ADDR, I2C_PORT, I2C_SPEED, I2C_DATA_PIN, I2C_CLK_PIN);
	myOLED.OLEDinit();
	myOLED.OLEDclearBuffer();
	myOLED.OLEDFillScreen(0xFF, 0);
	myOLED.OLEDupdate();
	sleep_ms(100);
	myOLED.OLEDFillScreen(0x00, 0);
	myOLED.OLEDupdate();
	sleep_ms(100);
}

void TestLoop(void)
{	
	if (myOLED.OLEDSetBufferPtr(myOLEDwidth, myOLEDheight, screenBuffer) != DisplayRet::Success)
	{
		printf("TestLoop : ERROR : OLEDSetBufferPtr Failed!\r\n");
		return;
	}
	char name[] = "Engine speed:";
	char value[] = "1024";
	char unit[] = "RPM";
	
	myOLED.OLEDclearBuffer();

	myOLED.drawRect(0, 0, myOLEDwidth, myOLEDheight, 1); // draw border

	myOLED.setFont(pFontDefault);
	myOLED.setCursor(((myOLEDwidth - (sizeof(name)-1)*6)/2), 36);
	myOLED.print(name);

	myOLED.setFont(pFontArialRound);
	myOLED.setCursor(((myOLEDwidth - (sizeof(value)-1)*16)/2), 128/2-12);
	myOLED.print(value);
	
	myOLED.setFont(pFontArialBold);
	myOLED.setCursor(((myOLEDwidth - (sizeof(unit)-1)*16)/2), 128/2+14);
	myOLED.print(unit);

	myOLED.OLEDupdate();
	busy_wait_ms(1000);
}