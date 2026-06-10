#include "Sound.h"
#include "HardwareProfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

static int sound_fd = -1;

void sound_init() {
    int fd = open("/dev/gpiochip0", O_RDONLY);
    if(fd < 0) return;
    struct gpiohandle_request req = {0};
    req.lineoffsets[0] = PIN_SOUND;
    req.lines = 1;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) == 0)
        sound_fd = req.fd;
    close(fd);
}

void delay_us_sound(uint32_t us) {
    struct timespec ts = {0, (long)us * 1000L};
    nanosleep(&ts, NULL);
}

void sound_beep(uint16_t freq, uint16_t ms) {
    if(sound_fd < 0 || freq == 0) {
        delay_us_sound(ms * 1000);
        return;
    }
    uint32_t half = 500000 / freq;
    uint32_t cycles = (ms * 1000) / (half * 2);
    if(cycles < 1) cycles = 1;
    struct gpiohandle_data data;
    for(uint32_t i=0; i<cycles; i++) {
        data.values[0] = 1;
        ioctl(sound_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
        delay_us_sound(half);
        data.values[0] = 0;
        ioctl(sound_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
        delay_us_sound(half);
    }
}

void sound_shoot() { sound_beep(800, 50); }
void sound_invader_explosion() { sound_beep(300, 60); delay_us_sound(15000); sound_beep(250, 80); }
void sound_player_explosion() { for(int i=0;i<3;i++) { sound_beep(400-i*100,80); delay_us_sound(40000); } sound_beep(100,200); }
void sound_start() { sound_beep(440,100); delay_us_sound(50000); sound_beep(660,100); delay_us_sound(50000); sound_beep(880,200); }
void sound_game_over() { for(int i=0;i<4;i++) { sound_beep(300,150); delay_us_sound(80000); } sound_beep(200,300); }
void sound_level_up() { sound_beep(440,80); delay_us_sound(40000); sound_beep(880,80); delay_us_sound(40000); sound_beep(1320,150); }
