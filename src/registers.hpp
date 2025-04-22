#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"


#define custom_IO_BANK0_BASE 0x40014000
#define custom_IO_BANK0_GPIO_CTRL_OFFSET 0x004

#define custom_PADS_BANK0_BASE 0x4001C000
#define custom_PADS_BANK0_GPIO_OFFSET 0x04
#define PUE_val 1<<4

#define custom_SIO_BASE 0xd0000000
#define custom_SIO_GPIO_OE_SET_OFFSET 0x024
#define custom_SIO_GPIO_OE_CLR_OFFSET 0x028
#define custom_SIO_GPIO_OUT_SET_OFFSET 0x014
#define custom_SIO_GPIO_OUT_CLR_OFFSET 0x018

enum pin_func_t 
{
    JTAG,
    SPI,
    UART,
    I2C,
    PWM,
    SIO,
    PIO0,
    PIO1,
    CLOCK,
    USB,
    NONE = 0x1f
};

enum pin_dir_t
{
    PIN_OUT = 1u, ///< set GPIO to output
    PIN_IN = 0u,  ///< set GPIO to input
};

void custom_i2c_init(i2c_inst_t* i2c_type, uint16_t CLKspeed, uint8_t  SDApin, uint8_t  SCLKpin);
void custom_set_pin_function(uint8_t pin, pin_func_t function);
void custom_set_pin_pullup(uint8_t pin);
void custom_set_pin_dir(uint8_t pin, pin_dir_t dir);
void custom_set_pin_output(uint8_t pin, bool value);

/*
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_RESET  _u(0x1f)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_BITS   _u(0x0000001f)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_MSB    _u(4)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_LSB    _u(0)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_ACCESS "RW"
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_VALUE_JTAG_TMS _u(0x00)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_VALUE_SPI0_SS_N _u(0x01)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_VALUE_UART0_RX _u(0x02)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_VALUE_I2C0_SCL _u(0x03)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_VALUE_PWM_B_0 _u(0x04)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_VALUE_SIO_1 _u(0x05)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_VALUE_PIO0_1 _u(0x06)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_VALUE_PIO1_1 _u(0x07)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_VALUE_USB_MUXING_VBUS_DETECT _u(0x09)
#define IO_BANK0_GPIO1_CTRL_FUNCSEL_VALUE_NULL _u(0x1f)
*/