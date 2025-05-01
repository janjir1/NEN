#include <stdio.h>
#include "defines.hpp"
#include "OLED_module.hpp"
#include "string.h"
#include "pico/stdlib.h"

SH110X myOLED(myOLEDwidth ,myOLEDheight); // instantiate a OLED object
uint8_t screenBuffer[FULLSCREEN/8];

// ====== Local Functions ======
void renderThrottle(int16_t* throttle);
void renderRPM(int16_t* rpm);

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

void myOLED_result(Result* res, Result* throttle, Result* rpm)
{
    if (myOLED.OLEDSetBufferPtr(myOLEDwidth, myOLEDheight, screenBuffer) != DisplayRet::Success)
	{
		printf("myOLED_result : ERROR : OLEDSetBufferPtr Failed!\r\n");
		return;
	}

	myOLED.OLEDclearBuffer();

	

	myOLED.setFont(pFontDefault);
	myOLED.setCursor(((myOLEDwidth - strlen(res->name)*6)/2), 36);
	myOLED.print(res->name);

	myOLED.setFont(pFontArialRound);
	//myOLED.setCursor(((myOLEDwidth - (sizeof(value)-1)*16)/2), 128/2-12);
    int x = 0;
    int16_t temp = res->value;
    if (temp < 0)
    {
        temp = -temp; // absolute value
        x++;
    }
    switch (temp)
    {
    case 0 ... 9:
        x += 1;
        break;
    case 10 ... 99:
        x += 2;
        break;
    case 100 ... 999:
        x += 3;
        break;
    case 1000 ... 9999:
        x += 4;
        break;
    case 10000 ... 32767:
        x += 5;
        break;
    
    default:
        break;
    }

    myOLED.setCursor((myOLEDwidth - x*16)/2, 128/2-12);
	myOLED.print(res->value, 10); // print value in decimal
	
	myOLED.setFont(pFontArialBold);
	myOLED.setCursor(((myOLEDwidth - strlen(res->unit)*16)/2), 128/2+14);
	myOLED.print(res->unit);

    //myOLED.drawRect((myOLEDwidth - 5*16)/2 - 1, (myOLEDwidth - 5*16)/2 - 1, 5*16 + 2, 5*16 + 2, 1); // draw border
    //myOLED.drawRect(0, 0, myOLEDwidth, myOLEDheight, 1); // draw border

    renderThrottle(&throttle->value); // draw throttle bar
    renderRPM(&rpm->value); // draw RPM bar

	myOLED.OLEDupdate();
    return;
}

void renderThrottle(int16_t* throttle)
{
    if (0 > *throttle || *throttle > 100)
    {
        printf("Wrong Throttle %: %d\r\n", *throttle);
        return;
    }
    myOLED.setFont(pFontDefault);
    myOLED.setCursor(110, 10);
    myOLED.print("Gas");

    myOLED.drawFastHLine(127, 127-100, -17, 1); // draw throttle bar base
    myOLED.drawFastHLine(127, 127-75, -12, 1); // draw throttle bar base
    myOLED.drawFastHLine(127, 127-50, -17, 1); // draw throttle bar base
    myOLED.drawFastHLine(127, 127-25, -12, 1); // draw throttle bar base
    myOLED.drawFastHLine(127, 127, -17, 1); // draw throttle bar base
    myOLED.fillRect(118, 128, 10, -(*throttle), 1); // fill throttle bar
    printf("Throttle %: %d\r\n", *throttle);
}

void renderRPM(int16_t* rpm)
{
    if (0 > *rpm || *rpm > 6000)
    {
        printf("Wrong RPM %: %d\r\n", *rpm);
        return;
    }
    int16_t temp = *rpm / 6000.0 * 100; // convert to percentage
    myOLED.setFont(pFontDefault);
    myOLED.setCursor(2, 10);
    myOLED.print("RPM");

    myOLED.drawFastHLine(0, 127-100, 17, 1); // draw RPM bar base
    myOLED.drawFastHLine(0, 127-83, 17, 1); // draw RPM bar base
    myOLED.drawFastHLine(0, 127-67, 17, 1); // draw RPM bar base
    myOLED.drawFastHLine(0, 127-50, 17, 1); // draw RPM bar base
    myOLED.drawFastHLine(0, 127-33, 17, 1); // draw RPM bar base
    myOLED.drawFastHLine(0, 127-17, 17, 1); // draw RPM bar base
    myOLED.drawFastHLine(0, 127, 17, 1); // draw RPM bar base
    myOLED.fillRect(0, 128, 10, -(temp), 1); // fill RPM bar
    printf("RPM %: %d\r\n", *rpm);
}