#pragma once
#include <cstdint>

// ============================================================
//  Hardware Profile - Raspberry Pi Zero 2W
//  ST7789 240x240 por SPI0
// ============================================================
#define BONNET_ADAFRUIT_ST7789

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

// ==================== otro hardware  ====================
#else
// ==================== PINES BCM ====================
        #define PIN_MOSI   10   // SPI0 MOSI  (pin físico 19)
        #define PIN_SCLK   11   // SPI0 SCLK  (pin físico 23)
        #define PIN_DC     21   // D/C        (pin físico 22)
        #define PIN_RST    16   // RESET      (pin físico 18)
        #define PIN_BL     20   // Backlight
#endif

/*
// ---- SPI / Display ----------------------------------------
#define PIN_MOSI    10   // SPI0 MOSI  (pin físico 19)
#define PIN_SCLK    11   // SPI0 SCLK  (pin físico 23)
#define PIN_DC      21//25   // D/C        (pin físico 22)
#define PIN_RST     16//24   // RESET      (pin físico 18)
#define PIN_BL      20//26   // Backlight  (pin físico 16)
#define PIN_CS       8   // SPI0 CE0   (pin físico 24)
*/
// ---- Sonido (buzzer pasivo) --------------------------------
#define PIN_SOUND   13   // GPIO12 / PWM0  (pin físico 32)

// ---- Joystick / Botones -----------------------------------
//  COMENTADO - modo demo: el juego corre solo con IA básica
//  Descomentar y conectar para jugar manualmente
//
 #define JOYSTICK_ENABLED
 #define PIN_JOY_UP     17  // (pin físico 11)
 #define PIN_JOY_DOWN  22   // (pin físico 13)
 #define PIN_JOY_LEFT  27   // (pin físico 15)
 #define PIN_JOY_RIGHT 23   // (pin físico 16)
 #define PIN_JOY_BTN_A  5   // Saltar  (pin físico 29)
 #define PIN_JOY_BTN_B  6   // (pin físico 31)

// ---- SPI velocidad ----------------------------------------
// ST7789 soporta hasta 62.5 MHz, pero Pi limita a 31.25 o 62.5
// Para 240x240 a RGB565: 240*240*2 = 115200 bytes por frame
// A 31.25 MHz SPI teórico: ~270 fps máximo de bus
// Con overhead pigpio/DMA: ~60-80 fps real
#define SPI_SPEED_HZ    31250000   // 31.25 MHz

// ---- Framerate target -------------------------------------
#define TARGET_FPS      30
#define FRAME_TIME_US   (1000000 / TARGET_FPS)

// ---- Comandos ST7789 (1-byte) ----------------------------
namespace ST7789Cmd {
    inline constexpr uint8_t NOP      = 0x00;
    inline constexpr uint8_t SWRESET  = 0x01;
    inline constexpr uint8_t RDDID    = 0x04;
    inline constexpr uint8_t SLPIN    = 0x10;
    inline constexpr uint8_t SLPOUT   = 0x11;
    inline constexpr uint8_t NORON    = 0x13;
    inline constexpr uint8_t INVOFF   = 0x20;
    inline constexpr uint8_t INVON    = 0x21;
    inline constexpr uint8_t DISPOFF  = 0x28;
    inline constexpr uint8_t DISPON   = 0x29;
    inline constexpr uint8_t CASET    = 0x2A;  // Column Address Set
    inline constexpr uint8_t RASET    = 0x2B;  // Row Address Set
    inline constexpr uint8_t RAMWR    = 0x2C;  // Memory Write
    inline constexpr uint8_t COLMOD   = 0x3A;  // Interface Pixel Format
    inline constexpr uint8_t MADCTL   = 0x36;  // Memory Data Access Control
}
