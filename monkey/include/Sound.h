#pragma once
#include "HardwareProfile.h"
#include <cstdint>
#include <vector>

enum class SFX : uint8_t {
    JUMP, LAND, BARREL_ROLL, HAMMER_HIT, DIE,
    LEVEL_CLEAR, BARREL_EXPLODE, COIN, MENU_BEEP, BOSS_ROAR
};

class Sound {
public:
    // Estructura visible para definir las tablas de notas
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

    bool enabled_{true};
    bool pigpio_ok_{false};
    int  remaining_ms_{0};

    // Para secuencias de notas
    std::vector<Note> currentSequence_;
    size_t currentNoteIdx_{0};
};
