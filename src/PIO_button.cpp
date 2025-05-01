#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "src/PIO_button.hpp"

// Include the generated header from our PIO program
#include "read_pin.pio.h"

void init_pio_button(PIO pio, uint8_t btn_pin){

	gpio_init(btn_pin);
    gpio_set_dir(btn_pin, GPIO_IN);
    gpio_pull_up(btn_pin);
    
    // Choose PIO instance and state machine

    
    // Add our program to the PIO's instruction memory
    uint offset = pio_add_program(pio, &read_pin_program);
    
    // Initialize the state machine with our program
    read_pin_program_init(pio, 0, offset, btn_pin);
    
}

bool was_button_pressed(PIO pio) {
    static bool last_state = true; // Assume button released initially (pull-up)
    bool pressed = false;

    // Process all available FIFO entries
    while (!pio_sm_is_rx_fifo_empty(pio, 0)) {
        uint32_t pin_state = pio_sm_get(pio, 0) & 0x01;
        
        // Detect transition from released (1) to pressed (0)
        if (last_state && !pin_state) {
            pressed = true;
        }
        
        // Update last known state
        last_state = pin_state;
    }

    return pressed;
}

