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
            for(uint8_t row=0; row<7; row++) {
                fill_rect(x+col*scale, y+row*scale, scale, scale,
                          (line & (1<<row)) ? fg : bg);
            }
        }
    }
    
    void draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale) {
        while(*s) { draw_char(x,y,*s++,fg,bg,scale); x += 6*scale; }
    }
    
    void draw_number(int16_t x, int16_t y, uint32_t num, uint16_t fg, uint16_t bg, uint8_t scale) {
        char buf[16]; sprintf(buf, "%u", num); draw_string(x,y,buf,fg,bg,scale);
    }
    
    void draw_player(int16_t x, int16_t y) {
        for(int i=0; i<PLAYER_WIDTH; i++) {
            int h = (i<3||i>12)?3:(i<5||i>10)?6:10;
            fill_rect(x+i, y+PLAYER_HEIGHT-h, 1, h, PLAYER_COLOR);
        }
        fill_rect(x+6, y+PLAYER_HEIGHT-4, 4, 4, PLAYER_COLOR);
    }
    
    void draw_invader_type1(int16_t x, int16_t y) {
        fill_rect(x+4,y,8,2,INVADER_COLOR1); fill_rect(x+2,y+2,12,2,INVADER_COLOR1);
        fill_rect(x+0,y+4,16,2,INVADER_COLOR1); fill_rect(x+2,y+6,12,2,INVADER_COLOR1);
        fill_rect(x+4,y+2,2,2,BLACK); fill_rect(x+10,y+2,2,2,BLACK);
        fill_rect(x+4,y+8,8,2,INVADER_COLOR1); fill_rect(x+6,y+10,4,2,INVADER_COLOR1);
        fill_rect(x+2,y+10,2,2,INVADER_COLOR1); fill_rect(x+12,y+10,2,2,INVADER_COLOR1);
    }
    void draw_invader_type2(int16_t x, int16_t y) {
        fill_rect(x+4,y,8,2,INVADER_COLOR2); fill_rect(x+2,y+2,12,2,INVADER_COLOR2);
        fill_rect(x+0,y+4,16,2,INVADER_COLOR2); fill_rect(x+2,y+6,12,2,INVADER_COLOR2);
        fill_rect(x+4,y+2,2,2,BLACK); fill_rect(x+10,y+2,2,2,BLACK);
        fill_rect(x+4,y+8,8,2,INVADER_COLOR2); fill_rect(x+6,y+10,4,2,INVADER_COLOR2);
        fill_rect(x+2,y+10,2,2,INVADER_COLOR2); fill_rect(x+12,y+10,2,2,INVADER_COLOR2);
    }
    void draw_invader_type3(int16_t x, int16_t y) {
        fill_rect(x+4,y,8,2,INVADER_COLOR3); fill_rect(x+2,y+2,12,2,INVADER_COLOR3);
        fill_rect(x+0,y+4,16,2,INVADER_COLOR3); fill_rect(x+2,y+6,12,2,INVADER_COLOR3);
        fill_rect(x+4,y+2,2,2,BLACK); fill_rect(x+10,y+2,2,2,BLACK);
        fill_rect(x+4,y+8,8,2,INVADER_COLOR3); fill_rect(x+6,y+10,4,2,INVADER_COLOR3);
        fill_rect(x+2,y+10,2,2,INVADER_COLOR3); fill_rect(x+12,y+10,2,2,INVADER_COLOR3);
    }
    void draw_bullet(int16_t x, int16_t y) { fill_rect(x, y, BULLET_WIDTH, BULLET_HEIGHT, BULLET_COLOR); }
    void draw_shield(int16_t x, int16_t y) {
        for(int i=0;i<16;i++) {
            int h = (i<3||i>12)?4:(i<5||i>10)?8:12;
            fill_rect(x+i, y+12-h, 1, h, SHIELD_COLOR);
        }
    }
    void draw_explosion(int16_t x, int16_t y, uint8_t frame) {
        int size = 4+frame*2; int off = size/2;
        fill_rect(x-off, y-off, size, size, (frame%2)?RED:YELLOW);
    }
    void draw_background(void) { fill_screen(BLACK); }
}
