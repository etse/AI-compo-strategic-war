#include "Map.h"


Map::Map() {
	log = _GUIlog::getInstance();
	map = nullptr;
	_mapSize = Point(0, 0);
}

void Map::reset() {
	for (int x = 0; x < _mapSize.X; ++x) {
		for (int y = 0; y < _mapSize.Y; ++y) {
			MapTile ^currentTile = map[x, y];
			currentTile->isVisible = false;
			currentTile->isBorderTile = false;
			currentTile->ant = nullptr;
			currentTile->harvesterValue = 0;
			currentTile->isBehindWall = false;
			currentTile->pathFound = false;
			currentTile->dangerLevel = 0;
			currentTile->exploreValue++;
		}
	}
}

void Map::createMap() {
	map = gcnew array<MapTile^, 2>(_mapSize.X, _mapSize.Y);

	for (int x = 0; x < _mapSize.X; ++x) {
		for (int y = 0; y < _mapSize.Y; ++y) {
			map[x,y] = gcnew MapTile(x, y);
		}
	}

	setMapPointers();
}

void Map::setMapPointers() {
	for (int x = 0; x < _mapSize.X; ++x) {
		for (int y = 0; y < _mapSize.Y; ++y) {
			MapTile ^currentTile = map[x, y];
			int north = y - 1;
			if (north < 0) north = _mapSize.Y - 1;
			currentTile->north = map[x, north];
			int south = y + 1;
			if (south >= _mapSize.Y) south = 0;
			currentTile->south = map[x, south];
			int west = x - 1;
			if (west < 0) west = _mapSize.X - 1;
			currentTile->west = map[west, y];
			int east = x + 1;
			if (east >= _mapSize.X) east = 0;
			currentTile->east = map[east, y];
			
		}
	}
}

void Map::setSize(int width, int height) {
	if (width > 0 && height > 0) {
		if (width != _mapSize.X || height != _mapSize.Y) {
			log->write("Map size has changed: " + width + "," + height);
			_mapSize = Point(width, height);
		}
	}
	if (!map) {
		createMap();
	}
}

MapTile^ Map::getTile(Point position) {
	return getTile(position.X, position.Y);
}

MapTile^ Map::getTile(int x, int y) {
	if (map && x >= 0 && y >= 0 && x < _mapSize.X && y < _mapSize.Y) {
		return map[x, y];
	}
	return nullptr;
}

int Map::width() {
	return _mapSize.X;
}

int Map::height() {
	return _mapSize.Y;
}

Point Map::mapSize() {
	return _mapSize;
}