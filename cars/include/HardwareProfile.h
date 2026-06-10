#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include <stdint.h>
#include <stdlib.h>
#define BONNET_ADAFRUIT_ST7789
#define USE_BUTTONS

#ifdef BONNET_ADAFRUIT_ST7789
	#define PIN_MOSI   10
	#define PIN_SCLK   11
	#define PIN_DC   25
	#define PIN_RST    24
	#define PIN_BL     26
	#ifdef USE_BUTTONS
	#define BTN_LEFT_PIN    27
	#define BTN_RIGHT_PIN   23
	#define BTN_UP_PIN      17
	#define BTN_DOWN_PIN    22
	#endif
#else
	#define PIN_MOSI   10
	#define PIN_SCLK   11
	#define PIN_DC     21
	#define PIN_RST    16
	#define PIN_BL     20
	#ifdef USE_BUTTONS
	#define BTN_LEFT_PIN     5
	#define BTN_RIGHT_PIN   27
	#define BTN_UP_PIN      22
	#define BTN_DOWN_PIN    17
	#endif
#endif

#define PIN_SOUND  13

#define SPI_DEVICE   "/dev/spidev0.0"
#define SPI_SPEED_HZ  40000000
#define GPIO_CHIP    "/dev/gpiochip0"

#define TFT_W       240
#define TFT_H       240

#define MAZE_COLS   9
#define MAZE_ROWS   9
#define MAZE_W      MAZE_COLS
#define MAZE_H      MAZE_ROWS
#define CELL_W      24
#define CELL_H      24
#define MAZE_OX     12
#define MAZE_OY     12
#define WALL_THICK  4
#define PACMAN_R    9
#define PACMAN_SIZE 18
#define GHOST_SIZE  18

#define HUD_Y       (MAZE_OY + MAZE_ROWS*CELL_H + 4)
#define HUD_H       (TFT_H - HUD_Y)

#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define YELLOW      0xFFE0
#define ORANGE      0xFC00
#define MAGENTA     0xF81F
#define GRAY        0x8410
#define DARK_BLUE   0x000C
#define PINK        0xF81F
#define COLOR565(r,g,b) ((uint16_t)((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|((b)>>3)))

#define WALL_COLOR   COLOR565(0,0,140)
#define WALL_EDGE    COLOR565(30,80,255)
#define FLOOR_COLOR  BLACK
#define DOT_COLOR    COLOR565(255,220,100)
#define POWER_COLOR  WHITE

void gpio_write(int pin, int val);
int  gpio_read (int pin);

#define DC_HIGH()    gpio_write(PIN_DC,  1)
#define DC_LOW()     gpio_write(PIN_DC,  0)
#define RST_HIGH()   gpio_write(PIN_RST, 1)
#define RST_LOW()    gpio_write(PIN_RST, 0)
#define BL_HIGH()    gpio_write(PIN_BL,  1)
#define BL_LOW()     gpio_write(PIN_BL,  0)
#define SOUND_ON()   ((void)0)
#define SOUND_OFF()  ((void)0)

#ifdef USE_BUTTONS
#define BTN_UP    (!gpio_read(BTN_UP_PIN))
#define BTN_DOWN  (!gpio_read(BTN_DOWN_PIN))
#define BTN_LEFT  (!gpio_read(BTN_LEFT_PIN))
#define BTN_RIGHT (!gpio_read(BTN_RIGHT_PIN))
#endif

int  hw_init(void);
void hw_close(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
void spi_write_byte(uint8_t d);
void spi_write_buf (const uint8_t *buf, uint32_t len);
void write_cmd  (uint8_t c);
void write_data (uint8_t d);
void push_color (uint16_t color);
void push_color_n(uint16_t color, uint32_t n);
void reset_display(void);
void init_display (void);
void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#endif
