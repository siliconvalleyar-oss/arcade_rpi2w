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
//GPIO26 - Used to turn the backlight on and off.
//SCK, MOSI, CE0 & GPIO25 - These are the display control pins. Note that MISO is not connected even though it is a SPI >
//GPIO5 & GPIO6

#ifdef BONNET_ADAFRUIT_ST7789
// ==================== PINES BCM ====================
        #define PIN_MOSI   10           // SPI0 MOSI
        #define PIN_SCLK   11           // SPI0 SCLK
        #define PIN_DC   25             // D/C
        #define PIN_RST    24//16//24   // RESET
        #define PIN_BL     26//20//23   // Backlight
// Botones (opciona)
        #ifdef USE_BUTTONS
        #define BTN_START_PIN    5//
        #define BTN_FIRE_PIN  6//
        #define BTN_LEFT_PIN  27//
        #define BTN_RIGHT_PIN 23//
        #endif
// ==================== otro hardware  ====================
#else
// ==================== PINES BCM ====================
        #define PIN_MOSI   10   // SPI0 MOSI  (pin físico 19)
        #define PIN_SCLK   11   // SPI0 SCLK  (pin físico 23)
        #define PIN_DC     21   // D/C        (pin físico 22)
        #define PIN_RST    16   // RESET      (pin físico 18)
        #define PIN_BL     20   // Backlight
// Botones (opcional)
        #ifdef USE_BUTTONS
        #define BTN_START_PIN    5
        #define BTN_FIRE_PIN 27
        #define BTN_LEFT_PIN  22
        #define BTN_RIGHT_PIN 17
        #endif
#endif

#define PIN_SOUND  13


/*
// PINES BCM
#define PIN_MOSI   10
#define PIN_SCLK   11
#define PIN_DC     21
#define PIN_RST    16
#define PIN_BL     20
*/
#define PIN_CS     8

//#define PIN_SOUND  13
#define PIN_LED    26

// BOTONES (activos LOW, usar pull-up externo de 10k a 3.3V)
//#define BTN_LEFT_PIN    17
//#define BTN_RIGHT_PIN   27
//#define BTN_FIRE_PIN    22

// SPI
#define SPI_DEVICE   "/dev/spidev0.0"
#define SPI_SPEED_HZ  20000000   // 20 MHz estable
#define GPIO_SYSFS   "/sys/class/gpio"

// PANTALLA
#define TFT_W       240
#define TFT_H       240

// CONFIGURACIÓN DEL JUEGO
#define PLAYER_WIDTH    16
#define PLAYER_HEIGHT   12
#define INVADER_WIDTH   16
#define INVADER_HEIGHT  12
#define BULLET_WIDTH    3
#define BULLET_HEIGHT   6
#define NUM_INVADERS_X  8
#define NUM_INVADERS_Y  5
#define PLAYER_Y        (TFT_H - 30)
#define PLAYER_SPEED    5
#define BULLET_SPEED    6
#define INVADER_SPEED   1

// COLORES RGB565
#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define YELLOW      0xFFE0
#define LIME        0xAFE0
#define DARK_GREEN  0x0320

#define INVADER_COLOR1  LIME
#define INVADER_COLOR2  GREEN
#define INVADER_COLOR3  CYAN
#define PLAYER_COLOR    WHITE
#define BULLET_COLOR    RED
#define SHIELD_COLOR    DARK_GREEN

// PROTOTIPOS GPIO (sysfs)
void gpio_export(int pin);
void gpio_set_dir(int pin, const char* dir);
void gpio_write(int pin, int val);
int  gpio_read(int pin);

// MACROS PARA PANTALLA
#define DC_HIGH()    gpio_write(PIN_DC, 1)
#define DC_LOW()     gpio_write(PIN_DC, 0)
#define RST_HIGH()   gpio_write(PIN_RST, 1)
#define RST_LOW()    gpio_write(PIN_RST, 0)
#define BL_HIGH()    gpio_write(PIN_BL, 1)
#define BL_LOW()     gpio_write(PIN_BL, 0)
#define CS_HIGH()    gpio_write(PIN_CS, 1)
#define CS_LOW()     gpio_write(PIN_CS, 0)

// MACROS PARA BOTONES (activos LOW)
#define BTN_LEFT()   (gpio_read(BTN_LEFT_PIN) == 0)
#define BTN_RIGHT()  (gpio_read(BTN_RIGHT_PIN) == 0)
#define BTN_FIRE()   (gpio_read(BTN_FIRE_PIN) == 0)

// PROTOTIPOS HARDWARE
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

#endif
