#include "../include/Game.h"
#include "../include/Renderer.h"
#include "../include/Sound.h"
#include "../include/Physics.h"
#include "../include/Graphics.h"
#include "../include/HardwareProfile.h"
#include "../include/Sprite.h"
#include <cmath>
#include <unistd.h>

Game::Game() : player(nullptr), running(true), coinFrame(0), gameMode(MODE_DEMO) {
    player = new Player(32, 32);
    entities.push_back(player);
    entities.push_back(new Enemy(150, 150, EntityType::GOOMBA));
}

Game::~Game() {
    for (auto e : entities) delete e;
}

void Game::showTitleScreen() {
    Graphics::fill_screen(BLACK);
    Sprite title;
    if (title.load("assets/super_mario_presentacion.png")) {
        int x = (TFT_W - title.w) / 2;
        int y = (TFT_H - title.h) / 2;
        Graphics::drawSprite(x, y, title, BLACK);
    } else {
        Renderer::drawText(40, 100, "MARIO BROS", RED, 2);
    }
    sound_start();
    delay_ms(3000);
}

void Game::showModeSelection() {
    Graphics::fill_screen(BLACK);
    Renderer::drawText(30, 60, "SELECCIONA MODO", WHITE, 1);

    int selected = 0; // 0=DEMO, 1=JUEGO
    uint32_t elapsed = 0;
    const uint32_t TIMEOUT = 3000;
    bool prevUp = true, prevDown = true;

    while (elapsed < TIMEOUT) {
        bool up = BTN_UP;
        bool down = BTN_DOWN;

        if (up && !prevUp) {
            selected = 1;
            sound_beep(880, 50);
            break;
        }
        if (down && !prevDown) {
            selected = 0;
            sound_beep(660, 50);
            break;
        }
        prevUp = up;
        prevDown = down;

        const char* mode1 = (selected == 0) ? "> DEMO" : "  DEMO";
        const char* mode2 = (selected == 1) ? "> JUEGO" : "  JUEGO";
        Graphics::fill_rect(50, 90, 140, 60, BLACK);
        Renderer::drawText(60, 100, mode1, (selected == 0) ? YELLOW : GRAY, 2);
        Renderer::drawText(60, 130, mode2, (selected == 1) ? YELLOW : GRAY, 2);

        delay_ms(50);
        elapsed += 50;
    }

    gameMode = (selected == 1) ? MODE_PLAY : MODE_DEMO;
    if (gameMode == MODE_DEMO) {
        Graphics::fill_screen(BLACK);
        Renderer::drawText(40, 100, "MODO DEMO", GRAY, 2);
        delay_ms(1000);
    } else {
        Graphics::fill_screen(BLACK);
        Renderer::drawText(40, 100, "MODO JUEGO", GREEN, 2);
        delay_ms(1000);
    }
}

void Game::update(float dt) {
    for (auto e : entities) {
        if (e->active)
            e->update(dt, level.tiles);
    }

    // Colisiones jugador-enemigo
    for (auto e : entities) {
        if (e->type == EntityType::GOOMBA && e->active) {
            Enemy* enemy = static_cast<Enemy*>(e);
            if (Physics::AABBvsAABB(player->getAABB(), enemy->getAABB())) {
                if (player->vy > 0 && player->y + player->height < enemy->y + enemy->height/2) {
                    enemy->stomp();
                    player->vy = -3.0f;
                } else {
                    player->takeDamage();
                }
            }
        }
    }

    // Eliminar enemigos inactivos
    for (auto it = entities.begin(); it != entities.end(); ) {
        if (!(*it)->active && (*it)->type != EntityType::PLAYER) {
            delete *it;
            it = entities.erase(it);
        } else ++it;
    }

    if (!player->active) running = false;

    if (gameMode == MODE_DEMO) {
        // Modo automático
        if (player->active) {
            player->move(Direction::RIGHT);
            bool shouldJump = false;
            float sensorX = player->x + player->width + 5;
            float sensorY = player->y + player->height / 2;
            int tileX = (int)(sensorX / TILE_SIZE);
            int tileY = (int)(sensorY / TILE_SIZE);
            if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
                TileType tile = (TileType)level.tiles[tileX][tileY];
                if (tile != TileType::EMPTY && tile != TileType::COIN_BRICK)
                    shouldJump = true;
            }
            for (auto e : entities) {
                if (e->type == EntityType::GOOMBA && e->active) {
                    if (e->x > player->x && e->x < player->x + 50 &&
                        std::abs(e->y - player->y) < 30) {
                        shouldJump = true;
                        break;
                    }
                }
            }
            if (shouldJump) player->jump();
        }
    } else {
        // Modo jugable — leer botones
        if (player->active) {
            if (BTN_LEFT) {
                player->move(Direction::LEFT);
            } else if (BTN_RIGHT) {
                player->move(Direction::RIGHT);
            } else {
                player->stopMove();
            }
            if (BTN_UP) player->jump();
        }
    }
}

void Game::redrawTilesInRect(int x, int y, int w, int h) {
    int startCol = x / TILE_SIZE;
    int endCol   = (x + w) / TILE_SIZE;
    int startRow = y / TILE_SIZE;
    int endRow   = (y + h) / TILE_SIZE;
    
    for (int row = startRow; row <= endRow; ++row) {
        for (int col = startCol; col <= endCol; ++col) {
            if (col >= 0 && col < MAP_WIDTH && row >= 0 && row < MAP_HEIGHT) {
                TileType t = (TileType)level.tiles[col][row];
                if (t != TileType::EMPTY) {
                    Renderer::drawTile(col * TILE_SIZE, row * TILE_SIZE, t);
                }
            }
        }
    }
}

void Game::draw() {
    // Restaurar fondo en posiciones anteriores de entidades activas
    for (auto e : entities) {
        if (!e->active) continue;
        int px = (int)e->prevX;
        int py = (int)e->prevY;
        int w = (int)e->width;
        int h = (int)e->height;
        
        Graphics::fill_rect(px, py, w, h, SKY_BLUE);
        redrawTilesInRect(px, py, w, h);
    }
    
    // Dibujar entidades en posiciones actuales
    for (auto e : entities) {
        if (e->active) e->draw();
    }
    
    // Actualizar posiciones previas
    for (auto e : entities) {
        e->prevX = e->x;
        e->prevY = e->y;
    }
    
    // HUD
    Renderer::drawText(2, 2, "MARIO", WHITE, 1);
    Renderer::drawNumber(50, 2, player->coins, YELLOW);
    Renderer::drawText(80, 2, "LIVES", WHITE);
    Renderer::drawNumber(130, 2, player->lives, RED);
}

void Game::run() {
    showTitleScreen();
    showModeSelection();

    Graphics::fill_screen(SKY_BLUE);
    level.draw();

    const int frameDelay = 16;
    while (running) {
        update(frameDelay / 1000.0f);
        draw();
        delay_ms(frameDelay);
    }

    Graphics::fill_screen(BLACK);
    Renderer::drawText(80, 100, "GAME OVER", RED, 2);
    delay_ms(2000);
}
