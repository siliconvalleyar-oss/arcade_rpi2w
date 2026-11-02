#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <time.h>

#define PIN_SOUND 13
static int gpio_fd=-1;

static void delay_us(long us){ struct timespec ts={0,us*1000}; nanosleep(&ts,NULL);}
static void delay_ms(long ms){ struct timespec ts={ms/1000,(ms%1000)*1000000L}; nanosleep(&ts,NULL);}

static void sound_init(void){
    int fd=open("/dev/gpiochip0",O_RDONLY);
    if(fd<0){perror("gpiochip");return;}
    struct gpiohandle_request req; memset(&req,0,sizeof(req));
    req.lineoffsets[0]=PIN_SOUND; req.lines=1; req.flags=GPIOHANDLE_REQUEST_OUTPUT;
    strncpy(req.consumer_label,"melody",15);
    if(ioctl(fd,GPIO_GET_LINEHANDLE_IOCTL,&req)==0) gpio_fd=req.fd;
    close(fd);
}
static void beep(int freq,int dur){
    if(gpio_fd<0||freq==0){delay_ms(dur);return;}
    int half=500000/freq; int cycles=(dur*1000)/(half*2); if(cycles<1)cycles=1;
    struct gpiohandle_data d; memset(&d,0,sizeof(d));
    for(int i=0;i<cycles;i++){ d.values[0]=1; ioctl(gpio_fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&d); delay_us(half);
        d.values[0]=0; ioctl(gpio_fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&d); delay_us(half);}
}
struct Note{int freq;int dur;};
static const Note mario[]={{659,125},{659,125},{0,125},{659,125},{0,167},{523,125},{659,125},{784,250},{0,250},{392,250}};
static const Note scale[]={{262,200},{294,200},{330,200},{349,200},{392,200},{440,200},{494,200},{523,400}};
int main(void){
    sound_init();
    printf("=== MELODIES ===\n");
    for(int r=0;r<3;r++){
        for(size_t i=0;i<sizeof(mario)/sizeof(mario[0]);i++){ beep(mario[i].freq,mario[i].dur); delay_ms(30);}
        delay_ms(500);
    }
    for(size_t i=0;i<sizeof(scale)/sizeof(scale[0]);i++){ beep(scale[i].freq,scale[i].dur); delay_ms(40);}
    printf("Done\n");
    return 0;
}
