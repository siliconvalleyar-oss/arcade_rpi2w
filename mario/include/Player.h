#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "AnimatedSprite.h"

class Player : public Entity {
public:
    Player(float x, float y);
    virtual ~Player();

    void update(float dt, const uint8_t tileMap[MAP_WIDTH][MAP_HEIGHT]) override;
    void draw() const override;

    void jump();
    void move(Direction dir);
    void stopMove();
    void takeDamage();

    int coins;
    int lives;
    bool big;
    float invincibleTimer;

private:
    Direction moveDir;
    bool jumpPressed;
    float jumpBuffer;
    
    AnimatedSprite animIdle;   // <-- Asegurar que estén declarados
    AnimatedSprite animRun;
    AnimatedSprite animJump;
    AnimatedSprite* currentAnim;
    Sprite currentSprite;
};

#endif
