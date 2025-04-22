#include "src/registers.hpp"
#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/i2c.h"


void custom_i2c_init(i2c_inst_t* i2c_type, uint16_t CLKspeed, uint8_t  SDApin, uint8_t  SCLKpin)
{
	//IO_BANK0_GPIO12_CTRL_FUNCSEL_VALUE_I2C0_SDA
    custom_set_pin_function(SDApin, I2C);	//Set SDA pin to I2C function
    custom_set_pin_function(SCLKpin, I2C);	//Set SCL pin to I2C function

	// Enable pull-up
	custom_set_pin_pullup(SDApin);	//Enable pull-up on SDA pin
    custom_set_pin_pullup(SCLKpin);	//Enable pull-up on SCL pin
}

void custom_set_pin_function(uint8_t pin, pin_func function)
{
	volatile uint32_t* gpio_ctrl;
	gpio_ctrl = reinterpret_cast<uint32_t*>(custom_IO_BANK0_BASE + custom_IO_BANK0_GPIO_CTRL_OFFSET + (pin*2*custom_IO_BANK0_GPIO_CTRL_OFFSET));	//IO_BANK0: 2.19.6.1 pg: 244
	*gpio_ctrl = (*gpio_ctrl & ~NONE) | function;	//Set function to pin
}

void custom_set_pin_pullup(uint8_t pin)
{
	volatile uint32_t* pad_ctrl;
	pad_ctrl = reinterpret_cast<uint32_t*>(custom_PADS_BANK0_BASE + (pin + 1)*custom_PADS_BANK0_GPIO_OFFSET);	//PADS_BANK0: 2.19.6.3 pg: 302
	*pad_ctrl = (*pad_ctrl & ~0x18) | (1<<4);
}