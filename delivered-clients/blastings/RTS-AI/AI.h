#pragma once

#include "GameState.h"
#include "..\include\gui_class.h"

using namespace System;
using namespace System::Drawing;

ref class PriorityQueue;
ref class Squad;

public ref class AI {
protected:
	static const unsigned int MAX_VIEW_DISTANCE = 55;
	static const double FRIENDLY_MODIFIER = 0.25;
	static const unsigned int EVADE_DISTANCE = 40;
	static const unsigned int ALLIES_STEPS = 15;
	static const unsigned int ATTACK_RANGE = 1;
	static const unsigned int MAX_PATH = 20;
	static const unsigned int MAX_DIRECTED_PATH = 60;

	static String ^ANT_HARVESTER = "harvester";
	static String ^ANT_SOLDIER = "soldier";
	static String ^ANT_STANDARD = "standard";


	_GUIlog ^log;
	List<AntMove^> ^antTargets;
	List<MapTile^> ^borderTiles;
	GameState ^gameState;

public:
	AI(GameState ^gameState);

	String^ getCommands();
	int getSquareDistance(Point from, Point to);
	int getSquareDistance(int deltaX, int deltaY);
	void processMissions();
	void tileCheck(MapTile ^currentTile, MapTile ^previous, List<MapTile^> ^visitedTiles, Queue<MapTile^> ^queue, int distanceX, int distanceY, bool checkAnt);
	void enemyTileCheck(MapTile ^currentTile, MapTile ^previous, Queue<MapTile^> ^queue, int distanceX, int distanceY);
	void evadeTileCheck(MapTile ^currentTile, MapTile ^previous, List<MapTile^> ^visitedTiles, Queue<MapTile^> ^queue, int distanceX, int distanceY);
	void alliesTileCheck(MapTile ^currentTile, MapTile ^previous, Squad ^currentSquad, List<MapTile^> ^visitedTiles, Queue<MapTile^> ^queue, int distanceX, int distanceY);
	void tileAStarCheck(MapTile ^currentTile, MapTile ^previous, MapTile ^target, List<MapTile^> ^visitedTiles, PriorityQueue ^queue, int distanceX, int distanceY, bool checkAnt);
	void attackTileCheck(MapTile ^currentTile, MapTile ^previous, MapTile ^target, List<MapTile^> ^visitedTiles, PriorityQueue ^queue, int distanceX, int distanceY);
	void tileViewCheck(MapTile ^currentTile, MapTile ^previous, Queue<MapTile^> ^queue, int distanceX, int distanceY);
	void assignAntsToTiles(List<MapTile^> ^targetTiles, AntType type, MissionType missionType, bool allAntTypes);
	void getViewArea();
	void calcEnemy();
	void evadeOrAttack();


	void sendReinforcements();
	void findCombatPath(Squad ^squad);
	void findPathMission(Mission ^mission, bool firstTileAntCheck);
	void evade(Ant ^ant);
	bool hasAllies(Ant ^ant, Squad ^squad);
	void calculateCombat(Squad ^squad);
	void distributeHarvesters();
	void explore();
	void giveRandomMove();
	void removeMission(Ant ^ant);

	bool isValidMove(AntMove ^move, bool checkAnt, bool checkWall);
	String^ getSpawnMode();

};

