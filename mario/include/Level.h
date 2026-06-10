#ifndef LEVEL_H
#define LEVEL_H

#include "Types.h"
#include "../include/TileSet.h"
#include <vector>

class Level {
public:
    	uint8_t tiles[MAP_WIDTH][MAP_HEIGHT];
    	Level();
    	void loadTestLevel();
    	TileType getTile(int x, int y) const;
    	void setTile(int x, int y, TileType type);
    	void removeTile(int x, int y);
    	void draw() const;
 	void setTileSet(const TileSet* ts) { tileSet = ts; }

private:
    const TileSet* tileSet = nullptr;

};

#endif
