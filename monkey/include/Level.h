#pragma once
#include "Types.h"
#include "Physics.h"
#include "Graphics.h"
#include <vector>
#include <array>
#include <string>
#include <memory>

// ============================================================
//  Level - Define la geometría, assets y dificultad de cada nivel
//
//  Niveles disponibles:
//    1 - Rivet Stage (escaleras + plataformas inclinadas)
//    2 - Pie Stage   (barriles desde ambos lados)
//    3 - Elevator    (ascensores móviles)
//    Bonus - tiempo limitado
// ============================================================

static constexpr int MAX_PLATFORMS  = 32;
static constexpr int MAX_BARRELS    = 8;
static constexpr int MAX_FLAMES     = 4;
static constexpr int MAX_BONUSES    = 16;

// Sprite atlas - todos los sprites del juego en un índice central
struct SpriteAtlas {
    // Jugador
    SpriteData playerIdle;
    SpriteData playerWalkA;
    SpriteData playerWalkB;
    SpriteData playerJump;
    SpriteData playerClimb;
    SpriteData playerHammer;
    SpriteData playerDead;

    // Donkey Kong
    SpriteData dkIdle;
    SpriteData dkThrow;
    SpriteData dkRoar;
    SpriteData dkStun;

    // Barrel
    SpriteData barrelRoll1;
    SpriteData barrelRoll2;
    SpriteData barrelExplode;

    // Flame
    SpriteData flame1;
    SpriteData flame2;

    // Tiles
    SpriteData tileFloor;
    SpriteData tileLadder;
    SpriteData tileLadderTop;

    // Bonus items
    SpriteData bonusHat;
    SpriteData bonusUmbrella;
    SpriteData bonusHandbag;
    SpriteData bonusHammer;

    // Pauline
    SpriteData paulineIdle;

    // Cargar todos los PNGs desde directorio assets/
    bool loadAll(const std::string& assetDir);

private:
    // Helper: cargar un PNG y convertir a SpriteData
    bool loadPNG(SpriteData& out, const std::string& path);
};

// Punto de bonus (sombrero, bolso, etc.)
struct BonusItem {
    Vec2f pos{};
    int   value{100};
    bool  collected{false};
    bool  active{false};
    const SpriteData* sprite{nullptr};
};

// Definición de un nivel
struct LevelDef {
    int  id{1};
    std::string bgColor;  // "SKY", "BLACK", etc.
    Color16     bgColorVal{Colors::SKY};

    // Plataformas y escaleras
    std::vector<PlatformTile> platforms;

    // Posiciones de spawn
    Vec2f playerSpawn{10.f, 200.f};
    Vec2f dkPos{100.f, 20.f};
    Vec2f paulinePos{110.f, 14.f};
    Vec2f barrelOrigin{};      // donde DK lanza los barriles
    Vec2f hammerPos1{};
    Vec2f hammerPos2{};

    // Items de bonus
    std::vector<BonusItem> bonusItems;

    // Tiempo límite (0 = sin límite)
    int timeLimitSec{200};

    // Rivet positions (para nivel de remaches)
    std::vector<Vec2i> rivets;

    // Constructor helpers
    static LevelDef makeLevel1();
    static LevelDef makeLevel2();
    static LevelDef makeLevel3();
};

// ============================================================
//  LevelManager - gestiona el nivel actual y su progreso
// ============================================================
class LevelManager {
public:
    void         loadLevel(int levelId, const SpriteAtlas& atlas);
    LevelDef&    current()      { return current_; }
    int          currentId()    const { return currentId_; }
    int          nextLevelId()  const;
    void         markBonusCollected(int idx);
    int          remainingBonuses() const;

    // Tiles para el motor de física
    const std::vector<PlatformTile>& platforms() const {
        return current_.platforms;
    }

private:
    LevelDef current_;
    int      currentId_{1};
};
