#include "../include/Graphics.h"
#include "../include/HardwareProfile.h"
#include "../include/picopng.h"
#include <fstream>
#include <vector>
#include <cstring>

void Graphics::fill_screen(uint16_t color) {
    set_window(0, 0, TFT_W-1, TFT_H-1);
    push_color_n(color, (uint32_t)TFT_W * TFT_H);
}

void Graphics::fill_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                          uint16_t color) {
    if(x>=(int16_t)TFT_W || y>=(int16_t)TFT_H || w<=0 || h<=0) return;
    if(x<0){ w+=x; x=0; }
    if(y<0){ h+=y; y=0; }
    if(x+w>(int16_t)TFT_W) w = TFT_W - x;
    if(y+h>(int16_t)TFT_H) h = TFT_H - y;
    set_window(x, y, x+w-1, y+h-1);
    push_color_n(color, (uint32_t)w * h);
}

void Graphics::draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if((uint16_t)x >= TFT_W || (uint16_t)y >= TFT_H) return;
    set_window(x, y, x, y);
    push_color(color);
}

void Graphics::draw_hline(int16_t x, int16_t y, int16_t len, uint16_t c) {
    fill_rect(x, y, len, 1, c);
}
void Graphics::draw_vline(int16_t x, int16_t y, int16_t len, uint16_t c) {
    fill_rect(x, y, 1, len, c);
}

static int16_t isqrt(int32_t v) {
    if(v <= 0) return 0;
    int16_t x = (int16_t)(v < 65536L ? (int16_t)v : 256);
    for(int i=0;i<8;i++) { int16_t nx=(int16_t)((x + v/x)>>1); if(nx>=x) break; x=nx; }
    while((int32_t)x*x > v) x--;
    return x;
}

void Graphics::fill_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    for(int16_t dy=-r; dy<=r; dy++) {
        int16_t dx = isqrt((int32_t)r*r - (int32_t)dy*dy);
        fill_rect(cx-dx, cy+dy, 2*dx+1, 1, color);
    }
}

void Graphics::draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    int16_t x=0, y=r, d=3-2*r;
    while(x<=y) {
        draw_pixel(cx+x,cy+y,color); draw_pixel(cx-x,cy+y,color);
        draw_pixel(cx+x,cy-y,color); draw_pixel(cx-x,cy-y,color);
        draw_pixel(cx+y,cy+x,color); draw_pixel(cx-y,cy+x,color);
        draw_pixel(cx+y,cy-x,color); draw_pixel(cx-y,cy-x,color);
        if(d<0) d+=4*x+6; else { d+=4*(x-y)+10; y--; }
        x++;
    }
}

void Graphics::draw_char(int16_t x, int16_t y, char c,
                          uint16_t fg, uint16_t bg, uint8_t scale) {
    if(c < 0x20 || c > 0x7E) c = ' ';
    uint8_t idx = c - 0x20;
    for(uint8_t col=0; col<5; col++) {
        uint8_t line = font5x7[idx][col];
        for(uint8_t row=0; row<7; row++) {
            fill_rect(x+col*scale, y+row*scale, scale, scale,
                      (line&(1<<row)) ? fg : bg);
        }
    }
}

void Graphics::draw_string(int16_t x, int16_t y, const char *s,
                             uint16_t fg, uint16_t bg, uint8_t scale) {
    while(*s) { draw_char(x, y, *s++, fg, bg, scale); x += 6*scale; }
}

void Graphics::draw_car(int16_t x, int16_t y, uint16_t color, int16_t size) {
    fill_rect(x, y, size, size, color);
    fill_rect(x + size/4, y, size/2, size/2, COLOR565(0,0,0));
    fill_rect(x, y + size*3/4, size/4, size/4, BLACK);
    fill_rect(x + size - size/4, y + size*3/4, size/4, size/4, BLACK);
    draw_pixel(x + size - 2, y + 2, WHITE);
    draw_pixel(x + 1, y + 2, WHITE);
}

void Graphics::drawSprite(int x, int y, const Sprite& sprite, uint16_t bgColor) {
    if (sprite.w == 0 || sprite.h == 0) return;

    int srcX = 0, srcY = 0;
    int drawW = sprite.w, drawH = sprite.h;

    if (x < 0) { srcX = -x; drawW += x; x = 0; }
    if (y < 0) { srcY = -y; drawH += y; y = 0; }
    if (x + drawW > (int)TFT_W) drawW = TFT_W - x;
    if (y + drawH > (int)TFT_H) drawH = TFT_H - y;
    if (drawW <= 0 || drawH <= 0) return;

    uint8_t bgR = ((bgColor >> 11) & 0x1F) * 255 / 31;
    uint8_t bgG = ((bgColor >> 5)  & 0x3F) * 255 / 63;
    uint8_t bgB = (bgColor & 0x1F) * 255 / 31;

    set_window(x, y, x + drawW - 1, y + drawH - 1);
    DC_HIGH();

    uint8_t rowBuf[480];
    for (int row = 0; row < drawH; ++row) {
        int srcRow = row + srcY;
        for (int col = 0; col < drawW; ++col) {
            uint32_t pixel = sprite.pixels[srcRow * sprite.w + (col + srcX)];
            uint8_t a = (pixel >> 24) & 0xFF;
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;

            uint16_t finalColor;
            if (a == 0) {
                finalColor = bgColor;
            } else if (a == 255) {
                finalColor = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            } else {
                uint8_t mixR = (r * a + bgR * (255 - a)) / 255;
                uint8_t mixG = (g * a + bgG * (255 - a)) / 255;
                uint8_t mixB = (b * a + bgB * (255 - a)) / 255;
                finalColor = ((mixR & 0xF8) << 8) | ((mixG & 0xFC) << 3) | (mixB >> 3);
            }
            rowBuf[col * 2 + 0] = finalColor >> 8;
            rowBuf[col * 2 + 1] = finalColor & 0xFF;
        }
        spi_write_buf(rowBuf, drawW * 2);
    }
}

Sprite Graphics::tintSprite(const Sprite& src, uint8_t rMult, uint8_t gMult, uint8_t bMult) {
    Sprite dst;
    dst.w = src.w;
    dst.h = src.h;
    dst.pixels.resize(src.pixels.size());
    dst.solid = src.solid;
    for (size_t i = 0; i < src.pixels.size(); ++i) {
        uint32_t p = src.pixels[i];
        uint8_t a = (p >> 24) & 0xFF;
        uint8_t r = (p >> 16) & 0xFF;
        uint8_t g = (p >> 8) & 0xFF;
        uint8_t b = p & 0xFF;
        r = (uint8_t)(((uint16_t)r * rMult) / 255);
        g = (uint8_t)(((uint16_t)g * gMult) / 255);
        b = (uint8_t)(((uint16_t)b * bMult) / 255);
        dst.pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    return dst;
}
