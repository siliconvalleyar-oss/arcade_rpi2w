#pragma once
#include "Types.h"
#include "Graphics.h"
#include <vector>
#include <memory>
#include <string_view>

// ============================================================
//  Entity - Base de todos los objetos del juego
//
//  Usa C++17:
//   - std::string_view para nombres
//   - if constexpr en subclases
//   - structured bindings al iterar
//  C++20:
//   - std::span para buffers
//   - designated initializers
// ============================================================

class Entity {
public:
    // ----- Tipos internos ----
    enum class Type : uint8_t {
        PLAYER, DONKEYKONG, BARREL, ENEMY, PLATFORM,
        LADDER, BONUS, FLAME, PAULINE, HAMMER
    };

    // ----- Constructor / Destructor ----
    explicit Entity(Type t) : type_(t) {}
    virtual ~Entity() = default;

    // No copiable, movible
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;

    // ----- Interface virtual ----
    virtual void update(int dt_ms)  = 0;
    virtual void draw(Framebuffer& fb) const = 0;
    virtual void onCollision(Entity& /*other*/) {}

    // ----- Posición y física ----
    Vec2f    pos{};         // posición world (float para física suave)
    Vec2f    vel{};         // velocidad en px/s
    Rect     bounds{};      // AABB en screen coords (actualizado cada frame)
    bool     grounded{false};
    bool     alive{true};
    bool     active{true};
    Type     type_;

    // ----- Animación ----
    struct Animation {
        std::vector<int> frames;   // índices en spriteSheet
        int fps{8};
        bool loop{true};
        std::string_view name;
    };

    // ----- Sprite actual ----
    // El sprite se renderiza desde sprites_[currentFrame_]
    const SpriteData* currentSprite() const {
        if (sprites_.empty()) return nullptr;
        return sprites_[currentFrame_];
    }

    void addSprite(const SpriteData* s) { sprites_.push_back(s); }

    void setAnimation(int firstFrame, int lastFrame, int fps, bool loop = true) {
        animStart_   = firstFrame;
        animEnd_     = lastFrame;
        animFps_     = fps;
        animLoop_    = loop;
        animTimer_   = 0;
        currentFrame_= firstFrame;
        animDone_    = false;
    }

    void tickAnimation(int dt_ms) {
        if (animStart_ == animEnd_) return;
        animTimer_ += dt_ms;
        int frameMs = (animFps_ > 0) ? (1000 / animFps_) : 100;
        while (animTimer_ >= frameMs) {
            animTimer_ -= frameMs;
            if (currentFrame_ < animEnd_) {
                ++currentFrame_;
            } else {
                if (animLoop_) currentFrame_ = animStart_;
                else            animDone_ = true;
            }
        }
    }

    bool  animDone() const { return animDone_; }
    int   frame()    const { return currentFrame_; }

    // ----- Helpers geométricos ----
    Rect screenRect() const { return bounds; }

    // Actualizar bounds desde pos + tamaño del sprite actual
    void updateBounds() {
        if (const SpriteData* s = currentSprite()) {
            bounds.x = static_cast<int>(pos.x);
            bounds.y = static_cast<int>(pos.y);
            bounds.w = s->w;
            bounds.h = s->h;
        }
    }

    // Centro de masa para físicas
    Vec2f center() const {
        return { pos.x + bounds.w * 0.5f,
                 pos.y + bounds.h * 0.5f };
    }

protected:
    std::vector<const SpriteData*> sprites_;
    int  currentFrame_{0};
    int  animStart_{0}, animEnd_{0}, animFps_{8};
    bool animLoop_{true}, animDone_{false};
    int  animTimer_{0};
};

// ============================================================
//  EntityPool - Pool de entidades para evitar allocaciones
//  en tiempo de juego (C++17 template)
// ============================================================
template<typename T, int MaxSize>
class EntityPool {
    static_assert(std::is_base_of_v<Entity, T>,
                  "T must derive from Entity");
public:
    // Activar una entidad del pool
    T* acquire() {
        for (auto& e : pool_) {
            if (!e.active) {
                e.active = true;
                e.alive  = true;
                return &e;
            }
        }
        return nullptr;  // pool lleno
    }

    void releaseAll() {
        for (auto& e : pool_) e.active = false;
    }

    // Iterar activos con lambda (C++17 if constexpr compatible)
    template<typename Fn>
    void forEach(Fn&& fn) {
        for (auto& e : pool_) {
            if (e.active) fn(e);
        }
    }

    template<typename Fn>
    void forEachConst(Fn&& fn) const {
        for (const auto& e : pool_) {
            if (e.active) fn(e);
        }
    }

    int activeCount() const {
        int n = 0;
        for (const auto& e : pool_) if (e.active) ++n;
        return n;
    }

    std::array<T, MaxSize> pool_;
};
