#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>

// ============================================================
//  Pantalla ST7789 - 240x240 píxeles, color RGB565
// ============================================================
static constexpr int SCREEN_W = 240;
static constexpr int SCREEN_H = 240;
static constexpr int SCREEN_PIXELS = SCREEN_W * SCREEN_H;

// ============================================================
//  Color RGB565 nativo del ST7789
//  El bus envía big-endian (byte alto primero)
// ============================================================
using Color16 = uint16_t;

// Convertir RGB888 -> RGB565
inline constexpr Color16 rgb(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return static_cast<Color16>(
        ((r & 0xF8u) << 8u) |
        ((g & 0xFCu) << 3u) |
        (b >> 3u)
    );
}

// Colores de la paleta del juego
namespace Colors {
    inline constexpr Color16 BLACK      = rgb(0,   0,   0  );
    inline constexpr Color16 WHITE      = rgb(255, 255, 255);
    inline constexpr Color16 RED        = rgb(220, 30,  30 );
    inline constexpr Color16 GREEN      = rgb(50,  200, 50 );
    inline constexpr Color16 BLUE       = rgb(30,  80,  220);
    inline constexpr Color16 YELLOW     = rgb(240, 220, 0  );
    inline constexpr Color16 ORANGE     = rgb(240, 130, 0  );
    inline constexpr Color16 CYAN       = rgb(0,   220, 220);
    inline constexpr Color16 MAGENTA    = rgb(220, 0,   220);
    inline constexpr Color16 DARKGRAY   = rgb(50,  50,  50 );
    inline constexpr Color16 GRAY       = rgb(120, 120, 120);
    inline constexpr Color16 LIGHTGRAY  = rgb(200, 200, 200);
    inline constexpr Color16 BROWN      = rgb(160, 80,  20 );
    inline constexpr Color16 DARKBROWN  = rgb(100, 50,  10 );
    inline constexpr Color16 SKIN       = rgb(250, 200, 150);
    inline constexpr Color16 STEEL      = rgb(80,  100, 140);
    inline constexpr Color16 SKY        = rgb(30,  60,  120);
    inline constexpr Color16 DARKBLUE   = rgb(10,  20,  80 );
    inline constexpr Color16 LADDER     = rgb(200, 180, 60 );
    inline constexpr Color16 PLATFORM   = rgb(160, 90,  30 );
    inline constexpr Color16 TRANSPARENT= 0xFFFF;   // Sentinel para alpha 0
}

// ============================================================
//  Vectores 2D enteros y float
// ============================================================
struct Vec2i {
    int x{0}, y{0};
    constexpr Vec2i() = default;
    constexpr Vec2i(int x, int y) : x(x), y(y) {}
    constexpr Vec2i operator+(Vec2i o) const { return {x+o.x, y+o.y}; }
    constexpr Vec2i operator-(Vec2i o) const { return {x-o.x, y-o.y}; }
    constexpr bool  operator==(Vec2i o) const { return x==o.x && y==o.y; }
};

struct Vec2f {
    float x{0.f}, y{0.f};
    constexpr Vec2f() = default;
    constexpr Vec2f(float x, float y) : x(x), y(y) {}
    constexpr Vec2f operator+(Vec2f o) const { return {x+o.x, y+o.y}; }
    constexpr Vec2f operator-(Vec2f o) const { return {x-o.x, y-o.y}; }
    constexpr Vec2f operator*(float s) const { return {x*s, y*s}; }
    constexpr Vec2f operator+=(Vec2f o) { x+=o.x; y+=o.y; return *this; }
    float length() const { return std::sqrt(x*x + y*y); }
};

// ============================================================
//  Rect AABB para colisiones
// ============================================================
struct Rect {
    int x{0}, y{0}, w{0}, h{0};
    constexpr Rect() = default;
    constexpr Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
    constexpr bool overlaps(const Rect& o) const {
        return x < o.x+o.w && x+w > o.x &&
               y < o.y+o.h && y+h > o.y;
    }
    constexpr bool contains(int px, int py) const {
        return px >= x && px < x+w && py >= y && py < y+h;
    }
    constexpr int  right()  const { return x + w; }
    constexpr int  bottom() const { return y + h; }
    constexpr Vec2i center() const { return {x + w/2, y + h/2}; }
};

// ============================================================
//  Estado del juego
// ============================================================
enum class GameState : uint8_t {
    TITLE,
    PLAYING,
    GAME_OVER,
    LEVEL_CLEAR,
    PAUSED
};

// ============================================================
//  Dificultad
// ============================================================
enum class Difficulty : uint8_t {
    EASY   = 0,
    NORMAL = 1,
    HARD   = 2
};

// ============================================================
//  Conversión RGBA8888 (de picoPNG) -> RGB565 con chroma key
//  Si alpha < 128, devuelve Colors::TRANSPARENT
// ============================================================
inline Color16 rgba8888_to_rgb565(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
    if (a < 128) return Colors::TRANSPARENT;
    return rgb(r, g, b);
}
