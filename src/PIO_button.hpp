#ifndef PIO_BUTTON_HPP
#define PIO_BUTTON_HPP

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"

// Include the generated header from our PIO program
#include "read_pin.pio.h"

/**
 * @brief Initialize a button with PIO handling
 * 
 * @param pio The PIO instance (pio0 or pio1)
 * @param btn_pin The GPIO pin connected to the button
 */
void init_pio_button(PIO pio, uint8_t btn_pin);

/**
 * @brief Check if the button was pressed since the last call
 * 
 * @param pio The PIO instance used for this button
 * @return true if button was pressed, false otherwise
 */
bool was_button_pressed(PIO pio);

#endif // PIO_BUTTON_HPP
