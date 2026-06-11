#include "../include/GameEngine.h"
#include "../include/Sound.h"
#include "../include/picopng.h"
#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <cstring>
#include <fstream>
#include <vector>

namespace GameEngine {

// ========== Sprites ==========
Sprite playerCar;
Sprite enemyCar;
bool spritesLoaded = false;

bool load_sprites(void) {
    if (spritesLoaded) return true;
    if (!playerCar.load("assets/car_01.png")) {
        fprintf(stderr, "Error: no se pudo cargar assets/car_01.png\n");
        return false;
    }
    if (!enemyCar.load("assets/enemy_00.png")) {
        fprintf(stderr, "Error: no se pudo cargar assets/enemy_00.png\n");
        return false;
    }
    spritesLoaded = true;
    fprintf(stderr, "Sprites loaded: %dx%d enemy:%dx%d\n",
            playerCar.w, playerCar.h, enemyCar.w, enemyCar.h);
    return true;
}

// ========== Road constants ==========
static const int16_t CENTER_X  = (ROAD_LEFT + ROAD_RIGHT) / 2;
static const int16_t DASH_W    = 4;
static const int16_t DASH_H    = 10;
static const int16_t DASH_STEP = 20;
static const uint16_t ROAD_COLOR   = COLOR565(20,20,20);
static const uint16_t ASPHALT_COLOR = COLOR565(40,40,40);

// ========== Game variables ==========
int16_t playerX, playerY;
int8_t  playerDir;
bool    demoMode;

Enemy enemies[8];
uint8_t enemyCount;

uint16_t score;
uint16_t highScore;
uint8_t  lives;
uint8_t  level;
bool     gameOver;

static int16_t prevPlayerX, prevPlayerY;
static int16_t prevEnemyX[8], prevEnemyY[8];
static int16_t enemySpeed;
static int16_t spawnDelay;
static int16_t spawnInterval;

// ========== Road scrolling ==========
static int16_t roadOffset = 0;
static int16_t roadSpeed  = 4;

static void erase_lane_dashes(void) {
    for (int16_t y = -DASH_STEP + roadOffset; y < (int16_t)TFT_H; y += DASH_STEP) {
        if (y >= 0 && y + DASH_H > 0)
            Graphics::fill_rect(CENTER_X - DASH_W/2, y, DASH_W, DASH_H, ROAD_COLOR);
    }
}

static void draw_lane_dashes(void) {
    for (int16_t y = -DASH_STEP + roadOffset; y < (int16_t)TFT_H; y += DASH_STEP) {
        if (y >= 0)
            Graphics::fill_rect(CENTER_X - DASH_W/2, y, DASH_W, DASH_H, WHITE);
    }
}

static void draw_edge_lines(void) {
    Graphics::draw_vline(ROAD_LEFT - 2, 0, TFT_H, WHITE);
    Graphics::draw_vline(ROAD_RIGHT + 1, 0, TFT_H, WHITE);
}

static bool rect_on_screen(int16_t x, int16_t y, int16_t w, int16_t h) {
    return (x + w > 0 && x < (int16_t)TFT_W &&
            y + h > 0 && y < (int16_t)TFT_H);
}

static void erase_sprite(int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!rect_on_screen(x, y, w, h)) return;
    Graphics::fill_rect(x, y, w, h, ROAD_COLOR);
}

// ========== Init ==========
void init_game(void) {
    playerX = (TFT_W / 2) - (PLAYER_W / 2);
    playerY = PLAYER_BASE_Y;
    playerDir = 0;
    prevPlayerX = playerX;
    prevPlayerY = playerY;

    enemyCount = 0;
    for (int i = 0; i < 8; i++) {
        enemies[i].active = false;
        prevEnemyX[i] = prevEnemyY[i] = 0;
    }

    score = 0;
    lives = 3;
    level = 1;
    gameOver = false;

    enemySpeed = 2 + (level / 3);
    if (enemySpeed > 6) enemySpeed = 6;
    spawnInterval = 60 - (level * 2);
    if (spawnInterval < 25) spawnInterval = 25;
    spawnDelay = 0;
    roadOffset = 0;
}

// ========== Draw road ==========
void draw_road(void) {
    Graphics::fill_screen(BLACK);
    Graphics::fill_rect(ROAD_LEFT - 4, 0, ROAD_RIGHT - ROAD_LEFT + 8, TFT_H, ASPHALT_COLOR);
    Graphics::fill_rect(ROAD_LEFT, 0, ROAD_RIGHT - ROAD_LEFT, TFT_H, ROAD_COLOR);
    draw_edge_lines();
    draw_lane_dashes();
}

// ========== Update player ==========
void update_player(void) {
    prevPlayerX = playerX;
    prevPlayerY = playerY;

    if (!demoMode) {
#ifdef USE_BUTTONS
        if (BTN_LEFT)       playerDir = -1;
        else if (BTN_RIGHT) playerDir = 1;
        else                playerDir = 0;
#endif
    } else {
        int16_t targetDir = 0;
        int16_t avoidLeft = 0, avoidRight = 0;
        for (int i = 0; i < 8; i++) {
            if (!enemies[i].active) continue;
            int16_t dy = enemies[i].y - playerY;
            if (dy > 0 && dy < 60) {
                if (abs(enemies[i].x - playerX) < PLAYER_W + 2) {
                    if (enemies[i].x < playerX) avoidLeft = 1;
                    else avoidRight = 1;
                }
            }
        }
        if (avoidLeft && !avoidRight) targetDir = 1;
        else if (avoidRight && !avoidLeft) targetDir = -1;
        else {
            static uint8_t rnd = 0;
            if (++rnd > 30) { rnd = 0; targetDir = (rand() % 3) - 1; }
        }
        playerDir = targetDir;
    }

    int16_t newX = playerX + playerDir * 4;
    if (newX >= ROAD_LEFT && newX + PLAYER_W <= ROAD_RIGHT)
        playerX = newX;
}

// ========== Spawn enemy ==========
static void spawn_enemy(void) {
    if (enemyCount >= 8) return;
    for (int i = 0; i < 8; i++) {
        if (!enemies[i].active) {
            enemies[i].x = ROAD_LEFT + rand() % (ROAD_RIGHT - ROAD_LEFT - ENEMY_W + 1);
            enemies[i].y = -ENEMY_H;
            enemies[i].active = true;
            enemyCount++;
            break;
        }
    }
}

// ========== Update enemies ==========
void update_enemies(void) {
    for (int i = 0; i < 8; i++) {
        prevEnemyX[i] = enemies[i].x;
        prevEnemyY[i] = enemies[i].y;
        if (!enemies[i].active) continue;

        enemies[i].y += enemySpeed;

        if (enemies[i].y > TFT_H + ENEMY_H) {
            enemies[i].active = false;
            enemyCount--;
            score += 10;
            sound_eat();
        }
    }

    if (spawnDelay <= 0) {
        spawn_enemy();
        spawnDelay = spawnInterval + (rand() % 15);
    } else {
        spawnDelay--;
    }
}

// ========== Draw all sprites after road ==========
static void draw_all_sprites(void) {
    Graphics::drawSprite(playerX, playerY, playerCar, ROAD_COLOR);
    for (int i = 0; i < 8; i++) {
        if (enemies[i].active && rect_on_screen(enemies[i].x, enemies[i].y, ENEMY_W, ENEMY_H))
            Graphics::drawSprite(enemies[i].x, enemies[i].y, enemyCar, ROAD_COLOR);
    }
}

// ========== Collisions ==========
static bool pixel_collision(int x1, int y1, const Sprite& s1,
                            int x2, int y2, const Sprite& s2) {
    int left   = (x1 > x2) ? x1 : x2;
    int top    = (y1 > y2) ? y1 : y2;
    int right  = ((x1 + s1.w) < (x2 + s2.w)) ? (x1 + s1.w) : (x2 + s2.w);
    int bottom = ((y1 + s1.h) < (y2 + s2.h)) ? (y1 + s1.h) : (y2 + s2.h);
    if (left >= right || top >= bottom) return false;

    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            if (s1.isSolid(x - x1, y - y1) && s2.isSolid(x - x2, y - y2))
                return true;
        }
    }
    return false;
}

void check_collisions(void) {
    for (int i = 0; i < 8; i++) {
        if (!enemies[i].active) continue;

        if (pixel_collision(playerX, playerY, playerCar,
                            enemies[i].x, enemies[i].y, enemyCar)) {
            if (lives > 0) lives--;
            sound_death();

            for (int j = 0; j < 8; j++) {
                enemies[j].active = false;
            }
            enemyCount = 0;

            Graphics::drawSprite(playerX, playerY, playerCar, ROAD_COLOR);
            playerX = (TFT_W / 2) - (PLAYER_W / 2);
            playerY = PLAYER_BASE_Y;
            prevPlayerX = playerX;
            prevPlayerY = playerY;
            Graphics::drawSprite(playerX, playerY, playerCar, ROAD_COLOR);

            draw_status();
            delay_ms(1200);

            if (lives == 0) gameOver = true;
            return;
        }
    }

    if (score > 0 && score / 500 >= level) {
        level = score / 500 + 1;
        enemySpeed = 2 + (level / 3);
        if (enemySpeed > 8) enemySpeed = 8;
        spawnInterval = 60 - (level * 2);
        if (spawnInterval < 20) spawnInterval = 20;
        sound_ghost();
    }
}

// ========== HUD ==========
void draw_status(void) {
    static uint16_t lastScore = 0xFFFF;
    static uint8_t  lastLives = 0xFF, lastLevel = 0xFF;
    if (score == lastScore && lives == lastLives && level == lastLevel)
        return;
    lastScore = score; lastLives = lives; lastLevel = level;

    if (score > highScore) highScore = score;

    Graphics::fill_rect(0, HUD_Y, TFT_W, HUD_H, BLACK);

    char buf[28];
    snprintf(buf, sizeof(buf), "S:%u H:%u L%u", score, highScore, level);
    Graphics::draw_string(4, HUD_Y + 1, buf, YELLOW, BLACK, 1);

    uint8_t pos = TFT_W - 4;
    for (uint8_t i = 0; i < lives && i < 5; i++) {
        pos -= 14;
        Graphics::draw_car(pos, HUD_Y + 3, YELLOW, 8);
    }

    Graphics::draw_hline(0, HUD_Y - 1, TFT_W, COLOR565(0, 100, 255));
}

// ========== Title ==========
void show_title(void) {
    Graphics::fill_screen(BLACK);
    Graphics::draw_string(28, 8,  "ROAD RACER", YELLOW, BLACK, 3);
    Graphics::draw_string(38, 40, "Esquiva Autos", CYAN,   BLACK, 2);

    Graphics::drawSprite(95, 80, playerCar, BLACK);
    Graphics::drawSprite(95, 140, enemyCar, BLACK);

    Graphics::draw_string(8,  200, "<-  -> esquiva",     WHITE,  BLACK, 1);
    Graphics::draw_string(28, 215, "PASA AUTOS = +10",   YELLOW, BLACK, 1);
    Graphics::draw_string(30, 228, "EMPIEZA EN BREVE",   GRAY,   BLACK, 1);

    sound_start();
    delay_ms(3000);
    Graphics::fill_screen(BLACK);
}

// ========== Game Over ==========
void show_game_over(void) {
    Graphics::fill_screen(BLACK);
    Graphics::draw_string((TFT_W - 108) / 2, TFT_H / 2 - 40,
                           "GAME OVER", RED, BLACK, 2);
    char buf[20];
    snprintf(buf, sizeof(buf), "SCORE: %u", score);
    Graphics::draw_string((TFT_W - 96) / 2, TFT_H / 2 - 10,
                           buf, YELLOW, BLACK, 2);
    snprintf(buf, sizeof(buf), "BEST:  %u", highScore);
    Graphics::draw_string((TFT_W - 96) / 2, TFT_H / 2 + 14,
                           buf, CYAN, BLACK, 2);
    Graphics::draw_string((TFT_W - 78) / 2, TFT_H / 2 + 46,
                           "PRESS RESET", WHITE, BLACK, 1);
    sound_death();
}

// ========== Game Loop ==========
void game_loop(bool demo) {
    demoMode = demo;

    if (!load_sprites()) {
        fprintf(stderr, "FATAL: No se pudieron cargar los sprites.\n");
        return;
    }

    show_title();
    highScore = 0;

restart:
    init_game();
    draw_road();
    draw_status();
    draw_all_sprites();

    struct timespec ts_last, ts_now;
    clock_gettime(CLOCK_MONOTONIC, &ts_last);

    while (true) {
        // Phase 1: save old positions, calculate new positions
        update_player();
        update_enemies();

        // Phase 2: erase old sprite positions
        erase_sprite(prevPlayerX, prevPlayerY, PLAYER_W, PLAYER_H);
        for (int i = 0; i < 8; i++) {
            if (prevEnemyX[i] != 0 || enemies[i].active)
                erase_sprite(prevEnemyX[i], prevEnemyY[i], ENEMY_W, ENEMY_H);
        }

        // Phase 3: erase old lane dashes, advance offset, draw new dashes + edge lines
        erase_lane_dashes();
        roadOffset = (roadOffset + roadSpeed) % DASH_STEP;
        draw_lane_dashes();
        draw_edge_lines();

        // Phase 4: draw all sprites
        draw_all_sprites();

        check_collisions();

        if (gameOver) {
            show_game_over();
            delay_ms(5000);
            goto restart;
        }

        draw_status();

        uint32_t frame_us = 33000;
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        long elapsed_us = (ts_now.tv_sec  - ts_last.tv_sec) * 1000000L
                        + (ts_now.tv_nsec - ts_last.tv_nsec) / 1000L;
        if (elapsed_us < (long)frame_us)
            delay_us((uint32_t)(frame_us - elapsed_us));
        clock_gettime(CLOCK_MONOTONIC, &ts_last);
    }
}

} // namespace GameEngine
