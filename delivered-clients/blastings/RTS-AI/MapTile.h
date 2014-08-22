#pragma once

#include "Ant.h"
#include "PriorityQueueNode.h"

using namespace System::Drawing;

public ref class MapTile : PriorityQueueNode {
public:
	Point position;
	bool isWall;
	bool hasFood;
	bool isVisible;
	bool visited;
	bool added;
	bool isBorderTile;
	bool isSpawner;
	bool isSpawnerDestroyed;
	bool isBehindWall;
	bool pathFound;
	bool explorerOnTheWay;

	int distanceX;
	int distanceY;
	int harvesterValue;
	int deltaHarvesterValue;
	int spawnerOwner;
	int dangerLevel;
	int totalCost;
	int exploreValue;

	Ant ^ant;
	MapTile ^north;
	MapTile ^south;
	MapTile ^west;
	MapTile ^east;
	MapTile ^previous;


	MapTile(int posX, int posY);
};

