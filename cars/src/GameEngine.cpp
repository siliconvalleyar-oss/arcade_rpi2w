// ============================================================
//  GameEngine.cpp — Auto Esquiva (Road Racer) para RPi + ST7789
//  Basado en la estructura del Pac-Man original.
//  Modo manual (botones izquierda/derecha) o demo automático.
// ============================================================

#include "../include/GameEngine.h"
#include "../include/Sound.h"
#include <cstdio>
#include <stdlib.h>
#include <time.h>

namespace GameEngine {

// ========== Dimensiones y constantes ==========
const int16_t PLAYER_W = 12;
const int16_t PLAYER_H = 12;
const int16_t ENEMY_W  = 12;
const int16_t ENEMY_H  = 12;

const int16_t ROAD_LEFT  = 20;
const int16_t ROAD_RIGHT = TFT_W - 20;
const int16_t PLAYER_BASE_Y = TFT_H - 30;

// Velocidades iniciales
int16_t enemySpeed = 2;      // píxeles por frame
int16_t spawnDelay = 0;      // contador para crear nuevos enemigos
int16_t spawnInterval = 40;  // frames entre spawns (disminuye con nivel)

// ========== Variables del juego ==========
int16_t playerX, playerY;
int8_t  playerDir;            // -1 izquierda, 1 derecha, 0 quieto
bool    demoMode = false;

Enemy enemies[8];
uint8_t enemyCount = 0;

uint16_t score = 0;
uint16_t highScore = 0;
uint8_t  lives = 3;
uint8_t  level = 1;
bool     gameOver = false;

// Variables auxiliares para anti‑flicker
static int16_t prevPlayerX, prevPlayerY;
static int16_t prevEnemyX[8], prevEnemyY[8];

// ========== Inicialización ==========
void init_game(void) {
    // Jugador
    playerX = (TFT_W / 2) - (PLAYER_W / 2);
    playerY = PLAYER_BASE_Y;
    playerDir = 0;
    prevPlayerX = playerX;
    prevPlayerY = playerY;

    // Enemigos
    enemyCount = 0;
    for (int i = 0; i < 8; i++) {
        enemies[i].active = false;
        prevEnemyX[i] = prevEnemyY[i] = 0;
    }

    score = 0;
    lives = 3;
    level = 1;
    gameOver = false;

    // Ajuste de velocidad según nivel
    enemySpeed = 2 + (level / 3);
    if (enemySpeed > 6) enemySpeed = 6;
    spawnInterval = 40 - (level * 2);
    if (spawnInterval < 15) spawnInterval = 15;
    spawnDelay = 0;
}

// ========== Dibujar carretera ==========
void draw_road(void) {
    // Fondo gris oscuro (asfalto)
    Graphics::fill_rect(ROAD_LEFT - 4, 0, ROAD_RIGHT - ROAD_LEFT + 8, TFT_H, COLOR565(40,40,40));
    // Asfalto principal
    Graphics::fill_rect(ROAD_LEFT, 0, ROAD_RIGHT - ROAD_LEFT, TFT_H, COLOR565(20,20,20));
    // Líneas laterales blancas
    Graphics::draw_vline(ROAD_LEFT - 2, 0, TFT_H, WHITE);
    Graphics::draw_vline(ROAD_RIGHT + 1, 0, TFT_H, WHITE);
    // Líneas de carril discontinuas (cada 20 píxeles)
    for (int y = 0; y < TFT_H; y += 20) {
        Graphics::fill_rect(ROAD_LEFT + (ROAD_RIGHT-ROAD_LEFT)/2 - 2, y, 4, 10, WHITE);
    }
}

// ========== Actualizar jugador (manual o demo) ==========
void update_player(void) {
    prevPlayerX = playerX;
    prevPlayerY = playerY;

    if (!demoMode) {
        // Modo manual: botones izquierda/derecha
#ifdef USE_BUTTONS
        if (BTN_LEFT)  playerDir = -1;
        else if (BTN_RIGHT) playerDir = 1;
        else playerDir = 0;
#endif
    } else {
        // ----- IA simple para demo -----
        // Busca el enemigo más cercano que esté en el mismo rango vertical
        int16_t dangerLeft = 0, dangerRight = 0;
        int16_t minDist = 1000;
        int16_t targetX = playerX;
        for (int i = 0; i < 8; i++) {
            if (!enemies[i].active) continue;
            int16_t dy = enemies[i].y - playerY;
            if (dy > 0 && dy < 60) {  // enemigo por delante y cercano
                if (abs(enemies[i].x - playerX) < PLAYER_W) {
                    // Colisión inminente: huir hacia el lado más despejado
                    if (enemies[i].x < playerX) dangerLeft = 1;
                    else dangerRight = 1;
                }
                if (abs(enemies[i].x - playerX) < minDist) {
                    minDist = abs(enemies[i].x - playerX);
                    targetX = enemies[i].x;
                }
            }
        }
        if (dangerLeft && !dangerRight) playerDir = 1;   // huir a la derecha
        else if (dangerRight && !dangerLeft) playerDir = -1;
        else if (!dangerLeft && !dangerRight) {
            // Movimiento aleatorio suave
            static uint8_t rnd = 0;
            if (++rnd > 30) {
                rnd = 0;
                if (rand() % 3 == 0) playerDir = (rand() % 3) - 1;
            }
        }
    }

    // Aplicar movimiento
    int16_t newX = playerX + playerDir * 4;
    if (newX >= ROAD_LEFT && newX + PLAYER_W <= ROAD_RIGHT)
        playerX = newX;
}

// ========== Crear nuevo enemigo ==========
void spawn_enemy(void) {
    if (enemyCount >= 8) return;
    for (int i = 0; i < 8; i++) {
        if (!enemies[i].active) {
            int16_t x = ROAD_LEFT + rand() % (ROAD_RIGHT - ROAD_LEFT - ENEMY_W);
            enemies[i].x = x;
            enemies[i].y = -ENEMY_H;
            enemies[i].active = true;
            enemyCount++;
            break;
        }
    }
}

// ========== Actualizar enemigos ==========
void update_enemies(void) {
    // Mover enemigos
    for (int i = 0; i < 8; i++) {
        if (!enemies[i].active) continue;
        prevEnemyX[i] = enemies[i].x;
        prevEnemyY[i] = enemies[i].y;

        enemies[i].y += enemySpeed;

        // Si sale de la pantalla, sumar puntos y desactivar
        if (enemies[i].y > TFT_H) {
            enemies[i].active = false;
            enemyCount--;
            score += 10;
            sound_eat();  // reutilizamos sonido de "comer"
        }
    }

    // Spawn controlado
    if (spawnDelay <= 0) {
        spawn_enemy();
        spawnDelay = spawnInterval;
    } else {
        spawnDelay--;
    }
}

// ========== Colisiones ==========
void check_collisions(void) {
    for (int i = 0; i < 8; i++) {
        if (!enemies[i].active) continue;
        // Colisión rectángulo vs rectángulo
        if (playerX < enemies[i].x + ENEMY_W &&
            playerX + PLAYER_W > enemies[i].x &&
            playerY < enemies[i].y + ENEMY_H &&
            playerY + PLAYER_H > enemies[i].y) {

            if (lives > 0) {
                lives--;
                sound_death();

                // Reiniciar posición de todos los enemigos
                for (int j = 0; j < 8; j++) {
                    enemies[j].active = false;
                }
                enemyCount = 0;
                playerX = (TFT_W / 2) - (PLAYER_W / 2);
                playerY = PLAYER_BASE_Y;
                draw_road();
                draw_status();
                Graphics::draw_car(playerX, playerY, YELLOW, PLAYER_W);
                delay_ms(1000);

                if (lives == 0) gameOver = true;
            }
            return;
        }
    }

    // Subir nivel cada 500 puntos
    if (score / 500 > level - 1) {
        level = score / 500 + 1;
        enemySpeed = 2 + (level / 3);
        if (enemySpeed > 8) enemySpeed = 8;
        spawnInterval = 40 - (level * 2);
        if (spawnInterval < 12) spawnInterval = 12;
        sound_ghost();  // sonido de "power pellet" usado para subir nivel
    }
}

// ========== Dibujar HUD ==========
void draw_status(void) {
    static uint16_t lastScore = 0xFFFF;
    static uint8_t lastLives = 0xFF, lastLevel = 0xFF;
    if (score == lastScore && lives == lastLives && level == lastLevel) return;
    lastScore = score; lastLives = lives; lastLevel = level;

    if (score > highScore) highScore = score;

    Graphics::fill_rect(0, HUD_Y, TFT_W, HUD_H, BLACK);

    // Vidas representadas con autitos mini (escala 0.5)
    for (uint8_t i = 0; i < lives && i < 5; i++)
        Graphics::draw_car(8 + i*14, HUD_Y + HUD_H/2 - 6, YELLOW, 8);

    char buf[28];
    sprintf(buf, "S:%u H:%u L%u", score, highScore, level);
    Graphics::draw_string(52, HUD_Y+2, buf, YELLOW, BLACK, 1);
    Graphics::draw_hline(0, HUD_Y-1, TFT_W, WALL_EDGE);
}

// ========== Pantalla de título ==========
void show_title(void) {
    Graphics::fill_screen(BLACK);
    Graphics::draw_string(28, 8,  "ROAD RACER", YELLOW, BLACK, 3);
    Graphics::draw_string(38, 40, "Esquiva Autos", CYAN,   BLACK, 2);

    // Auto grande centrado
    Graphics::draw_car(110, 100, YELLOW, 20);
    // Enemigo grande
    Graphics::draw_car(110, 150, RED, 20);

    Graphics::draw_string(8,  200, "← →  esquiva enemigos", WHITE,  BLACK, 1);
    Graphics::draw_string(28, 224, "Demo activo por defecto", GRAY,   BLACK, 1);

    sound_start();
    delay_ms(3000);
    Graphics::fill_screen(BLACK);
}

// ========== Game Over ==========
void show_game_over(void) {
    Graphics::fill_screen(BLACK);
    Graphics::draw_string((TFT_W-108)/2, TFT_H/2-40, "GAME OVER", RED, BLACK, 2);
    char buf[20];
    sprintf(buf, "SCORE: %u",   score);
    Graphics::draw_string((TFT_W-96)/2, TFT_H/2-10,  buf, YELLOW,BLACK,2);
    sprintf(buf, "BEST:  %u",   highScore);
    Graphics::draw_string((TFT_W-96)/2, TFT_H/2+14,  buf, CYAN,  BLACK,2);
    Graphics::draw_string((TFT_W-78)/2, TFT_H/2+46, "PRESS RESET", WHITE, BLACK, 1);
    sound_death();
}

// ========== Bucle principal ==========
void game_loop(bool demo) {
    demoMode = demo;

    show_title();

restart_game:
    init_game();
    draw_road();
    draw_status();
    Graphics::draw_car(playerX, playerY, YELLOW, PLAYER_W);

    struct timespec ts_last, ts_now;
    clock_gettime(CLOCK_MONOTONIC, &ts_last);

    while (true) {
        // Actualizaciones
        update_player();
        update_enemies();
        check_collisions();

        if (gameOver) {
            show_game_over();
            while (1);
        }

        // ----- Redibujado con anti‑flicker -----
        // Borrar área del jugador anterior
        Graphics::fill_rect(prevPlayerX, prevPlayerY, PLAYER_W, PLAYER_H, COLOR565(20,20,20));
        // Borrar área de cada enemigo anterior
        for (int i = 0; i < 8; i++) {
            if (prevEnemyX[i] != 0 || prevEnemyY[i] != 0) {
                Graphics::fill_rect(prevEnemyX[i], prevEnemyY[i], ENEMY_W, ENEMY_H, COLOR565(20,20,20));
            }
        }

        // Dibujar líneas de carril (para cubrir huecos)
        for (int y = 0; y < TFT_H; y += 20) {
            Graphics::fill_rect(ROAD_LEFT + (ROAD_RIGHT-ROAD_LEFT)/2 - 2, y, 4, 10, WHITE);
        }

        // Dibujar jugador
        Graphics::draw_car(playerX, playerY, YELLOW, PLAYER_W);
        // Dibujar enemigos
        for (int i = 0; i < 8; i++) {
            if (enemies[i].active) {
                Graphics::draw_car(enemies[i].x, enemies[i].y, RED, ENEMY_W);
                prevEnemyX[i] = enemies[i].x;
                prevEnemyY[i] = enemies[i].y;
            } else {
                prevEnemyX[i] = prevEnemyY[i] = 0;
            }
        }

        draw_status();

        // Frame timing (30 fps aprox, pero ajustable)
        uint32_t frame_us = 33000;  // ~30 fps
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        long elapsed_us = (ts_now.tv_sec  - ts_last.tv_sec) * 1000000L
                        + (ts_now.tv_nsec - ts_last.tv_nsec) / 1000L;
        if (elapsed_us < (long)frame_us)
            delay_us((uint32_t)(frame_us - elapsed_us));
        clock_gettime(CLOCK_MONOTONIC, &ts_last);
    }
}

} // namespace GameEngine