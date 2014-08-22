#include "Squad.h"


Squad::Squad() {
	ants = gcnew List<Ant^>();
	enemyAnts = gcnew List<Ant^>();
	myTotalStrength = 0;
	enemyTotalStrength = 0;
}

int Squad::numAnts() {
	return ants->Count;
}

int Squad::getAttackStrength(Ant ^ant) {
	if (ant->type() == AntType::Harvester) {
		return 2;
	} else if (ant->type() == AntType::Standard) {
		return 3;
	} else if (ant->type() == AntType::Soldier) {
		return 5;
	}
	return 0;
}

void Squad::addAnt(Ant ^ant, bool enemy) {
	if (enemy) {
		enemyAnts->Add(ant);
		enemyTotalStrength += getAttackStrength(ant);
	} else {
		ants->Add(ant);
		myTotalStrength += getAttackStrength(ant);
	}
}

bool Squad::hasHigherStrength() {
	return myTotalStrength > enemyTotalStrength;
}


List<Ant^>^ Squad::getAnts() {
	return ants;
}

List<Ant^>^ Squad::getEnemyAnts() {
	return enemyAnts;
}
