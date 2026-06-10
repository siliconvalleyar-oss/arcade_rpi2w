#pragma once
#include "Types.h"
#include <vector>
#include <functional>

// ============================================================
//  Physics - Motor de física 2D para plataformas
//
//  Características:
//   - Gravedad configurable por dificultad
//   - Detección de colisión AABB contra tiles de plataforma
//   - Respuesta de colisión con empuje mínimo
//   - Escaleras: suspenden gravedad, permiten movimiento vertical
//   - Barriles: física parabólica con rebote
// ============================================================

// Constantes de física
namespace PhysConst {
    inline constexpr float GRAVITY         = 480.f;   // px/s²
    inline constexpr float MAX_FALL_SPEED  = 320.f;   // px/s
    inline constexpr float JUMP_SPEED      = -240.f;  // px/s (negativo = arriba)
    inline constexpr float WALK_SPEED      = 60.f;    // px/s
    inline constexpr float RUN_SPEED       = 90.f;    // px/s
    inline constexpr float BARREL_SPEED_X  = 70.f;    // px/s
    inline constexpr float BARREL_BOUNCE_Y = -120.f;  // px/s rebote
    inline constexpr float LADDER_SPEED    = 55.f;    // px/s
    inline constexpr float HAMMER_SPEED    = 50.f;    // px/s extra
}

// Tile de plataforma (estático, no mueve)
struct PlatformTile {
    Rect  rect;
    bool  passThrough{false};  // Solo colisión desde arriba
    bool  isLadder{false};
};

// Resultado de resolución de colisión
struct CollisionResult {
    bool hitFloor{false};
    bool hitCeil{false};
    bool hitLeft{false};
    bool hitRight{false};
    bool onLadder{false};
};

class Physics {
public:
    explicit Physics(float gravityScale = 1.0f)
        : gravScale_(gravityScale) {}

    void setGravityScale(float s) { gravScale_ = s; }

    // Integrar velocidad + posición para una entidad
    // Retorna resultado de colisión con el mapa
    CollisionResult integrate(
        Vec2f& pos, Vec2f& vel,
        int   sprW, int sprH,
        const std::vector<PlatformTile>& tiles,
        int dt_ms,
        bool onLadder = false) const;

    // Verificar si una posición está sobre una escalera
    bool isOverLadder(const Rect& bounds,
                      const std::vector<PlatformTile>& tiles) const;

    // Verificar si un rect está sobre suelo
    bool isGrounded(const Rect& bounds,
                    const std::vector<PlatformTile>& tiles) const;

    // Física de barril: parábola + rebote en suelo
    CollisionResult integrateBarrel(
        Vec2f& pos, Vec2f& vel,
        const std::vector<PlatformTile>& tiles,
        int dt_ms) const;

private:
    float gravScale_{1.0f};

    // Resolver colisión de un Rect contra todos los tiles
    // Devuelve el desplazamiento de corrección
    CollisionResult resolve(Rect& r, Vec2f& vel,
                            const std::vector<PlatformTile>& tiles,
                            bool falling) const;
};
