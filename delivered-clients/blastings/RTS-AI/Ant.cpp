#include "Ant.h"
#include "MapTile.h"


Ant::Ant() {
	_position = Point(0);
	_type = AntType::Standard;
	_owner = -1;
	_move = nullptr;
	mission = nullptr;
	isInDanger = false;
	turnsToEnemy = 0;
	//path = gcnew List<MapTile^>();
}

bool Ant::Equals(Ant ^other) {
	return (_position == other->position());
}

void Ant::setPosition(Point newPosition) {
	_position = newPosition;
}

void Ant::setType(String ^typeStr) {
	if (typeStr->Trim() == "\"harvester\"") {
		_type = AntType::Harvester;
	} else if (typeStr->Trim() == "\"soldier\"") {
		_type = AntType::Soldier;
	} else {
		_type = AntType::Standard;
	}

}

void Ant::setOwner(int ownerId) {
	_owner = ownerId;
}

void Ant::setMove(AntMove ^move) {
	_move = move;
}

bool Ant::hasMoved() {
	return _move != nullptr;
}

int Ant::owner() {
	return _owner;
}

Point Ant::position() {
	return _position;
}

AntType Ant::type() {
	return _type;
}

AntMove^ Ant::move() {
	return _move;
}

Move Ant::randomMove() {
	int x = _rand.Next() % 4;
	return (Move)x;
}

Move Ant::moveTowards(MapTile ^tile) {
	int dirX = tile->position.X - _position.X;
	int dirY = tile->position.Y - _position.Y;
	int absX = Math::Abs(dirX);
	int absY = Math::Abs(dirY);

	Move move = Move::Idle;
	if (absX > absY) {
		if (dirX > 0) {
			move = Move::East;
		} else if (dirX < 0) {
			move = Move::West;
		}
	} else if (absX > 0 && absX == absY) { //If equal distance and not null, move randomly.
		move = randomMove();
	} else {
		if (dirY > 0) {
			move = Move::South;
		} else if (dirY < 0) {
			move = Move::North;
		}
	}
	return move;
}

String^ Ant::moveFromEnum(Move move) {
	switch (move) {
	case(Move::North) : return MOVE_NORTH;
	case(Move::South) : return MOVE_SOUTH;
	case(Move::West) : return MOVE_WEST;
	case(Move::East) : return MOVE_EAST;
	}
	return "";
}
