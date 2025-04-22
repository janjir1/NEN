#include "src/registers.hpp"
#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/i2c.h"

/*!
 * \brief Initialize I2C pins and set pull-up resistors
 *
 * \param i2c_type Pointer to the I2C instance (i2c0 or i2c1)
 * \param CLKspeed Clock speed in KHz
 * \param SDApin GPIO pin number for SDA
 * \param SCLKpin GPIO pin number for SCL
 */
void custom_i2c_init(i2c_inst_t* i2c_type, uint16_t CLKspeed, uint8_t  SDApin, uint8_t  SCLKpin)
{
	//IO_BANK0_GPIO12_CTRL_FUNCSEL_VALUE_I2C0_SDA
    custom_set_pin_function(SDApin, I2C);	//Set SDA pin to I2C function
    custom_set_pin_function(SCLKpin, I2C);	//Set SCL pin to I2C function

	// Enable pull-up
	custom_set_pin_pullup(SDApin);	//Enable pull-up on SDA pin
    custom_set_pin_pullup(SCLKpin);	//Enable pull-up on SCL pin

    // custom_reset_hw_block(i2c_type == i2c0 ? I2C0 : I2C1);	//Reset I2C0 block
    // custom_unreset_hw_block(i2c_type == i2c0 ? I2C0 : I2C1);	//Unreset I2C0 block



    return;
}

/*!
 * \brief Set the function of a GPIO pin
 *
 * \param pin GPIO number
 * \param function Function to set for the pin
 *
 * \note This function sets the function of a GPIO pin. The pin must be in the
 * GPIO bank and not in any other peripheral mode.
 */
void custom_set_pin_function(uint8_t pin, pin_func_t function)
{
	volatile uint32_t* gpio_ctrl;
	gpio_ctrl = reinterpret_cast<uint32_t*>(custom_IO_BANK0_BASE + custom_IO_BANK0_GPIO_CTRL_OFFSET + (pin*2*custom_IO_BANK0_GPIO_CTRL_OFFSET));	//IO_BANK0: 2.19.6.1 pg: 244
	*gpio_ctrl = (*gpio_ctrl & ~NONE) | function;	//Set function to pin
    return;
}

/*!
 * \brief Set the pull-up resistor for a GPIO pin
 *
 * \param pin GPIO number
 *
 * \note This function sets the pull-up resistor for a GPIO pin. The pin must be in the
 * GPIO bank and not in any other peripheral mode.
 */
void custom_set_pin_pullup(uint8_t pin)
{
    if (pin >= 0 && pin <= 29)
    {
        volatile uint32_t* pad_ctrl;
        pad_ctrl = reinterpret_cast<uint32_t*>(custom_PADS_BANK0_BASE + (pin + 1)*custom_PADS_BANK0_GPIO_OFFSET);	//PADS_BANK0: 2.19.6.3 pg: 302
    	*pad_ctrl = (*pad_ctrl & ~0x18) | (1<<4);
    } else
    {
        printf("...\nInvalid GPIO number!\n");
    }
    return;
}

/*!
 * \brief Set the direction of a GPIO pin
 *
 * \param pin GPIO number
 * \param dir Direction of the pin (input or output)
 *
 * \note This function sets the direction of a GPIO pin. The pin must be in the
 * GPIO bank and not in any other peripheral mode.
 */
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
    volatile uint32_t *gpio_out;
    if (value)
    {
        gpio_out = reinterpret_cast<uint32_t *>(custom_SIO_BASE + custom_SIO_GPIO_OUT_SET_OFFSET);
        *gpio_out = (1 << pin); // Set pin to high
    }
    else
    {
        gpio_out = reinterpret_cast<uint32_t *>(custom_SIO_BASE + custom_SIO_GPIO_OUT_CLR_OFFSET);
        *gpio_out = (1 << pin); // Set pin to low
    }
    return;
}

// void custom_reset_hw_block(reset_block_t block)
// {
//     volatile uint32_t *reset_block;
//     reset_block = reinterpret_cast<uint32_t *>(custom_RESETS_RESET_OFFSET); // RESETS:
//     if (block >= ADC && block <= USBCTRL)
//     {
//         *reset_block |= (1 << block); // Set reset bit for the specified block
//     }
//     else
//     {
//         printf("Invalid block number!\n");
//     }
//     return;
// }

// void custom_unreset_hw_block(reset_block_t block)
// {
//     volatile uint32_t *reset_block;
//     reset_block = reinterpret_cast<uint32_t *>(custom_RESETS_RESET_OFFSET + custom_REG_ALIAS_CLR_BITS); // RESETS:
//     if (block >= ADC && block <= USBCTRL)
//     {
//         *reset_block |= (1 << block); // Clear reset bit for the specified block
//     }
//     else
//     {
//         printf("Invalid block number!\n");
//     }
//     return;
// }