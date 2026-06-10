#include "HardwareProfile.h"
#include "Graphics.h"
#include "GameEngine.h"
#include "Sound.h"
#include "FrameBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

static int spi_fd = -1;

// Funciones GPIO vía sysfs
void gpio_export(int pin) {
    char buf[64];
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if(fd<0) return;
    snprintf(buf,sizeof(buf),"%d",pin);
    write(fd,buf,strlen(buf));
    close(fd);
    usleep(100000);
}
void gpio_set_dir(int pin, const char* dir) {
    char path[64];
    snprintf(path,sizeof(path),"/sys/class/gpio/gpio%d/direction",pin);
    int fd = open(path,O_WRONLY);
    if(fd<0) return;
    write(fd,dir,strlen(dir));
    close(fd);
}
void gpio_write(int pin, int val) {
    char path[64];
    snprintf(path,sizeof(path),"/sys/class/gpio/gpio%d/value",pin);
    int fd = open(path,O_WRONLY);
    if(fd<0) return;
    if(val) write(fd,"1",1); else write(fd,"0",1);
    close(fd);
}
int gpio_read(int pin) {
    char path[64], val;
    snprintf(path,sizeof(path),"/sys/class/gpio/gpio%d/value",pin);
    int fd = open(path,O_RDONLY);
    if(fd<0) return 1;
    read(fd,&val,1);
    close(fd);
    return (val=='0')?0:1;
}

void delay_ms(uint32_t ms) {
    struct timespec ts={ms/1000, (long)(ms%1000)*1000000L};
    nanosleep(&ts,NULL);
}
void delay_us(uint32_t us) {
    struct timespec ts={0, (long)us*1000L};
    nanosleep(&ts,NULL);
}
void spi_write_byte(uint8_t d) {
    struct spi_ioc_transfer tr={0};
    tr.tx_buf=(unsigned long)&d;
    tr.len=1;
    tr.speed_hz=SPI_SPEED_HZ;
    tr.bits_per_word=8;
    ioctl(spi_fd,SPI_IOC_MESSAGE(1),&tr);
}
void spi_write_buf(const uint8_t *buf, uint32_t len) {
    if(!len) return;
    struct spi_ioc_transfer tr={0};
    tr.tx_buf=(unsigned long)buf;
    tr.len=len;
    tr.speed_hz=SPI_SPEED_HZ;
    tr.bits_per_word=8;
    ioctl(spi_fd,SPI_IOC_MESSAGE(1),&tr);
}
void write_cmd(uint8_t c) { DC_LOW(); spi_write_byte(c); }
void write_data(uint8_t d) { DC_HIGH(); spi_write_byte(d); }
void push_color(uint16_t color) {
    DC_HIGH();
    uint8_t buf[2]={color>>8,color&0xFF};
    spi_write_buf(buf,2);
}
void push_color_n(uint16_t color, uint32_t n) {
    if(!n) return;
    DC_HIGH();
    const uint32_t CHUNK=512;
    uint8_t buf[CHUNK*2];
    uint8_t hi=color>>8, lo=color&0xFF;
    for(uint32_t i=0;i<CHUNK;i++) { buf[2*i]=hi; buf[2*i+1]=lo; }
    while(n) { uint32_t c=(n<CHUNK)?n:CHUNK; spi_write_buf(buf,c*2); n-=c; }
}
void set_window(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1) {
    write_cmd(0x2A); write_data(x0>>8); write_data(x0); write_data(x1>>8); write_data(x1);
    write_cmd(0x2B); write_data(y0>>8); write_data(y0); write_data(y1>>8); write_data(y1);
    write_cmd(0x2C);
    DC_HIGH();
}
void init_display() {
    RST_HIGH(); delay_ms(10); RST_LOW(); delay_ms(20); RST_HIGH(); delay_ms(150);
    write_cmd(0x11); delay_ms(120);
    write_cmd(0x36); write_data(0x00);
    write_cmd(0x3A); write_data(0x05);
    write_cmd(0x21); write_cmd(0x13);
    write_cmd(0xB2); write_data(0x0C); write_data(0x0C); write_data(0x00); write_data(0x33); write_data(0x33);
    write_cmd(0xB7); write_data(0x35);
    write_cmd(0xBB); write_data(0x37);
    write_cmd(0xC0); write_data(0x2C);
    write_cmd(0xC2); write_data(0x01);
    write_cmd(0xC3); write_data(0x12);
    write_cmd(0xC4); write_data(0x20);
    write_cmd(0xC6); write_data(0x0F);
    write_cmd(0xD0); write_data(0xA4); write_data(0xA1);
    write_cmd(0xE0); uint8_t g1[]={0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
    for(int i=0;i<14;i++) write_data(g1[i]);
    write_cmd(0xE1); uint8_t g2[]={0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
    for(int i=0;i<14;i++) write_data(g2[i]);
    write_cmd(0x29); delay_ms(120);
}

int hw_init() {
    const int out[]={PIN_DC,PIN_RST,PIN_BL,PIN_CS};
    const int in[]={BTN_LEFT_PIN,BTN_RIGHT_PIN,BTN_FIRE_PIN};
    for(int i=0;i<4;i++) { gpio_export(out[i]); gpio_set_dir(out[i],"out"); }
    for(int i=0;i<3;i++) { gpio_export(in[i]); gpio_set_dir(in[i],"in"); }
    // LED
    gpio_export(PIN_LED); gpio_set_dir(PIN_LED,"out"); gpio_write(PIN_LED,1);
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if(spi_fd<0) return -1;
    uint8_t mode=SPI_MODE_3; ioctl(spi_fd,SPI_IOC_WR_MODE,&mode);
    uint8_t bits=8; ioctl(spi_fd,SPI_IOC_WR_BITS_PER_WORD,&bits);
    uint32_t speed=SPI_SPEED_HZ; ioctl(spi_fd,SPI_IOC_WR_MAX_SPEED_HZ,&speed);
    CS_HIGH(); DC_LOW(); RST_HIGH(); BL_LOW();
    return 0;
}
void hw_close() {
    BL_LOW();
    if(spi_fd>=0) close(spi_fd);
}
void sig_handler(int s) { (void)s; hw_close(); _exit(0); }

int main() {
    signal(SIGINT,sig_handler); signal(SIGTERM,sig_handler);
    if(hw_init()<0) { fprintf(stderr,"Hardware init failed\n"); return 1; }
    init_display();
    BL_HIGH();
    FrameBuffer::get().init();
    sound_init();
    GameEngine::game_loop();
    hw_close();
    return 0;
}
