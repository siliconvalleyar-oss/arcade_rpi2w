#include "../include/Renderer.h"
#include "../include/Hw.h"
#include <cstdio>

// ============================================================
//  Renderer - ST7789 vía SPI ioctl (/dev/spidev0.0)
//  GPIO ioctl (/dev/gpiochip0) para DC/RST/BL
// ============================================================

Renderer::~Renderer()
{
    shutdown();
}

bool Renderer::init()
{
    if (initialized_) return true;

    // ----- Backlight off durante init -----
    HW_BL_LOW();

    // ----- Reset HW del ST7789 -----
    HW_RST_HIGH();
    hw_delay_us(5000);
    HW_RST_LOW();
    hw_delay_us(20000);
    HW_RST_HIGH();
    hw_delay_us(150000);

    // ----- Secuencia de inicialización del ST7789 -----
    st7789Init();

    // ----- Encender backlight -----
    HW_BL_HIGH();

    send_buf_.reserve(SCREEN_W * 40 * 2);  // buffer para ~40 filas

    initialized_ = true;
    printf("[Renderer] ST7789 240x240 inicializado OK\n");
    return true;
}

void Renderer::shutdown()
{
    if (!initialized_) return;
    setBacklight(false);
    initialized_ = false;
}

// ============================================================
//  Helpers SPI
// ============================================================
void Renderer::sendCmd(uint8_t cmd)
{
    HW_DC_LOW();    // DC=0 → comando
    hw_spi_write_byte(cmd);
}

void Renderer::sendData1(uint8_t d)
{
    HW_DC_HIGH();   // DC=1 → datos
    hw_spi_write_byte(d);
}

void Renderer::sendData(const uint8_t* data, int len)
{
    if (len <= 0) return;
    HW_DC_HIGH();
    hw_spi_write_buf(data, static_cast<uint32_t>(len));
}

// Ventana de escritura CASET/RASET
void Renderer::setWindow(int x0, int y0, int x1, int y1)
{
    // Column Address Set
    sendCmd(ST7789Cmd::CASET);
    uint8_t col[4] = {
        static_cast<uint8_t>(x0 >> 8), static_cast<uint8_t>(x0 & 0xFF),
        static_cast<uint8_t>(x1 >> 8), static_cast<uint8_t>(x1 & 0xFF)
    };
    sendData(col, 4);

    // Row Address Set
    sendCmd(ST7789Cmd::RASET);
    uint8_t row[4] = {
        static_cast<uint8_t>(y0 >> 8), static_cast<uint8_t>(y0 & 0xFF),
        static_cast<uint8_t>(y1 >> 8), static_cast<uint8_t>(y1 & 0xFF)
    };
    sendData(row, 4);

    // Memory Write
    sendCmd(ST7789Cmd::RAMWR);
}

// ============================================================
//  Secuencia de init del ST7789 240x240
// ============================================================
void Renderer::st7789Init()
{
    // Software reset
    sendCmd(ST7789Cmd::SWRESET);
    hw_delay_us(150000);

    // Sleep out
    sendCmd(ST7789Cmd::SLPOUT);
    hw_delay_us(10000);

    // Interface Pixel Format: 0x55 = RGB565 (16-bit)
    sendCmd(ST7789Cmd::COLMOD);
    sendData1(0x55);
    hw_delay_us(10000);

    // Memory Data Access Control
    sendCmd(ST7789Cmd::MADCTL);
    sendData1(0x00);

    // Display Inversion ON
    sendCmd(ST7789Cmd::INVON);
    hw_delay_us(10000);

    // Normal Display Mode On
    sendCmd(ST7789Cmd::NORON);
    hw_delay_us(10000);

    // Display On
    sendCmd(ST7789Cmd::DISPON);
    hw_delay_us(100000);

    // Limpiamos pantalla (evita garbage visual al encender)
    setWindow(0, 0, SCREEN_W - 1, SCREEN_H - 1);
    static constexpr int FILL_BYTES = SCREEN_W * SCREEN_H * 2;
    std::vector<uint8_t> zeros(FILL_BYTES, 0x00);
    sendData(zeros.data(), FILL_BYTES);
}

// ============================================================
//  present() - Volcado PARCIAL (solo filas sucias)
// ============================================================
void Renderer::present(Framebuffer& fb)
{
    if (!initialized_) return;

    auto ranges = fb.getDirtyRanges();
    if (ranges.empty()) {
        fb.commit();
        return;
    }

    for (const auto& [startRow, endRow] : ranges) {
        if (startRow >= endRow) continue;

        // Preparar datos big-endian
        fb.prepareSendBuffer(send_buf_, startRow, endRow);

        // Configurar ventana: columna 0..239, fila startRow..(endRow-1)
        setWindow(0, startRow, SCREEN_W - 1, endRow - 1);

        // Enviar píxeles
        sendData(send_buf_.data(), static_cast<int>(send_buf_.size()));
    }

    fb.commit();
}

// ============================================================
//  presentFull() - Volcado COMPLETO (primer frame / nivel nuevo)
// ============================================================
void Renderer::presentFull(Framebuffer& fb)
{
    if (!initialized_) return;

    fb.clear(Colors::BLACK);   // esto marca todo dirty - luego se sobreescribe
    fb.prepareSendBuffer(send_buf_, 0, SCREEN_H);
    setWindow(0, 0, SCREEN_W - 1, SCREEN_H - 1);
    sendData(send_buf_.data(), static_cast<int>(send_buf_.size()));
    fb.commit();
}

void Renderer::setBacklight(bool on)
{
    if (on) HW_BL_HIGH(); else HW_BL_LOW();
}
