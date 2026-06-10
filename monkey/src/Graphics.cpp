#include "../include/Graphics.h"
#include "../include/fonts.h"
#include "../include/picopng.h"
#include <fstream>
#include <cstring>
#include <algorithm>

// ============================================================
//  SpriteData
// ============================================================
void SpriteData::fromRGBA(const uint8_t* rgba, int width, int height)
{
    w = width;
    h = height;
    const size_t total = static_cast<size_t>(w * h);
    pixels.resize(total);
    solid.resize(total);

    for (size_t i = 0; i < total; ++i) {
        const uint8_t r = rgba[i * 4 + 0];
        const uint8_t g = rgba[i * 4 + 1];
        const uint8_t b = rgba[i * 4 + 2];
        const uint8_t a = rgba[i * 4 + 3];
        pixels[i] = rgba8888_to_rgb565(r, g, b, a);
        solid[i]  = (a >= 128);
    }
}

// ============================================================
//  Framebuffer
// ============================================================
void Framebuffer::clear(Color16 c)
{
    back_.fill(c);
    dirtyRows_.fill(true);   // todo el frame es sucio tras clear
}

void Framebuffer::hline(int x, int y, int len, Color16 c)
{
    if (static_cast<unsigned>(y) >= static_cast<unsigned>(SCREEN_H)) return;
    const int x0 = std::max(x, 0);
    const int x1 = std::min(x + len, SCREEN_W);
    if (x0 >= x1) return;

    Color16* row = back_.data() + y * SCREEN_W;
    for (int i = x0; i < x1; ++i) row[i] = c;
    dirtyRows_[y] = true;
}

void Framebuffer::vline(int x, int y, int len, Color16 c)
{
    if (static_cast<unsigned>(x) >= static_cast<unsigned>(SCREEN_W)) return;
    const int y0 = std::max(y, 0);
    const int y1 = std::min(y + len, SCREEN_H);
    for (int j = y0; j < y1; ++j) {
        back_[j * SCREEN_W + x] = c;
        dirtyRows_[j] = true;
    }
}

void Framebuffer::fillRect(int x, int y, int w, int h, Color16 c)
{
    const int x0 = std::max(x, 0);
    const int y0 = std::max(y, 0);
    const int x1 = std::min(x + w, SCREEN_W);
    const int y1 = std::min(y + h, SCREEN_H);
    if (x0 >= x1 || y0 >= y1) return;

    for (int j = y0; j < y1; ++j) {
        Color16* row = back_.data() + j * SCREEN_W;
        for (int i = x0; i < x1; ++i) row[i] = c;
        dirtyRows_[j] = true;
    }
}

void Framebuffer::drawRect(int x, int y, int w, int h, Color16 c)
{
    hline(x, y,         w, c);
    hline(x, y + h - 1, w, c);
    vline(x,         y, h, c);
    vline(x + w - 1, y, h, c);
}

// ---- Blit sprite con transparencia ----
void Framebuffer::blitSprite(const SpriteData& spr, int dx, int dy)
{
    if (!spr.valid()) return;

    const int x0 = std::max(dx, 0);
    const int y0 = std::max(dy, 0);
    const int x1 = std::min(dx + spr.w, SCREEN_W);
    const int y1 = std::min(dy + spr.h, SCREEN_H);
    if (x0 >= x1 || y0 >= y1) return;

    for (int sy = y0 - dy, dy2 = y0; dy2 < y1; ++sy, ++dy2) {
        const Color16* srcRow = spr.pixels.data() + sy * spr.w;
        Color16*       dstRow = back_.data()       + dy2 * SCREEN_W;
        bool rowChanged = false;

        for (int sx = x0 - dx, dx2 = x0; dx2 < x1; ++sx, ++dx2) {
            const Color16 c = srcRow[sx];
            if (c != Colors::TRANSPARENT) {
                dstRow[dx2] = c;
                rowChanged = true;
            }
        }
        if (rowChanged) dirtyRows_[dy2] = true;
    }
}

// ---- Blit con flip horizontal ----
void Framebuffer::blitSpriteFlipH(const SpriteData& spr, int dx, int dy)
{
    if (!spr.valid()) return;

    const int x0 = std::max(dx, 0);
    const int y0 = std::max(dy, 0);
    const int x1 = std::min(dx + spr.w, SCREEN_W);
    const int y1 = std::min(dy + spr.h, SCREEN_H);
    if (x0 >= x1 || y0 >= y1) return;

    for (int sy = y0 - dy, dy2 = y0; dy2 < y1; ++sy, ++dy2) {
        const Color16* srcRow = spr.pixels.data() + sy * spr.w;
        Color16*       dstRow = back_.data()       + dy2 * SCREEN_W;
        bool rowChanged = false;

        for (int sx_flipped = spr.w - 1 - (x0 - dx), dx2 = x0;
             dx2 < x1;
             --sx_flipped, ++dx2)
        {
            const Color16 c = srcRow[sx_flipped];
            if (c != Colors::TRANSPARENT) {
                dstRow[dx2] = c;
                rowChanged = true;
            }
        }
        if (rowChanged) dirtyRows_[dy2] = true;
    }
}

// ---- Blit con color de fondo en transparentes ----
void Framebuffer::blitSpriteWithBg(const SpriteData& spr, int dx, int dy,
                                    Color16 bg_color)
{
    if (!spr.valid()) return;

    const int x0 = std::max(dx, 0);
    const int y0 = std::max(dy, 0);
    const int x1 = std::min(dx + spr.w, SCREEN_W);
    const int y1 = std::min(dy + spr.h, SCREEN_H);
    if (x0 >= x1 || y0 >= y1) return;

    for (int sy = y0 - dy, dy2 = y0; dy2 < y1; ++sy, ++dy2) {
        const Color16* srcRow = spr.pixels.data() + sy * spr.w;
        Color16*       dstRow = back_.data()       + dy2 * SCREEN_W;

        for (int sx = x0 - dx, dx2 = x0; dx2 < x1; ++sx, ++dx2) {
            const Color16 c = srcRow[sx];
            dstRow[dx2] = (c != Colors::TRANSPARENT) ? c : bg_color;
        }
        dirtyRows_[dy2] = true;
    }
}

// ============================================================
//  Fuente 5x7
// ============================================================
void Framebuffer::drawChar(int x, int y, char ch, Color16 fg, Color16 bg)
{
    const int idx = static_cast<int>(ch) - FONT_FIRST;
    if (idx < 0 || idx >= FONT_COUNT) return;
    const uint8_t* col = FONT_DATA[idx];

    for (int cx = 0; cx < FONT_W; ++cx) {
        uint8_t colData = col[cx];
        for (int cy = 0; cy < FONT_H; ++cy) {
            if (colData & (1 << cy)) {
                setPixel(x + cx, y + cy, fg);
            } else if (bg != Colors::TRANSPARENT) {
                setPixel(x + cx, y + cy, bg);
            }
        }
    }
}

void Framebuffer::drawText(int x, int y, const char* txt,
                            Color16 fg, Color16 bg, int scale)
{
    if (!txt) return;
    int cx = x;
    while (*txt) {
        if (scale == 1) {
            drawChar(cx, y, *txt, fg, bg);
            cx += FONT_W + 1;
        } else {
            // Dibujamos cada pixel escalado
            const int idx = static_cast<int>(*txt) - FONT_FIRST;
            if (idx >= 0 && idx < FONT_COUNT) {
                const uint8_t* col = FONT_DATA[idx];
                for (int qx = 0; qx < FONT_W; ++qx) {
                    for (int qy = 0; qy < FONT_H; ++qy) {
                        Color16 c = (col[qx] & (1 << qy)) ? fg : bg;
                        if (c != Colors::TRANSPARENT)
                            fillRect(cx + qx*scale, y + qy*scale,
                                     scale, scale, c);
                    }
                }
            }
            cx += (FONT_W + 1) * scale;
        }
        ++txt;
    }
}

// ============================================================
//  Dirty-range helpers
// ============================================================
std::vector<Framebuffer::DirtyRange> Framebuffer::getDirtyRanges() const
{
    std::vector<DirtyRange> ranges;
    ranges.reserve(16);

    int i = 0;
    while (i < SCREEN_H) {
        if (!dirtyRows_[i]) { ++i; continue; }
        // Inicio de rango sucio
        int start = i;
        while (i < SCREEN_H && dirtyRows_[i]) ++i;
        ranges.push_back({start, i});   // [start, i)
    }
    return ranges;
}

void Framebuffer::commit()
{
    // Copiar solo las filas sucias de back -> front
    for (int y = 0; y < SCREEN_H; ++y) {
        if (dirtyRows_[y]) {
            const Color16* src = back_.data()  + y * SCREEN_W;
                  Color16* dst = front_.data() + y * SCREEN_W;
            std::memcpy(dst, src, SCREEN_W * sizeof(Color16));
        }
    }
    dirtyRows_.fill(false);
}

// ---- Preparar buffer big-endian para SPI ----
void Framebuffer::prepareSendBuffer(std::vector<uint8_t>& out,
                                     int startRow, int endRow) const
{
    const int numRows  = endRow - startRow;
    const int numPixels= numRows * SCREEN_W;
    out.resize(static_cast<size_t>(numPixels) * 2);

    const Color16* src = back_.data() + startRow * SCREEN_W;
    uint8_t*       dst = out.data();

    // ST7789 espera big-endian: byte alto primero
    for (int i = 0; i < numPixels; ++i) {
        const Color16 p = src[i];
        dst[i * 2]     = static_cast<uint8_t>(p >> 8);
        dst[i * 2 + 1] = static_cast<uint8_t>(p & 0xFF);
    }
}
