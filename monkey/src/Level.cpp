#include "../include/Level.h"
#include "../include/picopng.h"
#include <fstream>
#include <vector>
#include <cstdio>

// ============================================================
//  SpriteAtlas - carga todos los PNGs
// ============================================================
bool SpriteAtlas::loadPNG(SpriteData& out, const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        fprintf(stderr, "[Assets] No se encontró: %s\n", path.c_str());
        // Generar sprite placeholder de 16x16 rojo
        std::vector<uint8_t> dummy(16 * 16 * 4, 0);
        for (int i = 0; i < 16 * 16; ++i) {
            dummy[i*4+0] = 255;  // R
            dummy[i*4+1] = 0;    // G
            dummy[i*4+2] = 0;    // B
            dummy[i*4+3] = 255;  // A
        }
        out.fromRGBA(dummy.data(), 16, 16);
        return false;
    }

    std::vector<unsigned char> buf(
        std::istreambuf_iterator<char>(file), {});

    std::vector<unsigned char> image;
    unsigned long w = 0, h = 0;
    if (decodePNG(image, w, h, buf.data(), buf.size()) != 0) {
        fprintf(stderr, "[Assets] Error decodificando PNG: %s\n", path.c_str());
        return false;
    }

    out.fromRGBA(image.data(), static_cast<int>(w), static_cast<int>(h));
    printf("[Assets] Cargado: %s (%lux%lu)\n", path.c_str(), w, h);
    return true;
}

bool SpriteAtlas::loadAll(const std::string& dir)
{
    // Si los PNGs no existen, loadPNG genera placeholders coloridos
    // para que el juego corra igual en modo demo
    bool ok = true;
    auto ld = [&](SpriteData& s, const char* name) {
        if (!loadPNG(s, dir + "/" + name)) ok = false;
    };

    ld(playerIdle,    "player_idle.png");
    ld(playerWalkA,   "player_walk_a.png");
    ld(playerWalkB,   "player_walk_b.png");
    ld(playerJump,    "player_jump.png");
    ld(playerClimb,   "player_climb.png");
    ld(playerHammer,  "player_hammer.png");
    ld(playerDead,    "player_dead.png");

    ld(dkIdle,        "dk_idle.png");
    ld(dkThrow,       "dk_throw.png");
    ld(dkRoar,        "dk_roar.png");
    ld(dkStun,        "dk_stun.png");

    ld(barrelRoll1,   "barrel_roll1.png");
    ld(barrelRoll2,   "barrel_roll2.png");
    ld(barrelExplode, "barrel_explode.png");

    ld(flame1,        "flame1.png");
    ld(flame2,        "flame2.png");

    ld(tileFloor,     "tile_floor.png");
    ld(tileLadder,    "tile_ladder.png");
    ld(tileLadderTop, "tile_ladder_top.png");

    ld(bonusHat,      "bonus_hat.png");
    ld(bonusUmbrella, "bonus_umbrella.png");
    ld(bonusHandbag,  "bonus_handbag.png");
    ld(bonusHammer,   "bonus_hammer.png");

    ld(paulineIdle,   "pauline.png");

    // ok=false no es fatal, placeholders funcionan
    return true;
}

// ============================================================
//  LevelDef - Geometría fija por nivel
//
//  Coordenadas para pantalla 240x240:
//    Y=0 arriba, Y=239 abajo
//    Cada plataforma es un rectángulo sólido
// ============================================================

LevelDef LevelDef::makeLevel1()
{
    LevelDef lv;
    lv.id         = 1;
    lv.bgColorVal = Colors::SKY;
    lv.timeLimitSec = 200;

    // ---- Suelo ----
    lv.platforms.push_back({ Rect{0, 230, 240, 10}, false, false });

    // ---- Plataformas inclinadas estilo DK arcade ----
    // P1 - inferior (el jugador empieza aquí)
    lv.platforms.push_back({ Rect{0,  195, 200, 6}, false, false });
    // P2
    lv.platforms.push_back({ Rect{40, 160, 200, 6}, false, false });
    // P3
    lv.platforms.push_back({ Rect{0,  125, 200, 6}, false, false });
    // P4
    lv.platforms.push_back({ Rect{40,  90, 200, 6}, false, false });
    // P5 - superior (DK está aquí)
    lv.platforms.push_back({ Rect{0,   55,  240, 6}, false, false });
    // P6 - cima
    lv.platforms.push_back({ Rect{80,  20,  80,  6}, false, false });

    // ---- Escaleras ----
    lv.platforms.push_back({ Rect{30,  165, 8, 35}, false, true });
    lv.platforms.push_back({ Rect{170, 130, 8, 35}, false, true });
    lv.platforms.push_back({ Rect{30,   95, 8, 35}, false, true });
    lv.platforms.push_back({ Rect{170,  60, 8, 35}, false, true });
    lv.platforms.push_back({ Rect{100,  26, 8, 34}, false, true });

    // ---- Posiciones ----
    lv.playerSpawn  = { 10.f, 178.f };
    lv.dkPos        = {  5.f,  26.f };
    lv.paulinePos   = { 100.f, 10.f };
    lv.barrelOrigin = { 20.f,  44.f };
    lv.hammerPos1   = { 55.f, 175.f };
    lv.hammerPos2   = { 155.f, 105.f };

    // ---- Bonus items ----
    for (int i = 0; i < 3; ++i) {
        BonusItem b;
        b.pos   = { 60.f + i * 40.f, 180.f };
        b.value = 300;
        b.active= true;
        lv.bonusItems.push_back(b);
    }
    for (int i = 0; i < 2; ++i) {
        BonusItem b;
        b.pos   = { 60.f + i * 60.f, 110.f };
        b.value = 500;
        b.active= true;
        lv.bonusItems.push_back(b);
    }

    return lv;
}

LevelDef LevelDef::makeLevel2()
{
    LevelDef lv;
    lv.id = 2;
    lv.bgColorVal = Colors::DARKBLUE;
    lv.timeLimitSec = 180;

    // Suelo
    lv.platforms.push_back({ Rect{0, 230, 240, 10}, false, false });
    // Plataformas más cortas y separadas
    lv.platforms.push_back({ Rect{0,   195, 110, 5}, false, false });
    lv.platforms.push_back({ Rect{130, 195, 110, 5}, false, false });
    lv.platforms.push_back({ Rect{0,   155, 110, 5}, false, false });
    lv.platforms.push_back({ Rect{130, 155, 110, 5}, false, false });
    lv.platforms.push_back({ Rect{0,   115, 110, 5}, false, false });
    lv.platforms.push_back({ Rect{130, 115, 110, 5}, false, false });
    lv.platforms.push_back({ Rect{0,    75,  240, 5}, false, false });
    lv.platforms.push_back({ Rect{80,   35,  80,  5}, false, false });

    // Escaleras
    lv.platforms.push_back({ Rect{100, 160, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{100, 120, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{100,  80, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{110,  40, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{50,  160, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{175, 160, 8, 40}, false, true });

    lv.playerSpawn  = { 10.f, 178.f };
    lv.dkPos        = {  5.f,  22.f };
    lv.paulinePos   = { 105.f, 22.f };
    lv.barrelOrigin = { 18.f,  65.f };
    lv.hammerPos1   = { 50.f, 140.f };
    lv.hammerPos2   = { 170.f, 140.f };

    for (int i = 0; i < 4; ++i) {
        BonusItem b;
        b.pos   = { 40.f + i * 45.f, 140.f };
        b.value = 400;
        b.active= true;
        lv.bonusItems.push_back(b);
    }

    return lv;
}

LevelDef LevelDef::makeLevel3()
{
    LevelDef lv;
    lv.id = 3;
    lv.bgColorVal = Colors::DARKBROWN;
    lv.timeLimitSec = 160;

    lv.platforms.push_back({ Rect{0,   230, 240, 10}, false, false });
    lv.platforms.push_back({ Rect{0,   195,  80,  5}, false, false });
    lv.platforms.push_back({ Rect{160, 195,  80,  5}, false, false });
    lv.platforms.push_back({ Rect{0,   155, 240,  5}, false, false });
    lv.platforms.push_back({ Rect{0,   115,  80,  5}, false, false });
    lv.platforms.push_back({ Rect{160, 115,  80,  5}, false, false });
    lv.platforms.push_back({ Rect{0,    75, 240,  5}, false, false });
    lv.platforms.push_back({ Rect{80,   35,  80,  5}, false, false });

    // Escaleras en el centro
    lv.platforms.push_back({ Rect{115, 160, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{115,  80, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{115,  40, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{40,  160, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{190, 160, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{40,   80, 8, 40}, false, true });
    lv.platforms.push_back({ Rect{190,  80, 8, 40}, false, true });

    lv.playerSpawn  = { 10.f, 178.f };
    lv.dkPos        = {  5.f,  22.f };
    lv.paulinePos   = { 115.f, 22.f };
    lv.barrelOrigin = { 18.f,  65.f };
    lv.hammerPos1   = { 30.f,  100.f };
    lv.hammerPos2   = { 190.f, 60.f };

    return lv;
}

// ============================================================
//  LevelManager
// ============================================================
void LevelManager::loadLevel(int id, const SpriteAtlas& /*atlas*/)
{
    currentId_ = id;
    switch(id) {
        case 1:  current_ = LevelDef::makeLevel1(); break;
        case 2:  current_ = LevelDef::makeLevel2(); break;
        case 3:  current_ = LevelDef::makeLevel3(); break;
        default: current_ = LevelDef::makeLevel1(); break;
    }
    printf("[Level] Nivel %d cargado (%zu plataformas, %zu items)\n",
           id,
           current_.platforms.size(),
           current_.bonusItems.size());
}

int LevelManager::nextLevelId() const
{
    // Cicla entre 1..3
    return (currentId_ % 3) + 1;
}

void LevelManager::markBonusCollected(int idx)
{
    if (idx >= 0 && idx < static_cast<int>(current_.bonusItems.size()))
        current_.bonusItems[idx].collected = true;
}

int LevelManager::remainingBonuses() const
{
    int n = 0;
    for (const auto& b : current_.bonusItems)
        if (b.active && !b.collected) ++n;
    return n;
}
