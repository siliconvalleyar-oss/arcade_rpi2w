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
static const Note tetris[]={{659,150},{494,150},{523,150},{587,150},{523,150},{494,150},{440,150},{440,150},{523,150},{659,150},{587,150},{523,150}};
static const Note pacman[]={{440,100},{0,40},{880,100},{0,40},{1320,200}};
static const Note dino[]={{440,100},{0,40},{880,100},{0,40},{1320,200},{0,40},{880,100}};
static const Note zelda[]={{330,200},{392,200},{440,400},{392,200},{330,200},{262,400},{294,200},{330,400}};
static const Note marioUg[]={{262,150},{262,150},{294,150},{262,150},{330,150},{349,300},{262,150},{262,150},{294,150},{262,150},{392,150},{349,300}};
static void play(const Note*a,size_t n){ for(size_t i=0;i<n;i++){beep(a[i].freq,a[i].dur);delay_ms(30);} }
int main(void){
    sound_init();
    while(1){
        printf("\nMenu melodias:\n1 Mario\n2 Tetris\n3 Pacman\n4 Dino\n5 Todas\n0 Salir\nOpcion: "); fflush(stdout);
        int c; if(scanf("%d",&c)!=1) break;
        if(c==0) break;
        if(c==1) play(mario,sizeof(mario)/sizeof(mario[0]));
        else if(c==2) play(tetris,sizeof(tetris)/sizeof(tetris[0]));
        else if(c==3) play(pacman,sizeof(pacman)/sizeof(pacman[0]));
        else if(c==4) play(dino,sizeof(dino)/sizeof(dino[0]));
        else if(c==5) play(zelda,sizeof(zelda)/sizeof(zelda[0]));
        else if(c==6) play(marioUg,sizeof(marioUg)/sizeof(marioUg[0]));
        else if(c==5){ play(mario,sizeof(mario)/sizeof(mario[0])); delay_ms(500);
                      play(tetris,sizeof(tetris)/sizeof(tetris[0])); delay_ms(500);
                      play(pacman,sizeof(pacman)/sizeof(pacman[0])); delay_ms(500);
                      play(dino,sizeof(dino)/sizeof(dino[0])); delay_ms(500);
                      play(zelda,sizeof(zelda)/sizeof(zelda[0])); delay_ms(500);
                      play(marioUg,sizeof(marioUg)/sizeof(marioUg[0])); }
    }
    return 0;
}
