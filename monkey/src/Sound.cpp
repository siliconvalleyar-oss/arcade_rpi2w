#include "../include/Sound.h"
#include "../include/Hw.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

// Tablas de notas
static constexpr Sound::Note SFX_JUMP[]         = {{600,40},{900,60}};
static constexpr Sound::Note SFX_LAND[]         = {{200,40},{150,30}};
static constexpr Sound::Note SFX_BARREL_ROLL[]  = {{110,80}};
static constexpr Sound::Note SFX_HAMMER_HIT[]   = {{300,30},{400,30}};
static constexpr Sound::Note SFX_DIE[]          = {{500,80},{400,80},{300,80},{200,120}};
static constexpr Sound::Note SFX_LEVEL_CLEAR[]  = {{523,80},{659,80},{784,80},{1047,160}};
static constexpr Sound::Note SFX_BARREL_EXPLODE[]= {{220,50},{180,50},{140,80}};
static constexpr Sound::Note SFX_COIN[]         = {{1000,40},{1200,60}};
static constexpr Sound::Note SFX_MENU_BEEP[]    = {{440,50}};
static constexpr Sound::Note SFX_BOSS_ROAR[]    = {{80,200},{60,200},{80,200}};

struct SFXDef {
    const Sound::Note* notes;
    int count;
};
static constexpr SFXDef SFX_TABLE[] = {
    { SFX_JUMP,          2 },
    { SFX_LAND,          2 },
    { SFX_BARREL_ROLL,   1 },
    { SFX_HAMMER_HIT,    2 },
    { SFX_DIE,           4 },
    { SFX_LEVEL_CLEAR,   4 },
    { SFX_BARREL_EXPLODE,3 },
    { SFX_COIN,          2 },
    { SFX_MENU_BEEP,     1 },
    { SFX_BOSS_ROAR,     3 },
};

bool Sound::init()
{
    int fd = open(GPIO_CHIP, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "[Sound] No se pudo abrir %s\n", GPIO_CHIP);
        return false;
    }

    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = PIN_SOUND;
    req.lines = 1;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    strncpy(req.consumer_label, "monkey_sound", 15);

    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        fprintf(stderr, "[Sound] Error al configurar GPIO %d\n", PIN_SOUND);
        close(fd);
        return false;
    }
    gpio_fd_ = req.fd;
    close(fd);

    ok_ = true;
    run_ = true;
    thread_ = std::thread(&Sound::beepThread, this);
    printf("[Sound] Buzzer en GPIO%d (bit-bang thread) listo\n", PIN_SOUND);
    return true;
}

void Sound::shutdown()
{
    bool wasOk = ok_;
    ok_ = false;
    if (wasOk) {
        std::lock_guard<std::mutex> lk(mtx_);
        toneActive_ = false;
    }
    cv_.notify_all();
    if (run_.exchange(false)) {
        if (thread_.joinable()) thread_.join();
    }
    if (gpio_fd_ >= 0) { close(gpio_fd_); gpio_fd_ = -1; }
}

void Sound::tone(unsigned freq, unsigned dur_ms)
{
    if (!ok_ || !enabled_ || gpio_fd_ < 0) return;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (freq == 0) {
            toneActive_ = false;
            toneFreqHz_ = 0;
            return;
        }
        toneFreqHz_ = freq;
        lastDurMs_  = dur_ms;
        toneActive_ = true;
    }
    cv_.notify_one();
}

void Sound::stop()
{
    {
        std::lock_guard<std::mutex> lk(mtx_);
        toneActive_ = false;
        toneFreqHz_ = 0;
        lastDurMs_  = 0;
        currentSequence_.clear();
        currentNoteIdx_ = 0;
    }
    cv_.notify_one();
}

void Sound::play(SFX sfx)
{
    if (!enabled_ || !ok_) return;
    int idx = static_cast<int>(sfx);
    if (idx < 0 || idx >= static_cast<int>(sizeof(SFX_TABLE)/sizeof(SFX_TABLE[0])))
        return;

    const SFXDef& def = SFX_TABLE[idx];
    if (def.count <= 0) return;

    std::lock_guard<std::mutex> lk(mtx_);
    currentSequence_.assign(def.notes, def.notes + def.count);
    currentNoteIdx_ = 0;
    if (currentNoteIdx_ < currentSequence_.size()) {
        const auto& n = currentSequence_[currentNoteIdx_];
        toneFreqHz_ = n.freq;
        lastDurMs_  = n.dur_ms;
        toneActive_ = true;
        currentNoteIdx_++;
    }
    cv_.notify_one();
}

void Sound::update(int dt_ms)
{
    (void)dt_ms;
    // El timing lo gestiona el hilo de bit-bang (beepThread).
    remaining_ms_ = 0;
}

// Hilo: reproduce la secuencia de notas por bit-bang (busy-wait preciso)
void Sound::beepThread()
{
    struct gpiohandle_data data;
    while (run_) {
        unsigned freq;
        unsigned dur;
        {
            std::unique_lock<std::mutex> lk(mtx_);
            cv_.wait(lk, [&]{ return !run_ || toneActive_; });
            if (!run_) break;
            freq = toneFreqHz_;
            dur  = lastDurMs_;
            toneActive_ = false;   // consumimos la orden
        }

        if (freq == 0) continue;

        uint32_t half_us = 500000 / freq;
        if (half_us < 1) half_us = 1;
        uint32_t cycles = (dur * 1000) / (half_us * 2);
        if (cycles < 1) cycles = 1;

        for (uint32_t i = 0; i < cycles; i++) {
            if (!run_ || !ok_) break;
            memset(&data, 0, sizeof(data));
            data.values[0] = 1;
            ioctl(gpio_fd_, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
            hw_delay_us(half_us);

            if (!run_ || !ok_) break;
            memset(&data, 0, sizeof(data));
            data.values[0] = 0;
            ioctl(gpio_fd_, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
            hw_delay_us(half_us);
        }

        // Encolar siguiente nota de la secuencia
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (currentNoteIdx_ < currentSequence_.size()) {
                toneFreqHz_ = currentSequence_[currentNoteIdx_].freq;
                lastDurMs_  = currentSequence_[currentNoteIdx_].dur_ms;
                toneActive_ = true;
                currentNoteIdx_++;
            }
        }
        cv_.notify_one();
    }
}
