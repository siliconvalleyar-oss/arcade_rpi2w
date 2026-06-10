#pragma once
#include "Entity.h"
#include "Physics.h"
#include "Sound.h"

// ============================================================
//  Barrel - El barril rodante clásico de Donkey Kong
// ============================================================
class Barrel final : public Entity {
public:
    Barrel();
    void init(Sound* snd, const Physics* phys,
              const SpriteData* roll1,
              const SpriteData* roll2,
              const SpriteData* explode);

    void launch(Vec2f startPos, float speedX);
    void update(int dt_ms) override;
    void draw(Framebuffer& fb) const override;
    void onCollision(Entity& other) override;

    void setPlatforms(const std::vector<PlatformTile>* p) { platforms_ = p; }
    bool exploding() const { return exploding_; }
    void explode();

private:
    Sound*       sound_{nullptr};
    const Physics* phys_{nullptr};
    const std::vector<PlatformTile>* platforms_{nullptr};

    bool  exploding_{false};
    int   explodeTimer_{0};
    int   rollAngle_{0};   // para animación de rodado
    float dirX_{1.f};      // 1=derecha, -1=izquierda
    int   fallCount_{0};   // cuántas veces ha caído de una plataforma
};

// ============================================================
//  Flame - Enemigo de fuego (niveles avanzados)
// ============================================================
class Flame final : public Entity {
public:
    Flame();
    void init(Sound* snd, const Physics* phys,
              const SpriteData* f1, const SpriteData* f2);

    void spawn(Vec2f pos);
    void update(int dt_ms) override;
    void draw(Framebuffer& fb) const override;

    void setPlatforms(const std::vector<PlatformTile>* p) { platforms_ = p; }

private:
    Sound*      sound_{nullptr};
    const Physics* phys_{nullptr};
    const std::vector<PlatformTile>* platforms_{nullptr};
    float dirX_{1.f};
    int   thinkTimer_{0};   // IA: cuándo cambiar dirección
};

// ============================================================
//  DonkeyKong - El jefe en la cima
// ============================================================
enum class DKState : uint8_t {
    IDLE, THROW, ROAR, STUN, DEAD
};

class DonkeyKong final : public Entity {
public:
    DonkeyKong();

    void init(Sound* snd,
              const SpriteData* idle,
              const SpriteData* throw_,
              const SpriteData* roar,
              const SpriteData* stun);

    void setPos(Vec2f p) { pos = p; }
    void update(int dt_ms) override;
    void draw(Framebuffer& fb) const override;

    // Retorna true cuando debe lanzar un barril
    bool wantsThrow() const { return wantsThrow_; }
    void clearThrow()       { wantsThrow_ = false; }
    Vec2f throwPos()  const { return throwPos_; }

    void hit(int damage);
    int  hp() const { return hp_; }
    bool defeated() const { return state_ == DKState::DEAD; }

    // Dificultad afecta la frecuencia de lanzamiento
    void setThrowInterval(int ms) { throwInterval_ = ms; }

private:
    Sound*  sound_{nullptr};
    DKState state_{DKState::IDLE};
    int     stateTimer_{0};
    int     throwTimer_{0};
    int     throwInterval_{2000};   // ms entre barriles
    int     hp_{3};
    bool    wantsThrow_{false};
    Vec2f   throwPos_{};
};
