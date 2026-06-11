#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "HardwareProfile.h"
#include "Graphics.h"

namespace GameEngine {

    // ---- Jugador ----
    extern int16_t playerX, playerY;
    extern int8_t  playerDir;          // -1 izquierda, +1 derecha, 0 quieto
    extern bool    demoMode;           // true = demo automático

    // ---- Enemigos ----
    struct Enemy {
        int16_t x, y;
        bool active;
    };
    extern Enemy enemies[8];
    extern uint8_t enemyCount;

    // ---- Estado del juego ----
    extern uint16_t score;
    extern uint8_t  lives;
    extern uint8_t  level;
    extern bool     gameOver;
    extern uint16_t highScore;

    // ---- Sprites ----
    extern Sprite playerCar;
    extern Sprite enemyCar;
    extern bool spritesLoaded;

    // ---- API ----
    bool load_sprites(void);
    void init_game(void);
    void draw_road(void);
    void update_player(void);
    void update_enemies(void);
    void check_collisions(void);
    void draw_status(void);
    void game_loop(bool demo);
    void show_title(void);
    void show_game_over(void);
}

#endif