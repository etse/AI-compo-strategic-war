#pragma once

using namespace System;
using namespace System::Drawing;
using namespace System::Collections::Generic;

ref class AntMove;
ref class MapTile;
ref class Mission;

public enum class AntType { Standard, Harvester, Soldier, All };
public enum class Move { North, South, West, East, Idle};

public ref class Ant : IEquatable<Ant^> {
protected:
	static Random _rand;
	Point _position;
	AntType _type;
	AntMove ^_move;
	int _owner;

public:
	static String ^MOVE_NORTH = "\"north\"";
	static String ^MOVE_SOUTH = "\"south\"";
	static String ^MOVE_WEST = "\"west\"";
	static String ^MOVE_EAST = "\"east\"";
	
	Mission ^mission;
	bool isInDanger;
	int turnsToEnemy;

	Ant();

	virtual bool Equals(Ant ^other);

	void setPosition(Point newPosition);
	void setType(String ^typeStr);
	void setOwner(int ownerId);
	void setMove(AntMove ^move);
	bool hasMoved();
	int owner();
	Point position();
	AntType type();
	AntMove^ move();

	Move randomMove();
	Move moveTowards(MapTile ^tile);
	static String^ moveFromEnum(Move move);
};

