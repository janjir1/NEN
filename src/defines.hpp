#pragma once

// =============== Defines =========================
// GPIO Defines
#define LED_pin 25
#define BTN_pin 24
#define NEOPIXEL_pin 23

// SPI Settings
#define SPI_PORT spi0
#define SPI_BAUDRATE 1000000 // 1MHz

#define PIN_MISO  0
#define PIN_CS    1
#define PIN_SCK   2
#define PIN_MOSI  3

// SH1107 Defines
#define myOLEDwidth  128
#define myOLEDheight 128
#define FULLSCREEN (myOLEDwidth * myOLEDheight)
extern uint8_t screenBuffer[FULLSCREEN/8];

// I2C Settings
#define I2C_PORT i2c1
#define I2C_ADDR        0x3C    // I2C address
#define I2C_SPEED       400     // I2C clock speed in kilo hertz
#define I2C_CLK_PIN     15
#define I2C_DATA_PIN    14
#define I2C_RESET_PIN   -1      // set to -1 if not used