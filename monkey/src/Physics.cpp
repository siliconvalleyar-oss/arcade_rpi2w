#include "../include/Physics.h"
#include <algorithm>
#include <cmath>

// ============================================================
//  Physics - Integración y resolución de colisiones
// ============================================================

// ---- Resolver AABB contra tiles ----
// Separamos el movimiento en X e Y para evitar falsos positivos
CollisionResult Physics::resolve(Rect& r, Vec2f& vel,
                                  const std::vector<PlatformTile>& tiles,
                                  bool falling) const
{
    CollisionResult res{};

    for (const auto& tile : tiles) {
        if (tile.isLadder) continue;   // escaleras no son sólidas en X/Y
        if (!r.overlaps(tile.rect)) continue;

        // ---- Calcular penetración en cada eje ----
        const int overlapLeft  = r.right()  - tile.rect.x;
        const int overlapRight = tile.rect.right() - r.x;
        const int overlapTop   = r.bottom() - tile.rect.y;
        const int overlapBot   = tile.rect.bottom() - r.y;

        // Mínima penetración de eje
        const int minX = std::min(overlapLeft, overlapRight);
        const int minY = std::min(overlapTop,  overlapBot);

        if (tile.passThrough) {
            // Solo colisionar si caemos Y el pie estaba encima del tile
            if (falling && overlapTop < overlapBot && overlapTop <= 8) {
                r.y -= overlapTop;
                vel.y = 0.f;
                res.hitFloor = true;
            }
            continue;
        }

        // Resolver por eje de menor penetración
        if (minX < minY) {
            if (overlapLeft < overlapRight) {
                r.x -= overlapLeft;
                vel.x = 0.f;
                res.hitRight = true;
            } else {
                r.x += overlapRight;
                vel.x = 0.f;
                res.hitLeft = true;
            }
        } else {
            if (overlapTop < overlapBot) {
                r.y -= overlapTop;
                vel.y = std::min(vel.y, 0.f);
                res.hitFloor = true;
            } else {
                r.y += overlapBot;
                vel.y = std::max(vel.y, 0.f);
                res.hitCeil = true;
            }
        }
    }
    return res;
}

// ---- Integrar posición + colisión para entidad genérica ----
CollisionResult Physics::integrate(Vec2f& pos, Vec2f& vel,
                                    int sprW, int sprH,
                                    const std::vector<PlatformTile>& tiles,
                                    int dt_ms,
                                    bool onLadder) const
{
    const float dt = dt_ms * 0.001f;

    // Aplicar gravedad (suspendida en escalera)
    if (!onLadder) {
        vel.y += PhysConst::GRAVITY * gravScale_ * dt;
        vel.y = std::min(vel.y, PhysConst::MAX_FALL_SPEED);
    } else {
        vel.y = std::clamp(vel.y,
                           -PhysConst::LADDER_SPEED,
                            PhysConst::LADDER_SPEED);
    }

    // Mover en X
    pos.x += vel.x * dt;
    Rect rx{ static_cast<int>(pos.x), static_cast<int>(pos.y), sprW, sprH };
    bool falling = vel.y > 0.f;
    CollisionResult res = resolve(rx, vel, tiles, falling);
    pos.x = static_cast<float>(rx.x);

    // Mover en Y
    pos.y += vel.y * dt;
    Rect ry{ static_cast<int>(pos.x), static_cast<int>(pos.y), sprW, sprH };
    CollisionResult resY = resolve(ry, vel, tiles, falling);
    pos.y = static_cast<float>(ry.y);

    // Combinar resultados
    res.hitFloor |= resY.hitFloor;
    res.hitCeil  |= resY.hitCeil;
    res.hitLeft  |= resY.hitLeft;
    res.hitRight |= resY.hitRight;

    // Verificar escaleras
    Rect finalRect{ static_cast<int>(pos.x), static_cast<int>(pos.y), sprW, sprH };
    res.onLadder = isOverLadder(finalRect, tiles);

    return res;
}

// ---- Detectar escalera bajo los pies ----
bool Physics::isOverLadder(const Rect& bounds,
                             const std::vector<PlatformTile>& tiles) const
{
    // Centro horizontal del personaje
    const int cx = bounds.x + bounds.w / 2;
    const int cy = bounds.y + bounds.h / 2;

    for (const auto& tile : tiles) {
        if (!tile.isLadder) continue;
        if (cx >= tile.rect.x && cx < tile.rect.right() &&
            cy >= tile.rect.y && cy < tile.rect.bottom())
            return true;
    }
    return false;
}

// ---- Verificar si está sobre suelo ----
bool Physics::isGrounded(const Rect& bounds,
                          const std::vector<PlatformTile>& tiles) const
{
    // Pequeño rect un píxel por debajo de los pies
    Rect foot{ bounds.x + 2, bounds.bottom(), bounds.w - 4, 2 };
    for (const auto& tile : tiles) {
        if (tile.isLadder) continue;
        if (foot.overlaps(tile.rect)) return true;
    }
    return false;
}

// ---- Física de barril: rebota en plataformas horizontales ----
CollisionResult Physics::integrateBarrel(Vec2f& pos, Vec2f& vel,
                                          const std::vector<PlatformTile>& tiles,
                                          int dt_ms) const
{
    const float dt = dt_ms * 0.001f;

    // Gravedad
    vel.y += PhysConst::GRAVITY * dt;
    vel.y  = std::min(vel.y, PhysConst::MAX_FALL_SPEED);

    // Mover
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;

    Rect r{ static_cast<int>(pos.x), static_cast<int>(pos.y), 12, 12 };
    CollisionResult res = resolve(r, vel, tiles, vel.y > 0.f);
    pos.x = static_cast<float>(r.x);
    pos.y = static_cast<float>(r.y);

    if (res.hitFloor) {
        // Rebote suave al tocar suelo
        vel.y = PhysConst::BARREL_BOUNCE_Y * 0.5f;
    }

    // Rebotar en paredes laterales
    if (res.hitLeft || res.hitRight) {
        vel.x = -vel.x;
    }

    // Límites de pantalla X
    if (pos.x < 0.f) {
        pos.x = 0.f;
        vel.x = std::abs(vel.x);
    } else if (pos.x > SCREEN_W - 14) {
        pos.x = static_cast<float>(SCREEN_W - 14);
        vel.x = -std::abs(vel.x);
    }

    return res;
}
