#include "MapTile.h"


MapTile::MapTile(int posX, int posY) {
	position = Point(posX, posY);
	isWall = false;
	hasFood = false;
	isVisible = false;
	isBorderTile = false;
	visited = false;
	added = false;
	isSpawner = false;
	isSpawnerDestroyed = false;
	isBehindWall = false;
	pathFound = false;
	explorerOnTheWay = false;
	north = nullptr;
	south = nullptr;
	west = nullptr;
	east = nullptr;
	ant = nullptr;
	previous = nullptr;
	distanceX = 0;
	distanceY = 0;
	harvesterValue = 0;
	deltaHarvesterValue = 999;
	spawnerOwner = 0;
	dangerLevel = 0;
	totalCost = 0;
	exploreValue = 0;
}
