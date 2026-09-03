#pragma once
#include "HardwareProfile.h"
#include <cstdint>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

enum class SFX : uint8_t {
    JUMP, LAND, BARREL_ROLL, HAMMER_HIT, DIE,
    LEVEL_CLEAR, BARREL_EXPLODE, COIN, MENU_BEEP, BOSS_ROAR
};

class Sound {
public:
    struct Note {
        unsigned freq;
        unsigned dur_ms;
    };

    Sound() = default;
    ~Sound() { shutdown(); }

    bool init();
    void shutdown();
    void play(SFX sfx);
    void stop();
    void update(int dt_ms);
    void setEnabled(bool e) { enabled_ = e; if (!e) stop(); }
    bool enabled() const { return enabled_; }

private:
    void tone(unsigned freq, unsigned dur_ms);
    void beepThread();

    bool enabled_{true};
    bool ok_{false};
    int  remaining_ms_{0};

    int  gpio_fd_{-1};

    std::thread thread_;
    std::atomic<bool> run_{false};
    std::atomic<bool> toneActive_{false};
    std::atomic<unsigned> toneFreqHz_{0};
    unsigned lastDurMs_{0};
    std::mutex  mtx_;
    std::condition_variable cv_;

    std::vector<Note> currentSequence_;
    size_t currentNoteIdx_{0};
};
