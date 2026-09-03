#ifndef PIEZO_DRIVER_HPP
#define PIEZO_DRIVER_HPP

#include <cstdint>
#include <cstring>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

class PiezoDriver {
private:
    uint8_t pin;
    int gpio_fd;
    std::atomic<bool> isPlaying;

    static void delay_us(uint32_t us) {
        struct timespec ts;
        ts.tv_sec  = us / 1000000;
        ts.tv_nsec = (us % 1000000) * 1000;
        nanosleep(&ts, nullptr);
    }

    void write_pin(int value) {
        struct gpiohandle_data data;
        std::memset(&data, 0, sizeof(data));
        data.values[0] = value ? 1 : 0;
        ioctl(gpio_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
    }

public:
    explicit PiezoDriver(uint8_t pin) : pin(pin), gpio_fd(-1), isPlaying(false) {
        int chip_fd = open(GPIO_PATH, O_RDONLY);
        if (chip_fd < 0) {
            throw std::runtime_error("Failed to open /dev/gpiochip0");
        }

        struct gpiohandle_request req;
        std::memset(&req, 0, sizeof(req));
        req.lineoffsets[0] = pin;
        req.lines = 1;
        req.flags = GPIOHANDLE_REQUEST_OUTPUT;
        req.default_values[0] = 0;
        std::strncpy(req.consumer_label, "arcade_sound", 15);

        if (ioctl(chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
            close(chip_fd);
            throw std::runtime_error("GPIO_GET_LINEHANDLE_IOCTL failed");
        }
        close(chip_fd);
        gpio_fd = req.fd;
    }

    ~PiezoDriver() {
        stop();
        if (gpio_fd >= 0) {
            close(gpio_fd);
        }
    }

    void generateTone(int frequency, int durationMs) {
        if (frequency <= 0) return;

        isPlaying = true;
        int periodUs = 1000000 / frequency;
        int halfPeriodUs = periodUs / 2;
        int cycles = (durationMs * 1000) / periodUs;

        for (int i = 0; i < cycles && isPlaying; ++i) {
            write_pin(1);
            delay_us(halfPeriodUs);
            write_pin(0);
            delay_us(halfPeriodUs);
        }
    }

    void generateCustomWaveform(const std::vector<int>& waveform, int sampleRate) {
        isPlaying = true;
        int sampleDelayUs = 1000000 / sampleRate;

        for (const auto& value : waveform) {
            if (!isPlaying) break;
            write_pin(value ? 1 : 0);
            delay_us(sampleDelayUs);
        }
    }

    void stop() {
        isPlaying = false;
        write_pin(0);
    }

    static constexpr const char* GPIO_PATH = "/dev/gpiochip0";
};

#endif
