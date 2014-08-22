#include "AntMove.h"


AntMove::AntMove(MapTile ^_fromTile, Move _move) {
	fromTile = _fromTile;
	move = _move;
	calculateToTile();
}

AntMove::AntMove(MapTile ^_fromTile, MapTile ^_toTile) {
	fromTile = _fromTile;
	toTile = _toTile;
	calculateMove();
}

String^ AntMove::toString() {
	return "[" + fromTile->position.X + ", " + fromTile->position.Y + ", " + Ant::moveFromEnum(move) + "]";
}

bool AntMove::Equals(AntMove ^other) {
	if (toTile && other && other->toTile && toTile->position == other->toTile->position) {
		return true;
	}
	return false;
}

AntMove^ AntMove::getOpposite() {
	Move newMove;
	if (move == Move::North) {
		newMove = Move::South;
	} else if (move == Move::South) {
		newMove = Move::North;
	} else if (move == Move::West) {
		newMove = Move::East;
	} else if (move == Move::East) {
		newMove = Move::West;
	}
	return gcnew AntMove(fromTile, newMove);
}

void AntMove::calculateToTile() {
	if (move == Move::North) {
		toTile = fromTile->north;
	} else if (move == Move::South) {
		toTile = fromTile->south;
	} else if (move == Move::West) {
		toTile = fromTile->west;
	} else if (move == Move::East) {
		toTile = fromTile->east;
	}
}

void AntMove::calculateMove() {
	if (toTile && fromTile) {
		int deltaX = toTile->position.X - fromTile->position.X;
		int deltaY = toTile->position.Y - fromTile->position.Y;

		if (Math::Abs(deltaX) > 1) {
			//If value is larger than 1 we need to wrap.
			deltaX *= -1;
		}
		if (Math::Abs(deltaY) > 1) {
			deltaY *= -1;
		}

		if (deltaX < 0) {
			move = Move::West;
		} else if (deltaX > 0) {
			move = Move::East;
		} else if (deltaY < 0) {
			move = Move::North;
		} else if (deltaY > 0) {
			move = Move::South;
		} else {
			move = Move::Idle;
		}
	} else {
		move = Move::Idle;
	}
}
