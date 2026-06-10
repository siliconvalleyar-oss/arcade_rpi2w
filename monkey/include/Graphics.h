#pragma once
#include "Types.h"
#include <array>
#include <vector>
#include <cstring>
#include <algorithm>
#include <span>          // C++20

// ============================================================
//  Sprite cargado desde PNG (picoPNG -> RGB565)
//  Los píxeles transparentes se marcan con Colors::TRANSPARENT
// ============================================================
struct SpriteData {
    int w{0}, h{0};
    std::vector<Color16> pixels;   // RGB565, TRANSPARENT donde alpha<128
    std::vector<bool>    solid;    // máscara de colisión (alpha > 0)

    bool valid() const { return w > 0 && h > 0 && !pixels.empty(); }

    // Cargar desde buffer RGBA8888 ya decodificado por picoPNG
    void fromRGBA(const uint8_t* rgba, int width, int height);

    // Acceso seguro
    Color16 pixel(int x, int y) const {
        if (x < 0 || x >= w || y < 0 || y >= h) return Colors::TRANSPARENT;
        return pixels[static_cast<size_t>(y * w + x)];
    }
    bool isSolid(int x, int y) const {
        if (x < 0 || x >= w || y < 0 || y >= h) return false;
        return solid[static_cast<size_t>(y * w + x)];
    }
};

// Frame de animación: índice al SpriteData en el atlas + offset
struct AnimFrame {
    int spriteIdx{0};
    int offsetX{0}, offsetY{0};
    int durationMs{100};
};

// ============================================================
//  Framebuffer - doble buffer + dirty-row tracking
//
//  Estrategia de actualización parcial:
//    - back  : donde dibujamos el frame actual
//    - front : lo que está actualmente en la pantalla
//    - dirtyRows[y] = true  si la fila y difiere entre back y front
//    - Al volcar al display, solo se envían las filas sucias
//    - Usando CASET/RASET del ST7789 para ventana de escritura
//
//  Para maximizar la eficiencia, se agrupan filas contiguas
//  en un solo RASET+RAMWR en lugar de una transferencia por fila.
// ============================================================
class Framebuffer {
public:
    // Limpiar con color
    void clear(Color16 c = Colors::BLACK);

    // Dibujar píxel (bounds-checked)
    inline void setPixel(int x, int y, Color16 c) noexcept {
        if (static_cast<unsigned>(x) >= static_cast<unsigned>(SCREEN_W) ||
            static_cast<unsigned>(y) >= static_cast<unsigned>(SCREEN_H))
            return;
        const int idx = y * SCREEN_W + x;
        back_[idx] = c;
        dirtyRows_[y] = true;
    }

    // Dibujar rectángulo sólido
    void fillRect(int x, int y, int w, int h, Color16 c);

    // Dibujar rectángulo outline
    void drawRect(int x, int y, int w, int h, Color16 c);

    // Dibujar línea horizontal (muy frecuente)
    void hline(int x, int y, int len, Color16 c);

    // Dibujar línea vertical
    void vline(int x, int y, int len, Color16 c);

    // Blit sprite con transparencia (TRANSPARENT se salta)
    // bg: color de fondo para mezclar (puede ser Colors::BLACK u otro)
    void blitSprite(const SpriteData& spr, int dx, int dy);

    // Blit sprite con flip horizontal
    void blitSpriteFlipH(const SpriteData& spr, int dx, int dy);

    // Blit suavizado: donde hay TRANSPARENT, pinta bg_color
    // (para borrar el sprite del frame anterior)
    void blitSpriteWithBg(const SpriteData& spr, int dx, int dy,
                          Color16 bg_color);

    // Texto pequeño (font 5x7 integrado)
    void drawChar(int x, int y, char c, Color16 fg, Color16 bg = Colors::TRANSPARENT);
    void drawText(int x, int y, const char* txt, Color16 fg,
                  Color16 bg = Colors::TRANSPARENT, int scale = 1);

    // ---- Acceso para el renderer ----
    // Retorna las filas sucias como rangos [start, end) para envío eficiente
    struct DirtyRange { int startRow; int endRow; };
    std::vector<DirtyRange> getDirtyRanges() const;

    // Copiar back->front y limpiar dirty flags
    void commit();

    // Acceso directo al buffer back (para SPI DMA)
    const Color16* backPtr()  const { return back_.data(); }
          Color16* backPtr()        { return back_.data(); }
    const Color16* frontPtr() const { return front_.data(); }

    // ---- Para el diff de pantalla: buffer completo big-endian
    // El ST7789 espera byte alto primero en RGB565
    void prepareSendBuffer(std::vector<uint8_t>& out,
                           int startRow, int endRow) const;

private:
    std::array<Color16, SCREEN_PIXELS> back_{};
    std::array<Color16, SCREEN_PIXELS> front_{};
    std::array<bool, SCREEN_H>         dirtyRows_{};
};
