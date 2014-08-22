#include "Mission.h"


Mission::Mission(MapTile ^from, MapTile ^to, MissionType _type) {
	currentIndex = 0;
	type = _type;
	currentTile = from;
	targetTile = to;
	path = gcnew List<MapTile^>();
	visitedTiles = nullptr;
}

bool Mission::Equals(Mission ^other) {
	return (targetTile == other->targetTile && type == other->type);
}
