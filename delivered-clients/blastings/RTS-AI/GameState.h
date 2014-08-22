#pragma once
#include "Ant.h"
#include "MapTile.h"

#include "../include/datatypes.h"
#include "../include/gui_class.h"
#include "Map.h"
#include "Mission.h"

using namespace System;
using namespace System::Collections::Generic;

public ref class GameState {
protected:
	_GUIlog ^log;
public:
	int myPlayerId;
	int numPlayers;
	unsigned int roundCounter;
	List<MapTile^> ^food;
	List<Ant^> ^myAnts; //All my ants
	List<Ant^> ^myStandardAnts; //My ants of type standard.
	List<Ant^> ^myHarvesters; //My ants of type harvester.
	List<Ant^> ^mySoldiers; //My ants of type soldier.
	List<Ant^> ^enemyAnts;
	List<MapTile^> ^mySpawners;
	List<MapTile^> ^enemySpawners;
	List<Mission^> ^antMissions;
	Map ^map;

	GameState();

	bool process(String ^data);

	void getPlayerId(String ^playerIdStr);
	void getMapSize(String ^mapSizeStr);
	void getNumPlayers(String ^numPlayersStr);
	void updateMapData(String ^mapDataStr);
	void parseMapLine(String ^line);
	void addSpawner(MapTile ^newSpawner);
	void addAnt(Ant ^newAnt);
	int getBracketEnd(Char bracketChar, Char bracketEnd, String ^str, int startIndex);
	
};

