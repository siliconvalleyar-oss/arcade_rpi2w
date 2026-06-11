#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define BONNET_ADAFRUIT_ST7789
#define USE_BUTTONS

#ifdef BONNET_ADAFRUIT_ST7789
  #define PIN_MOSI   10
  #define PIN_SCLK   11
  #define PIN_DC   25
  #define PIN_RST    24
  #define PIN_BL     26
  #ifdef USE_BUTTONS
  #define BTN_START_PIN    5
  #define BTN_FIRE_PIN  6
  #define BTN_LEFT_PIN  27
  #define BTN_RIGHT_PIN 23
  #endif
#else
  #define PIN_MOSI   10
  #define PIN_SCLK   11
  #define PIN_DC     21
  #define PIN_RST    16
  #define PIN_BL     20
  #ifdef USE_BUTTONS
  #define BTN_START_PIN    5
  #define BTN_FIRE_PIN 27
  #define BTN_LEFT_PIN  22
  #define BTN_RIGHT_PIN 17
  #endif
#endif

#define PIN_SOUND  13
#define SPI_DEVICE   "/dev/spidev0.0"
#define SPI_SPEED_HZ  40000000
#define GPIO_CHIP    "/dev/gpiochip0"

#define TFT_W       240
#define TFT_H       240

#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define YELLOW      0xFFE0
#define GRAY        0x8410
#define DARK_GRAY   0x4208
#define LIGHT_GRAY  0xC618

#define COLOR565(r,g,b) ((uint16_t)((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|((b)>>3)))

// Game constants
#define GROUND_Y        200
#define DINO_X          45
#define GRAVITY         1
#define JUMP_VEL        (-13)
#define GAME_SPEED_INIT 6
#define MAX_CACTUS      3

void gpio_write(int pin, int val);
int  gpio_read(int pin);

#define DC_HIGH()    gpio_write(PIN_DC, 1)
#define DC_LOW()     gpio_write(PIN_DC, 0)
#define RST_HIGH()   gpio_write(PIN_RST, 1)
#define RST_LOW()    gpio_write(PIN_RST, 0)
#define BL_HIGH()    gpio_write(PIN_BL, 1)
#define BL_LOW()     gpio_write(PIN_BL, 0)

#define BTN_JUMP()   (gpio_read(BTN_FIRE_PIN) == 0)
#define BTN_FIRE()   (gpio_read(BTN_FIRE_PIN) == 0)
#define BTN_LEFT()   (gpio_read(BTN_LEFT_PIN) == 0)
#define BTN_RIGHT()  (gpio_read(BTN_RIGHT_PIN) == 0)

int  hw_init(void);
void hw_close(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
void spi_write_byte(uint8_t d);
void spi_write_buf(const uint8_t *buf, uint32_t len);
void write_cmd(uint8_t c);
void write_data(uint8_t d);
void push_color(uint16_t color);
void push_color_n(uint16_t color, uint32_t n);
void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void init_display(void);
void sig_handler(int s);

#endif
