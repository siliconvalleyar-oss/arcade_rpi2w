#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "HardwareProfile.h"
#include "Sprite.h"
#include <stdint.h>

namespace GameEngine {
    struct Cactus { int16_t x; uint8_t type; bool active; };

    extern Sprite dinoSprites[2];
    extern Sprite cactusSprites[2];
    extern bool spritesLoaded;

    bool load_sprites(void);
    void init_game(void);
    void update(void);
    void draw(void);
    void game_loop(void);
}

#endif
