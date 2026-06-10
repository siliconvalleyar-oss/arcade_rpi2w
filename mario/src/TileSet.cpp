#include "../include/TileSet.h"
#include <cstdio>

bool TileSet::load(const std::string& basePath) {
    char filename[64];
    for (int i = 0; i < MAX_TILES; ++i) {
        snprintf(filename, sizeof(filename), "%s/sprite_%02d.png", basePath.c_str(), i);
        if (!tiles[i].load(filename)) {
            // Si falta algún sprite no es crítico, continuamos
        }
    }
    loaded = true;
    return true;
}

const Sprite& TileSet::getTile(TileType type) const {
    // Mapear tipos de TileType a índices de sprite (personalizable)
    switch (type) {
        case TileType::GROUND:      return tiles[0];
        case TileType::BRICK:       return tiles[1];
        case TileType::QUESTION:    return tiles[2];
        case TileType::USED_BLOCK:  return tiles[3];
        case TileType::HIDDEN_BLOCK:return tiles[4];
        case TileType::PIPE_TOP_LEFT:    return tiles[10];
        case TileType::PIPE_TOP_RIGHT:   return tiles[11];
        case TileType::PIPE_BOTTOM_LEFT: return tiles[12];
        case TileType::PIPE_BOTTOM_RIGHT:return tiles[13];
        default: return tiles[0];
    }
}

const Sprite& TileSet::getTileByIndex(int index) const {
    if (index >= 0 && index < MAX_TILES) return tiles[index];
    return tiles[0];
}
