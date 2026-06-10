#pragma once
#include "Types.h"
#include "Graphics.h"
#include "Renderer.h"
#include "Sound.h"
#include "Physics.h"
#include "Player.h"
#include "Enemy.h"
#include "Level.h"
#include <array>
#include <memory>
#include <chrono>

// ============================================================
//  Game - Game Engine principal
//
//  Loop principal:
//    1. readInput()      -> lee GPIO joystick / genera IA demo
//    2. update(dt)       -> actualiza todas las entidades
//    3. checkCollisions()-> AABB entre entidades
//    4. draw()           -> renderiza al framebuffer
//    5. renderer.present()-> volcado parcial al ST7789
//
//  Demo AI:
//    Si JOYSTICK_ENABLED no está definido, el jugador es
//    controlado por una IA simple que intenta llegar a Pauline
//    evitando barriles. Muestra el juego en pantalla completa.
// ============================================================

// Configuración de dificultad
struct DifficultyConfig {
    float  gravityScale{1.0f};
    float  barrelSpeed{70.f};
    int    dkThrowIntervalMs{2000};
    int    maxBarrels{3};
    int    maxFlames{0};
    float  playerSpeed{60.f};
    int    bonusTimeMs{4000};   // tiempo del bonus countdown
    float  scoreMultiplier{1.0f};

    static DifficultyConfig make(Difficulty d);
};

// Puntuaciones para tabla de scoring
struct ScoreEntry {
    char     name[4]{'A','A','A','\0'};
    int      score{0};
    int      level{1};
};

class Game {
public:
    Game() = default;
    ~Game() = default;

    // Inicializar hardware, cargar assets, crear nivel 1
    bool init();
    void shutdown();

    // Loop principal - retorna false cuando debe salir
    bool run();

private:
    // ----- Sub-sistemas -----
    Renderer    renderer_;
    Sound       sound_;
    Physics     physics_;
    Framebuffer fb_;

    SpriteAtlas    atlas_;
    LevelManager   levels_;

    // ----- Entidades -----
    Player      player_;
    DonkeyKong  dk_;

    EntityPool<Barrel, MAX_BARRELS> barrels_;
    EntityPool<Flame,  MAX_FLAMES>  flames_;

    // ----- Estado del juego -----
    GameState    state_{GameState::TITLE};
    Difficulty   difficulty_{Difficulty::NORMAL};
    DifficultyConfig diffCfg_;

    int  currentLevel_{1};
    int  stageTimer_{0};      // tiempo restante en nivel (ms)
    int  gameTimer_{0};       // timer global
    int  titleTimer_{0};
    int  levelClearTimer_{0};
    int  gameOverTimer_{0};

    // ----- High scores -----
    std::array<ScoreEntry, 5> highScores_{};

    // ----- Timers -----
    using Clock = std::chrono::steady_clock;
    Clock::time_point lastFrame_;

    // ----- Métodos de juego -----
    void readInput(InputState& inp);
    void updatePlaying(int dt_ms);
    void updateTitle(int dt_ms);
    void updateGameOver(int dt_ms);
    void updateLevelClear(int dt_ms);

    void checkCollisions();
    bool playerHitsBarrel(const Barrel& b) const;
    bool playerHitsFlame(const Flame& f) const;
    bool playerHammerHitsBarrel(Barrel& b);

    void draw();
    void drawTitle();
    void drawHUD();
    void drawLevelBackground();
    void drawPlatforms();
    void drawLadders();
    void drawBonusItems();
    void drawGameOver();
    void drawLevelClear();
    void drawLives();
    void drawScore();
    void drawDifficultySelect();

    void spawnBarrel();
    void spawnFlame();
    void startLevel(int id);
    void nextLevel();
    void playerDied();
    void gameOver();
    void levelCleared();
    void updateScores(int score);

    // ----- Demo AI -----
    void runDemoAI(InputState& inp);
    int  demoAITimer_{0};
    int  demoAIState_{0};

    // ----- Helpers de dibujo -----
    void drawNumber(int x, int y, int n, Color16 c, int digits=6);
    void drawFilledPlatform(const PlatformTile& p);
    void drawPaulinePlaceholder();

    bool initialized_{false};
};
