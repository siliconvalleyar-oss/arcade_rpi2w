#include "../include/Hw.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/gpio.h>

// ============================================================
//  Hw - GPIO + SPI vía ioctl (patrón pacman/cars)
// ============================================================

static int spi_fd  = -1;
static int gpio_fd = -1;
static int gpio_out_fd = -1;   // DC, RST, BL
static int gpio_in_fd  = -1;   // botones joystick (entradas)

static const uint32_t out_pins[HW_N_OUT_PINS] = { PIN_DC, PIN_RST, PIN_BL };
static uint8_t out_state[HW_N_OUT_PINS] = { 0, 1, 0 };
static int n_in_lines = 0;
static uint32_t in_pins[6] = { PIN_JOY_UP, PIN_JOY_DOWN, PIN_JOY_LEFT, PIN_JOY_RIGHT, PIN_JOY_BTN_A, PIN_JOY_BTN_B };

static int find_out_idx(int pin) {
    for (int i = 0; i < HW_N_OUT_PINS; i++)
        if ((int)out_pins[i] == pin) return i;
    return -1;
}

static int find_in_idx(int pin) {
    for (int i = 0; i < n_in_lines; i++)
        if ((int)in_pins[i] == pin) return i;
    return -1;
}

static int gpio_init(void) {
    gpio_fd = open(GPIO_CHIP, O_RDONLY);
    if (gpio_fd < 0) { perror("open /dev/gpiochip0"); return -1; }

    struct gpiohandle_request req_out;
    memset(&req_out, 0, sizeof(req_out));
    req_out.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req_out.lines = HW_N_OUT_PINS;
    for (int i = 0; i < HW_N_OUT_PINS; i++) {
        req_out.lineoffsets[i]    = out_pins[i];
        req_out.default_values[i] = out_state[i];
    }
    strncpy(req_out.consumer_label, "monkey_out", 15);
    if (ioctl(gpio_fd, GPIO_GET_LINEHANDLE_IOCTL, &req_out) < 0) {
        perror("GPIO_GET_LINEHANDLE_IOCTL (salidas)");
        close(gpio_fd); gpio_fd = -1;
        return -2;
    }
    gpio_out_fd = req_out.fd;

#ifdef JOYSTICK_ENABLED
    n_in_lines = 6;
    struct gpiohandle_request req_in;
    memset(&req_in, 0, sizeof(req_in));
    req_in.flags = GPIOHANDLE_REQUEST_INPUT;
    req_in.lines = n_in_lines;
    for (int i = 0; i < n_in_lines; i++) req_in.lineoffsets[i] = in_pins[i];
    strncpy(req_in.consumer_label, "monkey_joy", 15);
    if (ioctl(gpio_fd, GPIO_GET_LINEHANDLE_IOCTL, &req_in) >= 0) {
        gpio_in_fd = req_in.fd;
    } else {
        n_in_lines = 0;
        gpio_in_fd = -1;
    }
#else
    n_in_lines = 0;
#endif

    return 0;
}

int hw_gpio_write(int pin, int val) {
    int idx = find_out_idx(pin);
    if (idx < 0) return -1;
    out_state[idx] = val ? 1 : 0;
    struct gpiohandle_data data;
    memset(&data, 0, sizeof(data));
    for (int i = 0; i < HW_N_OUT_PINS; i++) data.values[i] = out_state[i];
    ioctl(gpio_out_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
    return 0;
}

int hw_gpio_read(int pin) {
    if (gpio_in_fd < 0) return 0;
    int idx = find_in_idx(pin);
    if (idx < 0) return 0;
    struct gpiohandle_data data;
    memset(&data, 0, sizeof(data));
    if (ioctl(gpio_in_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) return 0;
    return data.values[idx];
}

static int spi_init(void) {
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("open /dev/spidev0.0");
        fprintf(stderr, "Asegurate de que dtparam=spi=on en /boot/config.txt\n");
        return -1;
    }

    uint8_t mode = SPI_MODE;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) { perror("SPI_IOC_WR_MODE"); return -2; }

    uint8_t bits = 8;
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);

    uint32_t speed = SPI_SPEED_HZ;
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("SPI_IOC_WR_MAX_SPEED_HZ");
        speed = 32000000;
        ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        fprintf(stderr, "SPI: usando 32 MHz como fallback\n");
    }
    fprintf(stderr, "SPI: modo %u, %u MHz\n", SPI_MODE, speed / 1000000);
    return 0;
}

void hw_spi_write_byte(uint8_t d) {
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf        = (unsigned long)&d;
    tr.rx_buf        = 0;
    tr.len           = 1;
    tr.speed_hz      = SPI_SPEED_HZ;
    tr.bits_per_word = 8;
    tr.cs_change     = 0;
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

void hw_spi_write_buf(const uint8_t* buf, uint32_t len) {
    if (!buf || !len) return;
    const uint32_t CHUNK = 4096;
    uint32_t offset = 0;
    while (offset < len) {
        uint32_t chunk = (len - offset < CHUNK) ? (len - offset) : CHUNK;
        struct spi_ioc_transfer tr;
        memset(&tr, 0, sizeof(tr));
        tr.tx_buf        = (unsigned long)(buf + offset);
        tr.rx_buf        = 0;
        tr.len           = chunk;
        tr.speed_hz      = SPI_SPEED_HZ;
        tr.bits_per_word = 8;
        tr.cs_change     = 0;
        ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
        offset += chunk;
    }
}

void hw_delay_ms(uint32_t ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, nullptr);
}

void hw_delay_us(uint32_t us) {
    struct timespec ts;
    ts.tv_sec  = 0;
    ts.tv_nsec = (long)us * 1000L;
    nanosleep(&ts, nullptr);
}

int hw_init(void) {
    if (gpio_init() < 0) return -1;
    if (spi_init() < 0) { hw_close(); return -2; }
    HW_RST_HIGH();
    HW_DC_LOW();
    HW_BL_LOW();
    return 0;
}

void hw_close(void) {
    if (gpio_out_fd >= 0) { close(gpio_out_fd); gpio_out_fd = -1; }
    if (gpio_in_fd  >= 0) { close(gpio_in_fd);  gpio_in_fd  = -1; }
    if (gpio_fd     >= 0) { close(gpio_fd);      gpio_fd     = -1; }
    if (spi_fd      >= 0) { close(spi_fd);       spi_fd      = -1; }
}
