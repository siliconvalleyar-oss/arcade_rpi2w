#include "Graphics.h"
#include "FrameBuffer.h"
#include <cstdio>

namespace Graphics {
    static FrameBuffer& fb = FrameBuffer::get();

    void fill_screen(uint16_t color) { fb.clear(color); }
    void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { fb.fillRect(x,y,w,h,color); }
    void draw_pixel(int16_t x, int16_t y, uint16_t color) { fb.setPixel(x,y,color); }
    void draw_hline(int16_t x, int16_t y, int16_t len, uint16_t color) { fill_rect(x,y,len,1,color); }
    void draw_vline(int16_t x, int16_t y, int16_t len, uint16_t color) { fill_rect(x,y,1,len,color); }
    void flush_buffer(void) { fb.flush(); }

    void draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale) {
        if(c < 0x20 || c > 0x7E) c = ' ';
        uint8_t idx = c - 0x20;
        for(uint8_t col=0; col<5; col++) {
            uint8_t line = font5x7[idx][col];
            for(uint8_t row=0; row<7; row++)
                fill_rect(x+col*scale, y+row*scale, scale, scale,
                          (line & (1<<row)) ? fg : bg);
        }
    }

    void draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale) {
        while(*s) { draw_char(x,y,*s++,fg,bg,scale); x += 6*scale; }
    }

    void draw_number(int16_t x, int16_t y, uint32_t num, uint16_t fg, uint16_t bg, uint8_t scale) {
        char buf[16]; sprintf(buf, "%u", num); draw_string(x,y,buf,fg,bg,scale);
    }

    void drawSprite(int x, int y, const Sprite& sprite, uint16_t /*bgColor*/) {
        if (sprite.w == 0 || sprite.h == 0) return;
        uint16_t* buf = fb.getBuffer();
        int16_t dx = x, dy = y, sw = sprite.w, sh = sprite.h;
        int16_t sx = 0, sy = 0;
        if (dx < 0) { sx = -dx; sw += dx; dx = 0; }
        if (dy < 0) { sy = -dy; sh += dy; dy = 0; }
        if (dx + sw > TFT_W) sw = TFT_W - dx;
        if (dy + sh > TFT_H) sh = TFT_H - dy;
        if (sw <= 0 || sh <= 0) return;
        fb.mark(dx, dy, sw, sh);
        for (int row = 0; row < sh; row++) {
            int off = (dy + row) * TFT_W + dx;
            const uint32_t* src = sprite.pixels.data() + (row + sy) * sprite.w + sx;
            for (int col = 0; col < sw; col++) {
                uint32_t p = src[col];
                uint8_t a = (p >> 24) & 0xFF;
                if (a == 0) continue;
                uint8_t sr = (p >> 16) & 0xFF;
                uint8_t sg = (p >> 8) & 0xFF;
                uint8_t sb = p & 0xFF;
                if (a == 255) {
                    buf[off + col] = ((sr & 0xF8) << 8) | ((sg & 0xFC) << 3) | (sb >> 3);
                } else {
                    uint16_t dst = buf[off + col];
                    uint8_t dr = ((dst >> 11) & 0x1F) * 255 / 31;
                    uint8_t dg = ((dst >> 5)  & 0x3F) * 255 / 63;
                    uint8_t db = (dst & 0x1F) * 255 / 31;
                    uint8_t mr = (sr * a + dr * (255 - a)) / 255;
                    uint8_t mg = (sg * a + dg * (255 - a)) / 255;
                    uint8_t mb = (sb * a + db * (255 - a)) / 255;
                    buf[off + col] = ((mr & 0xF8) << 8) | ((mg & 0xFC) << 3) | (mb >> 3);
                }
            }
        }
    }
}
