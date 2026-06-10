#pragma once
#include "HardwareProfile.h"
#include "Graphics.h"
#include <cstdint>
#include <vector>

// ============================================================
//  Renderer - maneja el display ST7789 vía pigpio SPI
//
//  Optimización de refresco parcial:
//   1. Framebuffer::getDirtyRanges() devuelve rangos de filas sucias
//   2. Por cada rango [r0, r1] se envía:
//        RASET(r0, r1-1)  + RAMWR + datos big-endian
//   3. Filas no sucias = 0 bytes enviados = cero parpadeo
//
//  En una escena de plataformas típica solo cambia ~30% de
//  las filas por frame, reduciendo la carga SPI a ~1/3.
// ============================================================
class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    // Inicializar pigpio + SPI + secuencia de reset del ST7789
    bool init();
    void shutdown();

    // Volcar framebuffer al display (solo filas sucias)
    void present(Framebuffer& fb);

    // Volcar frame completo (útil en primer frame / cambio de nivel)
    void presentFull(Framebuffer& fb);

    // Encender/apagar backlight
    void setBacklight(bool on);

    bool initialized() const { return initialized_; }

private:
    void sendCmd(uint8_t cmd);
    void sendData(const uint8_t* data, int len);
    void sendData1(uint8_t d);
    void setWindow(int x0, int y0, int x1, int y1);
    void st7789Init();

    int  spi_handle_{-1};
    bool initialized_{false};

    // Buffer temporal para conversión big-endian
    std::vector<uint8_t> send_buf_;
};
