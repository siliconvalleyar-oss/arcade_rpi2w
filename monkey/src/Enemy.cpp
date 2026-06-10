#include "../include/Enemy.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

// ============================================================
//  Barrel
// ============================================================
Barrel::Barrel() : Entity(Type::BARREL)
{
    active = false;
}

void Barrel::init(Sound* snd, const Physics* phys,
                  const SpriteData* roll1,
                  const SpriteData* roll2,
                  const SpriteData* explode)
{
    sound_   = snd;
    phys_    = phys;
    sprites_.resize(3, nullptr);
    sprites_[0] = roll1;
    sprites_[1] = roll2;
    sprites_[2] = explode;
    active = false;
}

void Barrel::launch(Vec2f startPos, float speedX)
{
    pos         = startPos;
    vel         = { speedX, -20.f };   // pequeño impulso vertical al lanzar
    dirX_       = (speedX > 0.f) ? 1.f : -1.f;
    active      = true;
    alive       = true;
    exploding_  = false;
    explodeTimer_= 0;
    fallCount_  = 0;
    currentFrame_= 0;
}

void Barrel::update(int dt_ms)
{
    if (!active) return;

    if (exploding_) {
        explodeTimer_ -= dt_ms;
        currentFrame_ = 2;
        if (explodeTimer_ <= 0) {
            active = false;
            alive  = false;
        }
        updateBounds();
        return;
    }

    if (!phys_ || !platforms_) {
        // Sin física: solo mover
        pos.x += vel.x * dt_ms * 0.001f;
        updateBounds();
        return;
    }

    const bool wasGrounded = grounded;
    auto res = phys_->integrateBarrel(pos, vel, *platforms_, dt_ms);
    grounded = res.hitFloor;

    if (grounded && !wasGrounded) {
        ++fallCount_;
        // El barril rueda en la dirección actual
        if (res.hitRight || res.hitLeft) {
            dirX_ = -dirX_;
            vel.x = dirX_ * std::abs(vel.x);
        }
        if (sound_) sound_->play(SFX::BARREL_ROLL);
    }

    // Animación de rodado: alternar roll1/roll2 cada 100ms
    rollAngle_ += dt_ms;
    currentFrame_ = (rollAngle_ / 100) % 2;

    // Caer fuera de pantalla = destruir
    if (pos.y > SCREEN_H + 20) {
        active = false;
        alive  = false;
    }

    updateBounds();
}

void Barrel::explode()
{
    if (exploding_) return;
    exploding_    = true;
    explodeTimer_ = 400;
    vel           = {0.f, 0.f};
    currentFrame_ = 2;
    if (sound_) sound_->play(SFX::BARREL_EXPLODE);
}

void Barrel::draw(Framebuffer& fb) const
{
    if (!active) return;

    const SpriteData* spr = sprites_[currentFrame_];
    if (!spr || !spr->valid()) {
        // Placeholder: círculo marrón
        fb.fillRect(static_cast<int>(pos.x),
                    static_cast<int>(pos.y),
                    12, 12,
                    exploding_ ? Colors::ORANGE : Colors::BROWN);
        return;
    }
    fb.blitSprite(*spr, static_cast<int>(pos.x), static_cast<int>(pos.y));
}

void Barrel::onCollision(Entity& /*other*/) {}

// ============================================================
//  Flame
// ============================================================
Flame::Flame() : Entity(Type::FLAME)
{
    active = false;
}

void Flame::init(Sound* snd, const Physics* phys,
                 const SpriteData* f1, const SpriteData* f2)
{
    sound_ = snd;
    phys_  = phys;
    sprites_.resize(2, nullptr);
    sprites_[0] = f1;
    sprites_[1] = f2;
    active = false;
}

void Flame::spawn(Vec2f p)
{
    pos       = p;
    vel       = { dirX_ * 40.f, 0.f };
    active    = true;
    alive     = true;
    thinkTimer_= 0;
    currentFrame_ = 0;
}

void Flame::update(int dt_ms)
{
    if (!active) return;

    // IA simple: buscar posición del jugador (no disponible aquí,
    // así que aleatoriamente cambia dirección cada cierto tiempo)
    thinkTimer_ -= dt_ms;
    if (thinkTimer_ <= 0) {
        thinkTimer_ = 800 + (std::rand() % 600);
        // Alternar dirección
        dirX_ = -dirX_;
        vel.x = dirX_ * 40.f;
    }

    if (phys_ && platforms_) {
        auto res = phys_->integrate(pos, vel, 10, 12, *platforms_, dt_ms, false);
        grounded = res.hitFloor;
        if (res.hitLeft || res.hitRight) {
            dirX_ = -dirX_;
            vel.x = dirX_ * 40.f;
        }
    } else {
        pos.x += vel.x * dt_ms * 0.001f;
    }

    // Animación
    animTimer_ += dt_ms;
    currentFrame_ = (animTimer_ / 120) % 2;

    // Límites
    if (pos.x < 0.f || pos.x > SCREEN_W - 10) {
        dirX_ = -dirX_;
        vel.x = dirX_ * 40.f;
    }
    if (pos.y > SCREEN_H) { active = false; alive = false; }

    updateBounds();
}

void Flame::draw(Framebuffer& fb) const
{
    if (!active) return;

    const SpriteData* spr = sprites_[currentFrame_];
    if (!spr || !spr->valid()) {
        // Placeholder: cuadrado naranja/rojo
        const Color16 c = (currentFrame_ == 0) ? Colors::ORANGE : Colors::RED;
        fb.fillRect(static_cast<int>(pos.x), static_cast<int>(pos.y), 10, 12, c);
        return;
    }
    fb.blitSprite(*spr, static_cast<int>(pos.x), static_cast<int>(pos.y));
}

// ============================================================
//  DonkeyKong
// ============================================================
DonkeyKong::DonkeyKong() : Entity(Type::DONKEYKONG) {}

void DonkeyKong::init(Sound* snd,
                       const SpriteData* idle,
                       const SpriteData* throw_,
                       const SpriteData* roar,
                       const SpriteData* stun)
{
    sound_ = snd;
    sprites_.resize(4, nullptr);
    sprites_[0] = idle;    // DKState::IDLE
    sprites_[1] = throw_;  // DKState::THROW
    sprites_[2] = roar;    // DKState::ROAR
    sprites_[3] = stun;    // DKState::STUN
    hp_    = 3;
    state_ = DKState::IDLE;
    active = true;
    alive  = true;
}

void DonkeyKong::update(int dt_ms)
{
    if (!active) return;
    stateTimer_  += dt_ms;
    throwTimer_  += dt_ms;
    wantsThrow_   = false;

    switch (state_) {
        case DKState::IDLE:
            currentFrame_ = 0;
            if (throwTimer_ >= throwInterval_) {
                throwTimer_ = 0;
                state_      = DKState::THROW;
                stateTimer_ = 0;
            }
            break;

        case DKState::THROW:
            currentFrame_ = 1;
            if (stateTimer_ >= 400) {
                wantsThrow_ = true;
                throwPos_   = { pos.x + 20.f, pos.y + 20.f };
                state_      = DKState::IDLE;
                stateTimer_ = 0;
                if (sound_) sound_->play(SFX::BOSS_ROAR);
            }
            break;

        case DKState::ROAR:
            currentFrame_ = 2;
            if (stateTimer_ >= 600) {
                state_ = DKState::IDLE;
                stateTimer_ = 0;
            }
            break;

        case DKState::STUN:
            currentFrame_ = 3;
            if (stateTimer_ >= 1200) {
                state_ = DKState::IDLE;
                stateTimer_ = 0;
            }
            break;

        case DKState::DEAD:
            currentFrame_ = 3;
            // Animación de caída - manejado por Game
            pos.y += 60.f * dt_ms * 0.001f;
            break;
    }

    updateBounds();
}

void DonkeyKong::draw(Framebuffer& fb) const
{
    if (!active) return;

    const SpriteData* spr = sprites_[currentFrame_];
    if (!spr || !spr->valid()) {
        // Placeholder: rectángulo marrón grande
        const Color16 c = (state_ == DKState::STUN || state_ == DKState::DEAD)
                          ? Colors::GRAY : Colors::DARKBROWN;
        fb.fillRect(static_cast<int>(pos.x), static_cast<int>(pos.y),
                    32, 32, c);
        // Ojos simples
        fb.fillRect(static_cast<int>(pos.x) + 6,  static_cast<int>(pos.y) + 6,  4, 4, Colors::WHITE);
        fb.fillRect(static_cast<int>(pos.x) + 22, static_cast<int>(pos.y) + 6,  4, 4, Colors::WHITE);
        return;
    }
    fb.blitSprite(*spr, static_cast<int>(pos.x), static_cast<int>(pos.y));
}

void DonkeyKong::hit(int damage)
{
    if (state_ == DKState::DEAD) return;
    hp_ -= damage;
    if (hp_ <= 0) {
        state_      = DKState::DEAD;
        stateTimer_ = 0;
        alive       = false;
        if (sound_) sound_->play(SFX::LEVEL_CLEAR);
    } else {
        state_      = DKState::STUN;
        stateTimer_ = 0;
        if (sound_) sound_->play(SFX::HAMMER_HIT);
    }
}
