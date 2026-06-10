#include "../include/Player.h"
#include "../include/Level.h"
#include <algorithm>
#include <cstdio>

// ============================================================
//  Player
// ============================================================

Player::Player() : Entity(Type::PLAYER) {}

void Player::init(Sound* snd, const Physics* phys)
{
    sound_   = snd;
    physics_ = phys;
    lives_   = 3;
    score_   = 0;
    state_   = PlayerState::IDLE;
    alive    = true;
    active   = true;
}

void Player::loadSprites(
    const SpriteData* idle,
    const SpriteData* walkA,
    const SpriteData* walkB,
    const SpriteData* jump,
    const SpriteData* climb,
    const SpriteData* hammer,
    const SpriteData* dead)
{
    sprites_.clear();
    sprites_.resize(IDX_COUNT, nullptr);
    sprites_[IDX_IDLE]   = idle;
    sprites_[IDX_WALKA]  = walkA;
    sprites_[IDX_WALKB]  = walkB;
    sprites_[IDX_JUMP]   = jump;
    sprites_[IDX_CLIMB]  = climb;
    sprites_[IDX_HAMMER] = hammer;
    sprites_[IDX_DEAD]   = dead;

    // Frame inicial
    currentFrame_ = IDX_IDLE;
}

// ============================================================
//  Update principal
// ============================================================
void Player::update(int dt_ms)
{
    if (!active) return;

    stateTimer_ += dt_ms;

    // Actualizar timers
    if (hammerTimer_    > 0) hammerTimer_    -= dt_ms;
    if (deathTimer_     > 0) deathTimer_     -= dt_ms;
    if (invincibleTimer_> 0) invincibleTimer_-= dt_ms;

    if (state_ == PlayerState::DEAD) {
        // Esperar animación de muerte
        tickAnimation(dt_ms);
        if (deathTimer_ <= 0) {
            alive = false;   // El Game manager detecta esto y resta vida
        }
        return;
    }

    if (state_ == PlayerState::WIN) return;

    applyInput(dt_ms);
    updateState(dt_ms);
    tickAnimation(dt_ms);

    // Actualizar bounds para colisiones
    updateBounds();
}

// ============================================================
//  Aplicar input -> velocidad
// ============================================================
void Player::applyInput(int dt_ms)
{
    if (!physics_ || !platforms_) return;
    const float dt = dt_ms * 0.001f;
    (void)dt;

    const float speed = (hammerTimer_ > 0)
                        ? PhysConst::WALK_SPEED + PhysConst::HAMMER_SPEED
                        : PhysConst::WALK_SPEED;

    // ---- Movimiento horizontal ----
    if (state_ != PlayerState::CLIMB) {
        if (input_.left) {
            vel.x = -speed;
            facingRight_ = false;
        } else if (input_.right) {
            vel.x = speed;
            facingRight_ = true;
        } else {
            // Fricción rápida
            vel.x *= 0.7f;
            if (std::abs(vel.x) < 2.f) vel.x = 0.f;
        }
    }

    // ---- Escalera ----
    if (onLadder_) {
        if (input_.up) {
            vel.y = -PhysConst::LADDER_SPEED;
            state_ = PlayerState::CLIMB;
        } else if (input_.down) {
            vel.y = PhysConst::LADDER_SPEED;
            state_ = PlayerState::CLIMB;
        } else if (state_ == PlayerState::CLIMB) {
            vel.y = 0.f;
        }
    } else if (state_ == PlayerState::CLIMB) {
        // Salimos de la escalera
        state_ = PlayerState::IDLE;
        vel.y  = 0.f;
    }

    // ---- Salto ----
    if (input_.jumpBtn && grounded && state_ != PlayerState::CLIMB) {
        vel.y = PhysConst::JUMP_SPEED;
        grounded = false;
        justJumped_ = true;
        state_ = PlayerState::JUMP;
        if (sound_) sound_->play(SFX::JUMP);
    }

    // ---- Integrar física ----
    if (platforms_) {
        bool climbing = (state_ == PlayerState::CLIMB);
        auto res = physics_->integrate(pos, vel,
                                        sprites_[IDX_IDLE] ? sprites_[IDX_IDLE]->w : 12,
                                        sprites_[IDX_IDLE] ? sprites_[IDX_IDLE]->h : 16,
                                        *platforms_, dt_ms, climbing);
        grounded   = res.hitFloor;
        onLadder_  = res.onLadder;

        if (res.hitFloor && justJumped_) {
            justJumped_ = false;
            if (sound_) sound_->play(SFX::LAND);
        }

        // Límites de pantalla
        if (pos.x < 0.f)            pos.x = 0.f;
        if (pos.x > SCREEN_W - 12)  pos.x = static_cast<float>(SCREEN_W - 12);
        if (pos.y > SCREEN_H)       kill();   // cayó por el fondo
    }
}

// ============================================================
//  Máquina de estados
// ============================================================
void Player::updateState(int dt_ms)
{
    (void)dt_ms;
    const PlayerState prev = state_;

    if (state_ == PlayerState::CLIMB) {
        selectSprite();
        return;
    }

    if (!grounded && state_ != PlayerState::JUMP) {
        if (vel.y > 20.f) state_ = PlayerState::FALL;
    }

    if (grounded) {
        if (state_ == PlayerState::JUMP || state_ == PlayerState::FALL)
            state_ = PlayerState::IDLE;
    }

    if (hammerTimer_ > 0 && grounded)
        state_ = PlayerState::HAMMER;
    else if (hammerTimer_ <= 0 && state_ == PlayerState::HAMMER)
        state_ = PlayerState::IDLE;

    if (grounded && state_ != PlayerState::HAMMER) {
        if (std::abs(vel.x) > 4.f)
            state_ = PlayerState::WALK;
        else if (vel.x == 0.f && state_ == PlayerState::WALK)
            state_ = PlayerState::IDLE;
    }

    if (state_ != prev) stateTimer_ = 0;

    selectSprite();
}

// ============================================================
//  Selección de sprite según estado
// ============================================================
void Player::selectSprite()
{
    switch (state_) {
        case PlayerState::IDLE:
            currentFrame_ = IDX_IDLE;
            break;
        case PlayerState::WALK:
        case PlayerState::FALL:
            // Alternar walkA/walkB cada 150ms
            currentFrame_ = ((stateTimer_ / 150) % 2 == 0)
                            ? IDX_WALKA : IDX_WALKB;
            break;
        case PlayerState::JUMP:
            currentFrame_ = IDX_JUMP;
            break;
        case PlayerState::CLIMB:
            currentFrame_ = ((stateTimer_ / 200) % 2 == 0)
                            ? IDX_CLIMB : IDX_WALKA;
            break;
        case PlayerState::HAMMER:
            currentFrame_ = IDX_HAMMER;
            break;
        case PlayerState::DEAD:
            currentFrame_ = IDX_DEAD;
            break;
        case PlayerState::WIN:
            currentFrame_ = IDX_IDLE;
            break;
    }
}

// ============================================================
//  Draw
// ============================================================
void Player::draw(Framebuffer& fb) const
{
    if (!active) return;

    // Parpadeo si es invencible
    if (invincibleTimer_ > 0 && (invincibleTimer_ / 80) % 2 == 0)
        return;

    const SpriteData* spr = sprites_[currentFrame_];
    if (!spr || !spr->valid()) {
        // Placeholder: rectángulo azul
        fb.fillRect(static_cast<int>(pos.x), static_cast<int>(pos.y),
                    12, 16, Colors::BLUE);
        return;
    }

    const int dx = static_cast<int>(pos.x);
    const int dy = static_cast<int>(pos.y);

    if (!facingRight_)
        fb.blitSpriteFlipH(*spr, dx, dy);
    else
        fb.blitSprite(*spr, dx, dy);

    // Efecto visual del martillo
    if (hammerTimer_ > 0) drawHammerEffect(fb);
}

void Player::drawHammerEffect(Framebuffer& fb) const
{
    // Pequeño arco visual encima del jugador al golpear
    const int cx = static_cast<int>(pos.x) + 6;
    const int cy = static_cast<int>(pos.y) - 4;
    const Color16 col = ((hammerTimer_ / 60) % 2) ? Colors::YELLOW : Colors::ORANGE;
    fb.fillRect(cx - 4, cy, 8, 3, col);
    fb.fillRect(cx - 2, cy - 3, 4, 3, col);
}

// ============================================================
//  Eventos
// ============================================================
void Player::kill()
{
    if (state_ == PlayerState::DEAD || invincibleTimer_ > 0) return;
    state_       = PlayerState::DEAD;
    deathTimer_  = 1500;   // 1.5s de animación
    vel          = {0.f, 0.f};
    currentFrame_= IDX_DEAD;
    if (sound_) sound_->play(SFX::DIE);
}

void Player::respawn(Vec2f spawnP)
{
    spawnPos_        = spawnP;
    pos              = spawnP;
    vel              = {0.f, 0.f};
    state_           = PlayerState::IDLE;
    alive            = true;
    active           = true;
    grounded         = false;
    onLadder_        = false;
    hammerTimer_     = 0;
    deathTimer_      = 0;
    invincibleTimer_ = 2000;  // 2s de invencibilidad al respawn
    currentFrame_    = IDX_IDLE;
    stateTimer_      = 0;
}

void Player::grabHammer()
{
    hammerTimer_ = 8000;   // 8 segundos con el martillo
    state_       = PlayerState::HAMMER;
    if (sound_) sound_->play(SFX::COIN);
}

void Player::onCollision(Entity& other)
{
    if (other.type_ == Type::BARREL || other.type_ == Type::FLAME ||
        other.type_ == Type::ENEMY) {
        if (hammerTimer_ > 0) {
            // Destruir con martillo
            other.alive = false;
            score_ += 300;
            if (sound_) sound_->play(SFX::HAMMER_HIT);
        } else {
            kill();
        }
    }

    if (other.type_ == Type::BONUS) {
        // Recolectar bonus
        other.alive = false;
        score_ += 100;
        if (sound_) sound_->play(SFX::COIN);
    }

    if (other.type_ == Type::HAMMER) {
        other.alive = false;
        grabHammer();
    }
}
