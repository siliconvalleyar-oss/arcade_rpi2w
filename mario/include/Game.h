#ifndef GAME_H
#define GAME_H

#include "Level.h"
#include "Player.h"
#include "Enemy.h"
#include <vector>

enum GameMode { MODE_DEMO, MODE_PLAY };

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    Level level;
    Player* player;
    std::vector<Entity*> entities;
    bool running;
    int coinFrame;
    GameMode gameMode;

    void update(float dt);
    void draw();
    void redrawTilesInRect(int x, int y, int w, int h);
    void checkCollisions();
    void showTitleScreen();
    void showModeSelection();
};

#endif
