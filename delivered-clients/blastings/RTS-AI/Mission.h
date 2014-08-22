#pragma once

#include "MapTile.h"

using namespace System::Collections::Generic;

public enum class MissionType { Food, Explore, AttackSpawner, Evade, Attack, Reinforce };

public ref class Mission : IEquatable<Mission^> {
public:
	int currentIndex;
	MissionType type;
	MapTile ^currentTile;
	MapTile ^targetTile;
	List<MapTile^> ^path; //For rendering purposes.
	List<MapTile^> ^visitedTiles; //For rendering purposes.

	Mission(MapTile ^from, MapTile ^to, MissionType _type);

	virtual bool Equals(Mission ^other);
};

