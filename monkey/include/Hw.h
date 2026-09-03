#pragma once
#include "HardwareProfile.h"
#include <cstdint>

// ============================================================
//  Hw - Capa de bajo nivel GPIO + SPI vía ioctl del kernel
//
//  Patrón idéntico a pacman/cars (que funcionan en CM5 y Zero 2W):
//    * /dev/gpiochip0  -> ioctl GPIO_V1_GET_LINEHANDLE_IOCTL
//    * /dev/spidev0.0  -> ioctl SPI_IOC_MESSAGE
//  Sin librerías externas (sin pigpio, sin bcm2835).
// ============================================================

// Pines de salida agrupados en un solo handle: DC, RST, BL
#define HW_IDX_DC      0
#define HW_IDX_RST     1
#define HW_IDX_BL      2
#define HW_N_OUT_PINS  3

#define HW_DC_HIGH()  hw_gpio_write(PIN_DC, 1)
#define HW_DC_LOW()   hw_gpio_write(PIN_DC, 0)
#define HW_RST_HIGH() hw_gpio_write(PIN_RST, 1)
#define HW_RST_LOW()  hw_gpio_write(PIN_RST, 0)
#define HW_BL_HIGH()  hw_gpio_write(PIN_BL, 1)
#define HW_BL_LOW()   hw_gpio_write(PIN_BL, 0)

// ---- Inicialización / cierre ----
int  hw_init(void);
void hw_close(void);

// ---- GPIO ----
int  hw_gpio_write(int pin, int val);
int  hw_gpio_read(int pin);

// ---- SPI ----
void hw_spi_write_byte(uint8_t d);
void hw_spi_write_buf(const uint8_t* buf, uint32_t len);

// ---- Delay ----
void hw_delay_ms(uint32_t ms);
void hw_delay_us(uint32_t us);
