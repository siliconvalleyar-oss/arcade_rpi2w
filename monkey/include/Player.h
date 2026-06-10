#pragma once
#include "Entity.h"
#include "Physics.h"
#include "Sound.h"
#include <optional>

// ============================================================
//  Player - Jumpman (Mario) controlable por joystick o IA demo
// ============================================================

// Estado del jugador
enum class PlayerState : uint8_t {
    IDLE, WALK, JUMP, FALL, CLIMB, HAMMER, DEAD, WIN
};

// Input del joystick (leer GPIO o simular en demo)
struct InputState {
    bool left{false}, right{false};
    bool up{false},   down{false};
    bool jumpBtn{false};
    bool hammerBtn{false};
};

class Player final : public Entity {
public:
    Player();

    void init(Sound* snd, const Physics* phys);

    // Cargar sprites desde SpriteData (cargados del PNG)
    void loadSprites(
        const SpriteData* idle,
        const SpriteData* walkA,
        const SpriteData* walkB,
        const SpriteData* jump,
        const SpriteData* climb,
        const SpriteData* hammer,
        const SpriteData* dead
    );

    // ----- Interface Entity ----
    void update(int dt_ms) override;
    void draw(Framebuffer& fb) const override;
    void onCollision(Entity& other) override;

    // ----- Input ----
    void setInput(const InputState& in) { input_ = in; }

    // ----- Estado ----
    PlayerState  state()    const { return state_; }
    int          lives()    const { return lives_; }
    int          score()    const { return score_; }
    bool         hasHammer()const { return hammerTimer_ > 0; }
    void         addScore(int v)  { score_ += v; }
    void         kill();
    void         respawn(Vec2f spawnPos);
    void         grabHammer();
    void         setOnLadder(bool v) { onLadder_ = v; }
    bool         onLadder()  const { return onLadder_; }
    void         setPlatforms(const std::vector<PlatformTile>* p) { platforms_ = p; }

private:
    void updateState(int dt_ms);
    void applyInput(int dt_ms);
    void selectSprite();
    void drawHammerEffect(Framebuffer& fb) const;

    PlayerState  state_{PlayerState::IDLE};
    InputState   input_{};
    Sound*       sound_{nullptr};
    const Physics* physics_{nullptr};
    const std::vector<PlatformTile>* platforms_{nullptr};

    int  lives_{3};
    int  score_{0};
    int  stateTimer_{0};     // tiempo en estado actual (ms)
    int  hammerTimer_{0};    // tiempo restante con martillo (ms)
    int  deathTimer_{0};
    int  invincibleTimer_{0};

    bool onLadder_{false};
    bool facingRight_{true};
    bool justJumped_{false};

    Vec2f spawnPos_{};

    // Sprites indexados
    enum SprIdx { IDX_IDLE=0, IDX_WALKA, IDX_WALKB, IDX_JUMP,
                  IDX_CLIMB, IDX_HAMMER, IDX_DEAD, IDX_COUNT };
    static constexpr int SPRITE_SCALE = 1;
};
