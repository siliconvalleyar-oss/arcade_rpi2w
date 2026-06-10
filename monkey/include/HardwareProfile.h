#pragma once
#include <cstdint>

#define BONNET_ADAFRUIT_ST7789

#ifdef BONNET_ADAFRUIT_ST7789
	#define PIN_MOSI   10
	#define PIN_SCLK   11
	#define PIN_DC   25
	#define PIN_RST    24
	#define PIN_BL     26
	#define PIN_SOUND  13
	#ifdef JOYSTICK_ENABLED
	#define PIN_JOY_UP     17
	#define PIN_JOY_DOWN  22
	#define PIN_JOY_LEFT  27
	#define PIN_JOY_RIGHT 23
	#define PIN_JOY_BTN_A  5
	#define PIN_JOY_BTN_B  6
	#endif
#else
	#define PIN_MOSI   10
	#define PIN_SCLK   11
	#define PIN_DC     21
	#define PIN_RST    16
	#define PIN_BL     20
	#define PIN_SOUND  13
	#ifdef JOYSTICK_ENABLED
	#define PIN_JOY_UP     17
	#define PIN_JOY_DOWN  27
	#define PIN_JOY_LEFT  22
	#define PIN_JOY_RIGHT 23
	#define PIN_JOY_BTN_A  5
	#define PIN_JOY_BTN_B  6
	#endif
#endif

#define JOYSTICK_ENABLED

#define SPI_SPEED_HZ    31250000
#define TARGET_FPS      30
#define FRAME_TIME_US   (1000000 / TARGET_FPS)

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
    inline constexpr uint8_t CASET    = 0x2A;
    inline constexpr uint8_t RASET    = 0x2B;
    inline constexpr uint8_t RAMWR    = 0x2C;
    inline constexpr uint8_t COLMOD   = 0x3A;
    inline constexpr uint8_t MADCTL   = 0x36;
}
