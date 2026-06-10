#ifndef PIEZO_DRIVER_HPP
#define PIEZO_DRIVER_HPP

#include <bcm2835.h>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <stdexcept> 


class PiezoDriver {
private:
    uint8_t pin;
    std::atomic<bool> isPlaying;
    
public:
    explicit PiezoDriver(uint8_t pin) : pin(pin), isPlaying(false) {
        if (!bcm2835_init()) {
            throw std::runtime_error("Failed to initialize bcm2835");
        }
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    }
    
    ~PiezoDriver() {
        stop();
        bcm2835_close();
    }
    
    void generateTone(int frequency, int durationMs) {
        if (frequency <= 0) return;
        
        isPlaying = true;
        int periodUs = 1000000 / frequency;
        int halfPeriodUs = periodUs / 2;
        int cycles = (durationMs * 1000) / periodUs;
        
        for (int i = 0; i < cycles && isPlaying; ++i) {
            bcm2835_gpio_write(pin, HIGH);
            bcm2835_delayMicroseconds(halfPeriodUs);
            bcm2835_gpio_write(pin, LOW);
            bcm2835_delayMicroseconds(halfPeriodUs);
        }
    }
    
    void generateCustomWaveform(const std::vector<int>& waveform, int sampleRate) {
        isPlaying = true;
        int sampleDelayUs = 1000000 / sampleRate;
        
        for (const auto& value : waveform) {
            if (!isPlaying) break;
            bcm2835_gpio_write(pin, value ? HIGH : LOW);
            bcm2835_delayMicroseconds(sampleDelayUs);
        }
    }
    
    void stop() {
        isPlaying = false;
        bcm2835_gpio_write(pin, LOW);
    }
};

#endif
