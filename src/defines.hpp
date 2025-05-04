#pragma once

// ================= Includes =====================


// ================= Defines =====================
// GPIO Defines
#define LED_PIN 25
#define BTN_PIN 24
#define NEOPIXEL_pin 23

// SPI Settings
#define MY_SPI_PORT spi0
#define MY_SPI_BAUDRATE 1000000 // 1MHz

// SPI Pin Definitions
#define MY_PIN_MISO  0
#define MY_PIN_CS    1
#define MY_PIN_SCK   2
#define MY_PIN_MOSI  3

// SH1107 Defines
#define myOLEDwidth  128
#define myOLEDheight 128
#define FULLSCREEN (myOLEDwidth * myOLEDheight)
//extern uint8_t screenBuffer[FULLSCREEN/8];

// I2C Settings
#define I2C_PORT i2c1
#define I2C_ADDR        0x3C    // I2C address
#define I2C_SPEED       400     // I2C clock speed in kilo hertz
#define I2C_CLK_PIN     15
#define I2C_DATA_PIN    14
#define I2C_RESET_PIN   (-1)      // set to -1 if not used

//main
#define POLL_PERIOD_MS 100
#define WRITE_ERROR_MAX 18