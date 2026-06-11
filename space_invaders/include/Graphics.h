#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "HardwareProfile.h"
#include "fonts.h"
#include "Sprite.h"

namespace Graphics {
    void fill_screen(uint16_t color);
    void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void draw_pixel(int16_t x, int16_t y, uint16_t color);
    void draw_hline(int16_t x, int16_t y, int16_t len, uint16_t color);
    void draw_vline(int16_t x, int16_t y, int16_t len, uint16_t color);
    void draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale);
    void draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale);
    void draw_number(int16_t x, int16_t y, uint32_t num, uint16_t fg, uint16_t bg, uint8_t scale);
    void draw_player(int16_t x, int16_t y);
    void draw_invader_type1(int16_t x, int16_t y);
    void draw_invader_type2(int16_t x, int16_t y);
    void draw_invader_type3(int16_t x, int16_t y);
    void draw_bullet(int16_t x, int16_t y);
    void draw_shield(int16_t x, int16_t y);
    void draw_explosion(int16_t x, int16_t y, uint8_t frame);
    void draw_background(void);
    void drawSprite(int x, int y, const Sprite& sprite, uint16_t bgColor);
    void flush_buffer(void);
}
#endif
