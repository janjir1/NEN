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
    return;
}

void custom_set_pin_function(uint8_t pin, pin_func_t function)
{
	volatile uint32_t* gpio_ctrl;
	gpio_ctrl = reinterpret_cast<uint32_t*>(custom_IO_BANK0_BASE + custom_IO_BANK0_GPIO_CTRL_OFFSET + (pin*2*custom_IO_BANK0_GPIO_CTRL_OFFSET));	//IO_BANK0: 2.19.6.1 pg: 244
	*gpio_ctrl = (*gpio_ctrl & ~NONE) | function;	//Set function to pin
    return;
}

void custom_set_pin_pullup(uint8_t pin)
{
	volatile uint32_t* pad_ctrl;
	pad_ctrl = reinterpret_cast<uint32_t*>(custom_PADS_BANK0_BASE + (pin + 1)*custom_PADS_BANK0_GPIO_OFFSET);	//PADS_BANK0: 2.19.6.3 pg: 302
	*pad_ctrl = (*pad_ctrl & ~0x18) | (1<<4);
    return;
}

void custom_set_pin_dir(uint8_t pin, pin_dir_t dir)
{
    volatile uint32_t* gpio_oe;
    if (dir == PIN_OUT) {
        gpio_oe = reinterpret_cast<uint32_t*>(custom_SIO_BASE + custom_SIO_GPIO_OE_SET_OFFSET);
        *gpio_oe = (1 << pin);	//Set pin to output
    } else if (dir == PIN_IN) {
        gpio_oe = reinterpret_cast<uint32_t*>(custom_SIO_BASE + custom_SIO_GPIO_OE_CLR_OFFSET);
        *gpio_oe &= ~(1 << pin);	//Set pin to input
    }
    else {
        printf("...\nInvalid direction!\n");
    }
    return;
}

/*! \brief Drive a single GPIO high/low
 *
 * \param pin GPIO number
 * \param value If false clear the GPIO, otherwise set it.
 */
void custom_set_pin_output(uint8_t pin, bool value)
{
    volatile uint32_t* gpio_out;
    if (value) {
        gpio_out = reinterpret_cast<uint32_t*>(custom_SIO_BASE + custom_SIO_GPIO_OUT_SET_OFFSET);
        *gpio_out = (1 << pin);	//Set pin to high
    } else {
        gpio_out = reinterpret_cast<uint32_t*>(custom_SIO_BASE + custom_SIO_GPIO_OUT_CLR_OFFSET);
        *gpio_out = (1 << pin);	//Set pin to low
    }
    return;
}