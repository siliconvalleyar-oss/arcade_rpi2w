#ifndef TILESET_H
#define TILESET_H

#include "Sprite.h"
#include "Types.h"
#include <vector>
#include <string>

class TileSet {
public:
    static const int MAX_TILES = 30;
    
    bool load(const std::string& basePath);
    const Sprite& getTile(TileType type) const;
    const Sprite& getTileByIndex(int index) const;
    
private:
    Sprite tiles[MAX_TILES];
    bool loaded;
};

#endif


