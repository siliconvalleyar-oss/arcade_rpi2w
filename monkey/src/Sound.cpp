#include "../include/Sound.h"
#include <pigpio.h>
#include <cstdio>

// Tablas de notas (Sound::Note ya es accesible)
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
    gpioSetMode(PIN_SOUND, PI_OUTPUT);
    gpioWrite(PIN_SOUND, 0);
    gpioSetPWMrange(PIN_SOUND, 255);   // Rango para PWM por software
    pigpio_ok_ = true;
    printf("[Sound] Buzzer en GPIO%d (soft-PWM) listo\n", PIN_SOUND);
    return true;
}

void Sound::shutdown()
{
    if (pigpio_ok_) stop();
}

void Sound::tone(unsigned freq, unsigned dur_ms)
{
    if (!pigpio_ok_ || !enabled_) return;
    if (freq == 0) {
        gpioPWM(PIN_SOUND, 0);
    } else {
        gpioSetPWMfrequency(PIN_SOUND, freq);
        gpioPWM(PIN_SOUND, 128);   // 50% duty cycle
    }
    remaining_ms_ = static_cast<int>(dur_ms);
}

void Sound::stop()
{
    if (!pigpio_ok_) return;
    gpioPWM(PIN_SOUND, 0);
    gpioWrite(PIN_SOUND, 0);
    remaining_ms_ = 0;
    currentSequence_.clear();
    currentNoteIdx_ = 0;
}

void Sound::play(SFX sfx)
{
    if (!enabled_ || !pigpio_ok_) return;
    int idx = static_cast<int>(sfx);
    if (idx < 0 || idx >= static_cast<int>(sizeof(SFX_TABLE)/sizeof(SFX_TABLE[0])))
        return;

    const SFXDef& def = SFX_TABLE[idx];
    if (def.count <= 0) return;

    currentSequence_.assign(def.notes, def.notes + def.count);
    currentNoteIdx_ = 0;
    if (currentNoteIdx_ < currentSequence_.size()) {
        const auto& n = currentSequence_[currentNoteIdx_];
        tone(n.freq, n.dur_ms);
        currentNoteIdx_++;
    }
}

void Sound::update(int dt_ms)
{
    if (!pigpio_ok_ || !enabled_) return;
    if (remaining_ms_ > 0) {
        remaining_ms_ -= dt_ms;
        if (remaining_ms_ <= 0) {
            if (currentNoteIdx_ < currentSequence_.size()) {
                const auto& n = currentSequence_[currentNoteIdx_];
                tone(n.freq, n.dur_ms);
                currentNoteIdx_++;
            } else {
                stop();
            }
        }
    }
}
