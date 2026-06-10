#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "HardwareProfile.h"
#include <string.h>

class FrameBuffer {
public:
    static FrameBuffer& get() {
        static FrameBuffer instance;
        return instance;
    }
    
    void init() {
        memset(buffer, 0, sizeof(buffer));
        memset(prev, 0, sizeof(prev));
        full = true;
        num = 0;
    }
    
    void setPixel(int16_t x, int16_t y, uint16_t color) {
        if(x<0 || x>=TFT_W || y<0 || y>=TFT_H) return;
        buffer[y*TFT_W + x] = color;
        mark(x, y, 1, 1);
    }
    
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        if(x>=TFT_W || y>=TFT_H || w<=0 || h<=0) return;
        if(x<0) { w+=x; x=0; }
        if(y<0) { h+=y; y=0; }
        if(x+w>TFT_W) w = TFT_W-x;
        if(y+h>TFT_H) h = TFT_H-y;
        for(int16_t row=0; row<h; row++) {
            int offset = (y+row)*TFT_W + x;
            for(int16_t col=0; col<w; col++) buffer[offset+col] = color;
        }
        mark(x, y, w, h);
    }
    
    void flush() {
        if(full) {
            // Enviar toda la pantalla
            set_window(0,0,TFT_W-1,TFT_H-1);
            DC_HIGH();
            uint8_t buf[TFT_W*TFT_H*2];
            int idx=0;
            for(int i=0; i<TFT_W*TFT_H; i++) {
                uint16_t c = buffer[i];
                buf[idx++] = c>>8;
                buf[idx++] = c&0xFF;
            }
            spi_write_buf(buf, sizeof(buf));
            memcpy(prev, buffer, sizeof(buffer));
            full = false;
            num = 0;
            return;
        }
        for(int i=0; i<num; i++) {
            int16_t x = rects[i].x, y = rects[i].y, w = rects[i].w, h = rects[i].h;
            for(int16_t row=0; row<h; row++) {
                int yp = y+row;
                int off = yp*TFT_W + x;
                bool dirty = false;
                for(int16_t col=0; col<w; col++)
                    if(buffer[off+col] != prev[off+col]) { dirty=true; break; }
                if(dirty) {
                    set_window(x, yp, x+w-1, yp);
                    DC_HIGH();
                    uint8_t buf[480];
                    int idx=0;
                    for(int16_t col=0; col<w; col++) {
                        uint16_t c = buffer[off+col];
                        buf[idx++] = c>>8;
                        buf[idx++] = c&0xFF;
                    }
                    spi_write_buf(buf, idx);
                    memcpy(&prev[off], &buffer[off], w*2);
                }
            }
        }
        num = 0;
    }
    
    void clear(uint16_t color) {
        for(int i=0; i<TFT_W*TFT_H; i++) buffer[i] = color;
        full = true;
    }
    
private:
    FrameBuffer() : full(true), num(0) {}
    
    void mark(int16_t x, int16_t y, int16_t w, int16_t h) {
        if(full) return;
        for(int i=0; i<num; i++) {
            if(x < rects[i].x+rects[i].w && x+w > rects[i].x &&
               y < rects[i].y+rects[i].h && y+h > rects[i].y) {
                int16_t nx = (x < rects[i].x) ? x : rects[i].x;
                int16_t ny = (y < rects[i].y) ? y : rects[i].y;
                int16_t nw = ((x+w) > (rects[i].x+rects[i].w)) ? (x+w-nx) : (rects[i].x+rects[i].w-nx);
                int16_t nh = ((y+h) > (rects[i].y+rects[i].h)) ? (y+h-ny) : (rects[i].y+rects[i].h-ny);
                rects[i].x = nx; rects[i].y = ny;
                rects[i].w = nw; rects[i].h = nh;
                return;
            }
        }
        if(num < 32) {
            rects[num].x = x; rects[num].y = y;
            rects[num].w = w; rects[num].h = h;
            num++;
        } else full = true;
    }
    
    uint16_t buffer[TFT_W*TFT_H];
    uint16_t prev[TFT_W*TFT_H];
    struct { int16_t x,y,w,h; } rects[32];
    int num;
    bool full;
};

#endif
