#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "HardwareProfile.h"
#include "Sprite.h"
#include <stdint.h>

namespace GameEngine {
    struct Invader { int16_t x,y; bool active; uint8_t type; uint8_t anim; };
    struct Bullet { int16_t x,y; bool active; bool is_player; };
    struct Explosion { int16_t x,y; uint8_t frame; bool active; };

    extern Sprite playerSprites[5];
    extern Sprite enemySprites[3];
    extern Sprite bulletSprite;
    extern Sprite impactSprite;
    extern bool spritesLoaded;

    bool load_sprites(void);
    void init_game(void);
    void update_player(void);
    void update_bullets(void);
    void update_invaders(void);
    void update_explosions(void);
    void check_collisions(void);
    void draw_game(void);
    void game_loop(void);

    extern int16_t player_x;
    extern bool game_active;
    extern uint16_t score, high_score;
    extern uint8_t lives, level, invaders_remaining;
}
#endif
