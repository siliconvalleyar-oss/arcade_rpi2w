#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "HardwareProfile.h"
#include <stdint.h>

namespace GameEngine {
    void init_game(void);
    void update(void);
    void draw(void);
    void game_loop(void);
}

#endif
