#pragma once

#include "MapTile.h"
#include "Ant.h"

public ref class AntMove : IEquatable<AntMove^> {
public:
	MapTile ^fromTile;
	MapTile ^toTile;
	Move move;

	AntMove(MapTile ^_fromTile, Move _move);
	AntMove(MapTile ^_fromTile, MapTile ^_toTile);

	String^ toString();
	virtual bool Equals(AntMove ^other);

	AntMove^ getOpposite();

protected:
	void calculateToTile();
	void calculateMove();
};

