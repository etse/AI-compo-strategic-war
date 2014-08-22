#pragma once

#include "MapTile.h"
#include "../include/gui_class.h"

public ref class Map {
protected:
	_GUIlog ^log;
	array<MapTile^, 2> ^map;
	Point _mapSize;

	void createMap();
	void setMapPointers();

public:
	Map();

	void reset();
	void setSize(int widht, int height);
	MapTile^ getTile(int x, int y);
	MapTile^ getTile(Point position);

	int width();
	int height();
	Point mapSize();
};

