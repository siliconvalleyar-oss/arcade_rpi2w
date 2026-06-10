#include "../include/Renderer.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>    // usleep

// pigpio - disponible en Raspberry Pi
// sudo apt-get install libpigpio-dev
#include <pigpio.h>

// ============================================================
//  Renderer
// ============================================================

Renderer::~Renderer()
{
    shutdown();
}

bool Renderer::init()
{
    if (initialized_) return true;

    // ----- Init pigpio -----
    if (gpioInitialise() < 0) {
        fprintf(stderr, "[Renderer] gpioInitialise() fallido\n");
        return false;
    }

    // ----- Configurar pines -----
    gpioSetMode(PIN_DC,  PI_OUTPUT);
    gpioSetMode(PIN_RST, PI_OUTPUT);
    gpioSetMode(PIN_BL,  PI_OUTPUT);

    // Backlight off durante init
    gpioWrite(PIN_BL, 0);

    // ----- Reset HW del ST7789 -----
    gpioWrite(PIN_RST, 1);
    usleep(5000);
    gpioWrite(PIN_RST, 0);
    usleep(20000);
    gpioWrite(PIN_RST, 1);
    usleep(150000);

    // ----- Abrir SPI0 -----
    // spiOpen(channel, baudRate, spiFlags)
    // channel 0 = CE0 (GPIO8)
    // spiFlags = 0 = modo 0, active-low CS
    spi_handle_ = spiOpen(0, SPI_SPEED_HZ, 0);
    if (spi_handle_ < 0) {
        fprintf(stderr, "[Renderer] spiOpen() fallido: %d\n", spi_handle_);
        gpioTerminate();
        return false;
    }

    // ----- Secuencia de inicialización del ST7789 -----
    st7789Init();

    // ----- Encender backlight -----
    gpioWrite(PIN_BL, 1);

    send_buf_.reserve(SCREEN_W * 40 * 2);  // buffer para ~40 filas

    initialized_ = true;
    printf("[Renderer] ST7789 240x240 inicializado OK\n");
    return true;
}

void Renderer::shutdown()
{
    if (!initialized_) return;
    setBacklight(false);
    if (spi_handle_ >= 0) {
        spiClose(spi_handle_);
        spi_handle_ = -1;
    }
    gpioTerminate();
    initialized_ = false;
}

// ============================================================
//  Helpers SPI
// ============================================================
void Renderer::sendCmd(uint8_t cmd)
{
    gpioWrite(PIN_DC, 0);   // DC=0 → comando
    spiWrite(spi_handle_, reinterpret_cast<char*>(&cmd), 1);
}

void Renderer::sendData1(uint8_t d)
{
    gpioWrite(PIN_DC, 1);   // DC=1 → datos
    spiWrite(spi_handle_, reinterpret_cast<char*>(&d), 1);
}

void Renderer::sendData(const uint8_t* data, int len)
{
    if (len <= 0) return;
    gpioWrite(PIN_DC, 1);
    // pigpio spiWrite necesita char*, cast seguro
    spiWrite(spi_handle_,
             const_cast<char*>(reinterpret_cast<const char*>(data)),
             len);
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
//  Basada en Adafruit + waveshare + datasheets
// ============================================================
void Renderer::st7789Init()
{
    // Software reset
    sendCmd(ST7789Cmd::SWRESET);
    usleep(150000);

    // Sleep out
    sendCmd(ST7789Cmd::SLPOUT);
    usleep(10000);

    // Interface Pixel Format: 0x55 = RGB565 (16-bit)
    sendCmd(ST7789Cmd::COLMOD);
    sendData1(0x55);
    usleep(10000);

    // Memory Data Access Control
    // Ajustar según orientación deseada:
    // 0x00 = normal, 0x60 = rotate 90°, 0xC0 = 180°, 0xA0 = 270°
    sendCmd(ST7789Cmd::MADCTL);
    sendData1(0x00);

    // Display Inversion ON (necesario en muchos módulos ST7789)
    sendCmd(ST7789Cmd::INVON);
    usleep(10000);

    // Normal Display Mode On
    sendCmd(ST7789Cmd::NORON);
    usleep(10000);

    // Display On
    sendCmd(ST7789Cmd::DISPON);
    usleep(100000);

    // Limpiamos pantalla (evita garbage visual al encender)
    // Pintamos todo en negro
    setWindow(0, 0, SCREEN_W - 1, SCREEN_H - 1);
    // ST7789: 240*240*2 = 115200 bytes en negro (0x00)
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

    // Forzar todo dirty
    fb.clear(Colors::BLACK);   // esto marca todo dirty  - luego se sobreescribe
    // En realidad llamamos directamente:
    fb.prepareSendBuffer(send_buf_, 0, SCREEN_H);
    setWindow(0, 0, SCREEN_W - 1, SCREEN_H - 1);
    sendData(send_buf_.data(), static_cast<int>(send_buf_.size()));
    fb.commit();
}

void Renderer::setBacklight(bool on)
{
    if (initialized_) gpioWrite(PIN_BL, on ? 1 : 0);
}
