#pragma once
#include "Ant.h"

ref class Squad {
	List<Ant^> ^ants;
	List<Ant^> ^enemyAnts;

	int myTotalStrength;
	int enemyTotalStrength;

public:

	int numAnts();
	int getAttackStrength(Ant ^ant);
	void addAnt(Ant ^ant, bool enemy);
	bool hasHigherStrength();


	List<Ant^> ^getAnts();
	List<Ant^> ^getEnemyAnts();

	Squad();
};

