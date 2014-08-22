#include "AI.h"

#include "Ant.h"
#include "AntMove.h"
#include "MapTile.h"
#include "PriorityQueue.h"
#include "Squad.h"

AI::AI(GameState ^_gameState) {
	log = _GUIlog::getInstance();
	antTargets = gcnew List<AntMove^>();
	borderTiles = gcnew List<MapTile^>();
	gameState = _gameState;
}

String^ AI::getCommands() {
	antTargets->Clear();
	//{"mode": spawn_mode, "moves": [[x, y, direction], [x, y, direction] ...]}\n
	StringBuilder ^commandBuilder = gcnew StringBuilder();
	String ^spawnMode = getSpawnMode();
	commandBuilder->Append("{\"mode\": \"" + spawnMode + "\"");
	calcEnemy();
	processMissions();
	getViewArea();
	evadeOrAttack();
	sendReinforcements();
	assignAntsToTiles(gameState->enemySpawners, AntType::All, MissionType::AttackSpawner, true);
	assignAntsToTiles(gameState->food, AntType::Harvester, MissionType::Food, false);
	distributeHarvesters();
	explore();
	//Try a random move for ants that have not been given a position.
	giveRandomMove();

	

	if (antTargets->Count > 0) {
		StringBuilder ^moveBuilder = gcnew StringBuilder();
		commandBuilder->Append(", \"moves\": [");
		for each (AntMove ^aMove in antTargets) {
			moveBuilder->Append(aMove->toString());
			moveBuilder->Append(", ");
		}
		String ^moves = moveBuilder->ToString();
		moves = moves->Remove(moves->Length - 2);
		commandBuilder->Append(moves);
		commandBuilder->Append("]");
	}
	commandBuilder->Append("}");
	return commandBuilder->ToString();
}

void AI::processMissions() {
	if (gameState->antMissions->Count > 0) {
		int missionCounter = 0;
		do {
			Mission ^mission = gameState->antMissions[missionCounter];
			bool missionDone = false;
			Ant ^ant = mission->currentTile->ant;
			if (mission->type == MissionType::Food) {
				missionDone = !mission->targetTile->hasFood;
			} else if (mission->type == MissionType::Explore) {
				MapTile ^targetTile = mission->targetTile;
				if (ant && (ant->type() != AntType::Harvester)) {
					missionDone = targetTile->exploreValue <= 0;
				}
			} else if (mission->type == MissionType::AttackSpawner) {
				missionDone = mission->targetTile->isSpawnerDestroyed;
			} else if (mission->type == MissionType::Attack) {
				missionDone = true;
			} else if (mission->currentIndex <= 0) {
				missionDone = true;
			}

			if (ant && !missionDone) {
				//log->write("round(" + gameState->roundCounter + ") Mission Ant found at : " + mission->currentTile->position.ToString());
				
				AntMove ^newMove = nullptr;
				MapTile ^nextTile = nullptr;
				int nextTileIndex = 0;
				if (mission->currentIndex > 0 && mission->currentIndex < mission->path->Count) {
					nextTileIndex = mission->currentIndex - 1;
					nextTile = mission->path[nextTileIndex];
					newMove = gcnew AntMove(mission->currentTile, nextTile);
				}
				
				if (!nextTile || !newMove || !isValidMove(newMove, true, true)) {
					//Calculate new path
					findPathMission(mission, true);
					if (mission->path->Count > 1) {
						//Path found
						nextTileIndex = mission->path->Count - 2;
						nextTile = mission->path[nextTileIndex];
						newMove = gcnew AntMove(mission->currentTile, nextTile);
						//if (!antTargets->Contains(newMove) && newMove->move != Move::Idle) {

					}
				}
				if (newMove) {
					if (isValidMove(newMove, true, true)) {
						antTargets->Add(newMove);
						ant->setMove(newMove);
						ant->mission = mission;
						//Move the ant from one tile to another 
						mission->currentTile->ant = nullptr;
						mission->currentTile = nextTile;
						mission->currentIndex = nextTileIndex;
						nextTile->ant = ant;
					}
				} else {
					//No valid move found, end mission.
					missionDone = true;
				}
				//Done processing this mission move to next.
				missionCounter++;
			} else {
				//log->write("round(" + gameState->roundCounter + ") No ant found at : " + mission->currentTile->position.ToString());
				//No ant found, remove mission.
				gameState->antMissions[missionCounter] = gameState->antMissions[gameState->antMissions->Count - 1]; //Swap with the last mission.
				gameState->antMissions->RemoveAt(gameState->antMissions->Count - 1); //Removing the last element is O(1) instead of O(count - index).
			}

		} while (missionCounter < gameState->antMissions->Count);
	}
}

void AI::assignAntsToTiles(List<MapTile^> ^targetTiles, AntType type, MissionType missionType, bool allAntTypes) {
	for each (MapTile ^targetTile in targetTiles) {
		if (missionType == MissionType::AttackSpawner && targetTile->isSpawner && targetTile->isSpawnerDestroyed) {
			continue; //Skip when already destroyed;
		}
		List<MapTile^> ^visitedTiles = gcnew List<MapTile^>();
		Queue<MapTile^> ^queue = gcnew Queue<MapTile^>(0); //Queue of tiles to check
		queue->Enqueue(targetTile);
		visitedTiles->Add(targetTile);
		targetTile->distanceX = 0;
		targetTile->distanceY = 0;
		Ant ^standardAnt = nullptr;
		bool antOnTheWay = false;
		do {
			MapTile ^currentTile = queue->Dequeue();
			if (!currentTile->visited && !currentTile->isWall) {
				int squaredDistance = getSquareDistance(currentTile->distanceX, currentTile->distanceY);
				if (squaredDistance <= MAX_VIEW_DISTANCE) {
					currentTile->visited = true;

					Ant ^currentAnt = currentTile->ant;
					if (currentAnt && !currentAnt->isInDanger && (currentAnt->owner() == gameState->myPlayerId) && (currentAnt->type() == type || allAntTypes)) {
						if (currentAnt->mission && currentAnt->mission->type != MissionType::Explore) {
							if (targetTile == currentAnt->mission->targetTile) {
								//Ant is on it's way
								antOnTheWay = true;
								break;
							}
						} else {
							//We found a harvester, let's move it towards food.
							AntMove ^newMove = gcnew AntMove(currentTile, currentTile->previous);
							//if (!currentTile->previous->ant && !antTargets->Contains(newMove) && newMove->move != Move::Idle) {
							if (isValidMove(newMove, true, false)) {
								removeMission(currentAnt);
								antTargets->Add(newMove);
								currentAnt->setMove(newMove);
								//Move the ant from one tile to another 
								currentTile->ant = nullptr;
								currentTile->previous->ant = currentAnt;
								Mission ^newMission = gcnew Mission(newMove->toTile, targetTile, missionType);
								//log->write("round(" + gameState->roundCounter + ") Ant found at : " + currentTile->position.ToString() + " mission target : " + newMission->currentTile->position.ToString());
								currentAnt->mission = newMission;
								gameState->antMissions->Add(newMission);
								MapTile ^previous = currentTile->previous;
								while(previous) {
									newMission->path->Add(previous);
									if (newMission->path->Count > MAX_PATH) {
										log->write("Path too long!");
										log->write("Tile: " + previous->position.ToString());										
										break;
									}
									previous = previous->previous;
								} 
								antOnTheWay = true;
								break; //Target found for this food. Skip to next.
							}
						}
					} else {
						if (missionType == MissionType::Food) {
							if (currentAnt && currentAnt->type() == AntType::Standard) {
								if (!currentAnt->mission || (currentAnt->mission && currentAnt->mission->type == MissionType::Explore)) {
									standardAnt = currentAnt;
								}
							}
						}
					}
					//Add neighbours if they are not visited.
					tileCheck(currentTile->north, currentTile, visitedTiles, queue, currentTile->distanceX, currentTile->distanceY + 1, false);
					tileCheck(currentTile->south, currentTile, visitedTiles, queue, currentTile->distanceX, currentTile->distanceY + 1, false);
					tileCheck(currentTile->east, currentTile, visitedTiles, queue, currentTile->distanceX + 1, currentTile->distanceY, false);
					tileCheck(currentTile->west, currentTile, visitedTiles, queue, currentTile->distanceX + 1, currentTile->distanceY, false);
				}
			}
		} while (queue->Count > 0);
		if (!antOnTheWay && standardAnt) {
			MapTile ^currentTile = gameState->map->getTile(standardAnt->position());
			AntMove ^newMove = gcnew AntMove(currentTile, currentTile->previous);
			if (isValidMove(newMove, true, false)) {
				removeMission(standardAnt);
				antTargets->Add(newMove);
				standardAnt->setMove(newMove);
				currentTile->ant = nullptr;
				currentTile->previous->ant = standardAnt;
				Mission ^newMission = gcnew Mission(newMove->toTile, targetTile, missionType);
				standardAnt->mission = newMission;
				gameState->antMissions->Add(newMission);
				MapTile ^previous = currentTile->previous;
				while (previous) {
					newMission->path->Add(previous);
					if (newMission->path->Count > MAX_PATH) {
						log->write("Path too long!");
						log->write("Tile: " + previous->position.ToString());
						break;
					}
					previous = previous->previous;
				}
				antOnTheWay = true;
			}
		}
		//Reset visited state before checking next targettile.
		for each (MapTile ^visitedTile in visitedTiles) {
			visitedTile->visited = false;
			visitedTile->previous = nullptr;
		}
	}
}

void AI::tileCheck(MapTile ^currentTile, MapTile ^previous, List<MapTile^> ^visitedTiles, Queue<MapTile^> ^queue, int distanceX, int distanceY, bool checkAnt) {
	if (checkAnt && currentTile->ant) return;
	if (!currentTile->visited && currentTile != currentTile->previous) {
		currentTile->distanceX = distanceX;
		currentTile->distanceY = distanceY;
		currentTile->previous = previous;
		queue->Enqueue(currentTile);
		visitedTiles->Add(currentTile);
	}
}

void AI::enemyTileCheck(MapTile ^currentTile, MapTile ^previous, Queue<MapTile^> ^queue, int distanceX, int distanceY) {
	if (!currentTile->visited && currentTile != currentTile->previous) {
		currentTile->distanceX = distanceX;
		currentTile->distanceY = distanceY;
		queue->Enqueue(currentTile);
	}
}

void AI::evadeTileCheck(MapTile ^currentTile, MapTile ^previous, List<MapTile^> ^visitedTiles, Queue<MapTile^> ^queue, int distanceX, int distanceY) {
	if (currentTile == previous->previous) return;
	int totalCost = currentTile->dangerLevel + previous->totalCost;
	if (!currentTile->visited) {
		currentTile->visited = true;
		currentTile->distanceX = distanceX;
		currentTile->distanceY = distanceY;
		currentTile->totalCost = totalCost;
		currentTile->previous = previous;
		visitedTiles->Add(currentTile);
		queue->Enqueue(currentTile);
	} /*else if(totalCost < currentTile->totalCost) {
		//Tile has been visited before, but this path is better.
		currentTile->distanceX = distanceX;
		currentTile->distanceY = distanceY;
		currentTile->totalCost = totalCost;
		currentTile->previous = previous;
	}*/
}

void AI::alliesTileCheck(MapTile ^currentTile, MapTile ^previous, Squad ^currentSquad, List<MapTile^> ^visitedTiles, Queue<MapTile^> ^queue, int distanceX, int distanceY) {
	if (currentTile == previous->previous) return;	
	if (!currentTile->added) {
		if (currentTile->ant) {
			if (currentTile->ant->owner() == gameState->myPlayerId) {
				currentSquad->addAnt(currentTile->ant, false);
			} else {
				currentSquad->addAnt(currentTile->ant, true);
			}
		}
		currentTile->added = true;
		currentTile->distanceX = distanceX;
		currentTile->distanceY = distanceY;
		currentTile->previous = previous;
		queue->Enqueue(currentTile);
		visitedTiles->Add(currentTile);
	} 
}

void AI::tileAStarCheck(MapTile ^currentTile, MapTile ^previous, MapTile ^target, List<MapTile^> ^visitedTiles, PriorityQueue ^queue, int distanceX, int distanceY, bool checkAnt) {
	if (checkAnt && currentTile->ant) return;
	if (!currentTile->added && !currentTile->visited && currentTile != currentTile->previous) {
		currentTile->distanceX = distanceX;
		currentTile->distanceY = distanceY;
		currentTile->previous = previous;
		currentTile->added = true;
		double priority = (double)getSquareDistance(currentTile->position, target->position);
		queue->enqueue(currentTile, priority);
		visitedTiles->Add(currentTile);		
	}
}

void AI::attackTileCheck(MapTile ^currentTile, MapTile ^previous, MapTile ^target, List<MapTile^> ^visitedTiles, PriorityQueue ^queue, int distanceX, int distanceY) {
	if (currentTile->ant || currentTile->isWall) return;
	if (!currentTile->added && !currentTile->visited && currentTile != currentTile->previous) {
		currentTile->distanceX = distanceX;
		currentTile->distanceY = distanceY;
		currentTile->previous = previous;
		currentTile->added = true;
		double priority = (double)getSquareDistance(currentTile->position, target->position);
		queue->enqueue(currentTile, priority);
		visitedTiles->Add(currentTile);
	}
}

int AI::getSquareDistance(Point from, Point to) {
	int deltaX = Math::Abs(to.X - from.X);
	int deltaY = Math::Abs(to.Y - from.Y);
	return Math::Min(deltaX, gameState->map->width() - deltaX) + Math::Min(deltaY, gameState->map->height() - deltaY);
}

int AI::getSquareDistance(int deltaX, int deltaY) {
	return deltaX * deltaX + deltaY * deltaY;
}

void AI::tileViewCheck(MapTile ^currentTile, MapTile ^previous, Queue<MapTile^> ^queue, int distanceX, int distanceY) {
	if (!currentTile->visited) {
		currentTile->distanceX = distanceX;
		currentTile->distanceY = distanceY;
		queue->Enqueue(currentTile);
	}

	if (!currentTile->pathFound && (currentTile->isWall || previous->isWall || previous->isBehindWall)) {
		currentTile->isBehindWall = true;		
	}

	if (/*currentTile->isBehindWall &&*/ !currentTile->isWall && !previous->isWall && !previous->isBehindWall) {
		currentTile->isBehindWall = false;
		currentTile->pathFound = true;
		currentTile->exploreValue = 0;
		currentTile->explorerOnTheWay = false;
		/*currentTile->visited = false;
		queue->Enqueue(currentTile);*/
	}	
}

void AI::getViewArea() {
	borderTiles->Clear();
	List<MapTile^> ^visitedTiles = gcnew List<MapTile^>();
	Queue<MapTile^> ^queue = gcnew Queue<MapTile^>(0); //Queue of tiles to check
	for each(Ant ^ant in gameState->myAnts) {
		MapTile ^antTile = gameState->map->getTile(ant->position());
		antTile->isBehindWall = false;
		antTile->distanceX = 0;
		antTile->distanceY = 0;
		queue->Enqueue(antTile);
		do {
			MapTile ^currentTile = queue->Dequeue();
			int squaredDistance = getSquareDistance(currentTile->distanceX, currentTile->distanceY);
			if (squaredDistance <= MAX_VIEW_DISTANCE) {
				if (!currentTile->visited && !currentTile->isBehindWall) {
					if (ant->type() == AntType::Harvester) {
						currentTile->harvesterValue += MAX_VIEW_DISTANCE - squaredDistance;
					}
					currentTile->dangerLevel -= (int)((double)(MAX_VIEW_DISTANCE - squaredDistance) * FRIENDLY_MODIFIER);
				}
				currentTile->isVisible = true;
				currentTile->visited = true;
				currentTile->isBorderTile = false;
				visitedTiles->Add(currentTile);
				//Add neighbours if they are not visited.
				tileViewCheck(currentTile->north, currentTile, queue, currentTile->distanceX, currentTile->distanceY + 1);
				tileViewCheck(currentTile->south, currentTile, queue, currentTile->distanceX, currentTile->distanceY + 1);
				tileViewCheck(currentTile->east, currentTile, queue, currentTile->distanceX + 1, currentTile->distanceY);
				tileViewCheck(currentTile->west, currentTile, queue, currentTile->distanceX + 1, currentTile->distanceY);
			} else {
				//We have reached the border of the view area.
				if (!currentTile->isVisible) {
					currentTile->isBorderTile = true;
					borderTiles->Add(currentTile);
				}
			}	
		} while (queue->Count > 0);		
		//Reset visited state before checking next ant.
		for each (MapTile ^visitedTile in visitedTiles) {
			visitedTile->visited = false;
		}
	}
}

void AI::calcEnemy() {
	List<MapTile^> ^visitedTiles = gcnew List<MapTile^>();
	Queue<MapTile^> ^queue = gcnew Queue<MapTile^>(0); //Queue of tiles to check
	for each(Ant ^ant in gameState->enemyAnts) {
		MapTile ^antTile = gameState->map->getTile(ant->position());
		antTile->distanceX = 0;
		antTile->distanceY = 0;
		queue->Enqueue(antTile);
		do {
			MapTile ^currentTile = queue->Dequeue();
			int squaredDistance = getSquareDistance(currentTile->distanceX, currentTile->distanceY);
			if (squaredDistance <= MAX_VIEW_DISTANCE) {
				if (!currentTile->visited && !currentTile->isBehindWall) {
					currentTile->dangerLevel += (int)((double)(MAX_VIEW_DISTANCE - squaredDistance) * FRIENDLY_MODIFIER);
				}
				if (currentTile->ant && currentTile->ant->owner() == gameState->myPlayerId) {
					currentTile->ant->isInDanger = true;
				}
				currentTile->visited = true;
				visitedTiles->Add(currentTile);
				//Add neighbours if they are not visited.
				enemyTileCheck(currentTile->north, currentTile, queue, currentTile->distanceX, currentTile->distanceY + 1);
				enemyTileCheck(currentTile->south, currentTile, queue, currentTile->distanceX, currentTile->distanceY + 1);
				enemyTileCheck(currentTile->east, currentTile, queue, currentTile->distanceX + 1, currentTile->distanceY);
				enemyTileCheck(currentTile->west, currentTile, queue, currentTile->distanceX + 1, currentTile->distanceY);
			}
		} while (queue->Count > 0);
		//Reset visited state before checking next ant.
		for each (MapTile ^visitedTile in visitedTiles) {
			visitedTile->visited = false;
		}
	}
}

void AI::evadeOrAttack() {
	for each (Ant^ ant in gameState->myAnts) {
		if (ant->isInDanger) {
			Squad ^squad = gcnew Squad();
			if (hasAllies(ant, squad) && squad->hasHigherStrength() && squad->getEnemyAnts()->Count > 0) {	//Compare combat strength and attack range etc.
				//Nearby friendly ants found.
				calculateCombat(squad);
			} else {
				evade(ant);
			}
		}
	}
}

void AI::sendReinforcements() {
	//Send newly spawned ants to reinforce dangered ants.
	for each (MapTile ^spawner in gameState->mySpawners) {
		if (spawner->ant && spawner->ant->type() != AntType::Harvester) {
			for each(Ant ^ant in gameState->myAnts) {
				if (ant->isInDanger) {
					Mission ^mission = gcnew Mission(spawner, gameState->map->getTile(ant->position()), MissionType::Reinforce);
					spawner->ant->mission = mission;
					gameState->antMissions->Add(mission);
					findPathMission(mission, true);
					if (mission->path->Count > 1) {
						//Path found
						int nextTileIndex = mission->path->Count - 2;
						MapTile ^nextTile = mission->path[nextTileIndex];
						AntMove ^newMove = gcnew AntMove(mission->currentTile, nextTile);
						if (isValidMove(newMove, false, false)) {
							removeMission(ant);
							antTargets->Add(newMove);
							ant->setMove(newMove);
							ant->mission = mission;
							//Move the ant from one tile to another 
							mission->currentTile->ant = nullptr;
							mission->currentTile = nextTile;
							mission->currentIndex = nextTileIndex;
							nextTile->ant = ant;
						}
					}
					break;
				}
			}
		}
	}

	for each (Ant ^soldier in gameState->mySoldiers) {
		if (soldier->mission && soldier->mission->type != MissionType::Explore) continue;
		Ant ^closestDangerAnt = nullptr;
		int dangerAntRange;
		for each(Ant ^ant in gameState->myAnts) {
			if (ant->isInDanger) {
				int range = getSquareDistance(soldier->position(), ant->position());
				if (!closestDangerAnt || range < dangerAntRange) {
					dangerAntRange = range;
					closestDangerAnt = ant;
				}
			}
		}
		if (closestDangerAnt) {
			MapTile ^antTile = gameState->map->getTile(closestDangerAnt->position());
			MapTile ^soldierTile = gameState->map->getTile(soldier->position());
			Mission ^reinforce = gcnew Mission(soldierTile, antTile, MissionType::Reinforce);
			findPathMission(reinforce, true);
			if (reinforce->path->Count > 1) {
				//Path found
				int nextTileIndex = reinforce->path->Count - 2;
				MapTile ^nextTile = reinforce->path[nextTileIndex];
				AntMove ^newMove = gcnew AntMove(reinforce->currentTile, nextTile);
				if (isValidMove(newMove, false, false)) {
					removeMission(soldier);
					antTargets->Add(newMove);
					soldier->setMove(newMove);
					soldier->mission = reinforce;
					reinforce->currentTile->ant = nullptr;
					reinforce->currentTile = nextTile;
					reinforce->currentIndex = nextTileIndex;
					nextTile->ant = soldier;
					gameState->antMissions->Add(reinforce);
				}
			}
		}
	}
}

void AI::findCombatPath(Squad ^squad) {
	List<Ant^> ^enemyAnts = squad->getEnemyAnts();
	if (enemyAnts->Count <= 0) return;

	for each (Ant ^ant in squad->getAnts()) {
		Ant ^closestEnemy = nullptr;
		int closestDistance = 100;
		for each (Ant ^enemyAnt in enemyAnts) {
			int distance = getSquareDistance(ant->position(), enemyAnt->position());
			if (!closestEnemy || distance < closestDistance) {
				closestDistance = distance;
				closestEnemy = enemyAnt;
			}
		}

		//No enemy ant found.
		if (!closestEnemy) continue;

		MapTile ^from = gameState->map->getTile(ant->position());
		MapTile ^to = gameState->map->getTile(closestEnemy->position());
		int maxDistance = 100;
		List<MapTile^> ^visitedTiles = gcnew List<MapTile^>();
		PriorityQueue ^pqueue = gcnew PriorityQueue(100);
		from->distanceX = 0;
		from->distanceY = 0;
		pqueue->enqueue(from, 0.0);
		visitedTiles->Add(from);
		int distanceToEnemy = getSquareDistance(from->position, to->position);
		Mission ^mission = gcnew Mission(from, to, MissionType::Attack);
		do {
			MapTile ^currentTile = (MapTile^)pqueue->dequeue();
			int squaredDistance = getSquareDistance(currentTile->distanceX, currentTile->distanceY);
			if ((distanceToEnemy - squaredDistance) <= ATTACK_RANGE) { //We have reached a tile where we can attack the enemy.
				MapTile ^previous = currentTile->previous;
				while (previous) {
					mission->path->Add(previous);
					if (mission->path->Count > MAX_DIRECTED_PATH) {
						log->write("Path too long!");
						log->write("Tile: " + previous->position.ToString());
						break;
					}
					previous = previous->previous;
				}
				removeMission(ant);
				gameState->antMissions->Add(mission);
				ant->mission = mission;
				break;
			}
			if (!currentTile->visited) {
				if (squaredDistance <= maxDistance) {
					currentTile->visited = true;

					//Add neighbours if they are not visited.
					attackTileCheck(currentTile->north, currentTile, to, visitedTiles, pqueue, currentTile->distanceX, currentTile->distanceY + 1);
					attackTileCheck(currentTile->south, currentTile, to, visitedTiles, pqueue, currentTile->distanceX, currentTile->distanceY + 1);
					attackTileCheck(currentTile->east, currentTile, to, visitedTiles, pqueue, currentTile->distanceX + 1, currentTile->distanceY);
					attackTileCheck(currentTile->west, currentTile, to, visitedTiles, pqueue, currentTile->distanceX + 1, currentTile->distanceY);
				}
			}
		} while (pqueue->count() > 0);
		
		//Reset visited state before checking next ant.
		for each (MapTile ^visitedTile in visitedTiles) {
			visitedTile->visited = false;
			visitedTile->added = false;
			visitedTile->previous = nullptr;
		}
	}
}

void AI::findPathMission(Mission ^mission, bool firstTileAntCheck) {
	MapTile ^to = mission->targetTile;
	MapTile ^from = mission->currentTile;
	int maxDistance = 200;
	mission->path->Clear();
	List<MapTile^> ^visitedTiles = gcnew List<MapTile^>();
	PriorityQueue ^pqueue = gcnew PriorityQueue(100);
	from->distanceX = 0;
	from->distanceY = 0;
	pqueue->enqueue(from, 0.0); 
	visitedTiles->Add(from);
	do {
		MapTile ^currentTile = (MapTile^)pqueue->dequeue();
		int squaredDistance = getSquareDistance(currentTile->distanceX, currentTile->distanceY);
		if (currentTile == to || squaredDistance >= maxDistance) { //Just create path to the tile we found when out of distance.
			//Target found
			MapTile ^previous = currentTile->previous;
			while(previous) {
				mission->path->Add(previous);
				if (mission->path->Count > MAX_DIRECTED_PATH) {
					log->write("Path too long!");
					log->write("Tile: " + previous->position.ToString());
					break;
				}
				previous = previous->previous;
			}
			break;
		}
		if (!currentTile->visited && !currentTile->isWall) {
			if (squaredDistance <= maxDistance) {
				currentTile->visited = true;

				//Add neighbours if they are not visited.
				tileAStarCheck(currentTile->north, currentTile, to, visitedTiles, pqueue, currentTile->distanceX, currentTile->distanceY + 1, firstTileAntCheck);
				tileAStarCheck(currentTile->south, currentTile, to, visitedTiles, pqueue, currentTile->distanceX, currentTile->distanceY + 1, firstTileAntCheck);
				tileAStarCheck(currentTile->east, currentTile, to, visitedTiles, pqueue, currentTile->distanceX + 1, currentTile->distanceY, firstTileAntCheck);
				tileAStarCheck(currentTile->west, currentTile, to, visitedTiles, pqueue, currentTile->distanceX + 1, currentTile->distanceY, firstTileAntCheck);
			}
		}
		firstTileAntCheck = false;
	} while (pqueue->count() > 0);
	//Reset visited state before checking next ant.
	for each (MapTile ^visitedTile in visitedTiles) {
		visitedTile->visited = false;
		visitedTile->added = false;
		visitedTile->previous = nullptr;
	}
	mission->visitedTiles = visitedTiles;
}

void AI::evade(Ant ^ant) {
	List<MapTile^> ^visitedTiles = gcnew List<MapTile^>();
	Queue<MapTile^> ^queue = gcnew Queue<MapTile^>(0); //Queue of tiles to check
	MapTile ^antTile = gameState->map->getTile(ant->position());
	MapTile ^safestTile = antTile;
	queue->Enqueue(antTile);
	visitedTiles->Add(antTile);
	antTile->distanceX = 0;
	antTile->distanceY = 0;
	do {
		MapTile ^currentTile = queue->Dequeue();
		if (!currentTile->isWall) {
			int squaredDistance = getSquareDistance(currentTile->distanceX, currentTile->distanceY);
			if (squaredDistance <= EVADE_DISTANCE) {
				//Add neighbours if they are not visited.
				evadeTileCheck(currentTile->north, currentTile, visitedTiles, queue, currentTile->distanceX, currentTile->distanceY + 1);
				evadeTileCheck(currentTile->south, currentTile, visitedTiles, queue, currentTile->distanceX, currentTile->distanceY + 1);
				evadeTileCheck(currentTile->east, currentTile, visitedTiles, queue, currentTile->distanceX + 1, currentTile->distanceY);
				evadeTileCheck(currentTile->west, currentTile, visitedTiles, queue, currentTile->distanceX + 1, currentTile->distanceY);
			}
		}
	} while (queue->Count > 0);

	for each (MapTile ^visitedTile in visitedTiles) {
		if (visitedTile->totalCost < safestTile->totalCost) {
			//We found a safer tile
			safestTile = visitedTile;
		}
	}

	//Create a mission to the safest tile.
	Mission ^mission = gcnew Mission(antTile, safestTile, MissionType::Evade);
	gameState->antMissions->Add(mission);
	MapTile ^previous = safestTile->previous;
	while (previous) {
		mission->path->Add(previous);
		if (mission->path->Count > MAX_PATH) {
			log->write("Path too long!");
			log->write("Tile: " + previous->position.ToString());
			break;
		}
		previous = previous->previous;
	}

	if (mission->path->Count > 1) {
		MapTile ^nextTile = mission->path[mission->path->Count - 2];
		AntMove ^newMove = gcnew AntMove(antTile, nextTile);
		if (isValidMove(newMove, true, false)) {
			removeMission(ant);
			antTargets->Add(newMove);
			ant->setMove(newMove);
			ant->mission = mission;
			//Move the ant from one tile to another 
			antTile->ant = nullptr;
			mission->currentTile = nextTile;
			nextTile->ant = ant;
		}
	}

	for each (MapTile ^visitedTile in visitedTiles) {
		visitedTile->visited = false;
		visitedTile->previous = nullptr;
		visitedTile->totalCost = 0;
	}
}

bool AI::hasAllies(Ant ^ant, Squad ^squad) {
	//List<Ant^> ^squad = gcnew List<Ant^>();
	List<MapTile^> ^visitedTiles = gcnew List<MapTile^>();
	Queue<MapTile^> ^queue = gcnew Queue<MapTile^>(0); //Queue of tiles to check
	MapTile ^antTile = gameState->map->getTile(ant->position());
	squad->addAnt(ant, false);
	antTile->distanceX = 0;
	antTile->distanceY = 0;
	queue->Enqueue(antTile);
	visitedTiles->Add(antTile);
	do {
		MapTile ^currentTile = queue->Dequeue();
		if (!currentTile->isWall) {
			int stepDistance = currentTile->distanceX + currentTile->distanceY;
			/*if (!currentTile->added) {
				visitedTiles->Add(currentTile);
			}*/
			if (!currentTile->visited && stepDistance <= ALLIES_STEPS) {
				currentTile->visited = true;
				alliesTileCheck(currentTile->north, currentTile, squad, visitedTiles, queue, currentTile->distanceX, currentTile->distanceY + 1);
				alliesTileCheck(currentTile->south, currentTile, squad, visitedTiles, queue, currentTile->distanceX, currentTile->distanceY + 1);
				alliesTileCheck(currentTile->east, currentTile, squad, visitedTiles, queue, currentTile->distanceX + 1, currentTile->distanceY);
				alliesTileCheck(currentTile->west, currentTile, squad, visitedTiles, queue, currentTile->distanceX + 1, currentTile->distanceY);
			}
		}
	} while (queue->Count > 0);

	for each (MapTile ^visitedTile in visitedTiles) {
		visitedTile->visited = false;
		visitedTile->previous = nullptr;
		visitedTile->added = false;
	}

	return squad->numAnts() > 1;
}

void AI::calculateCombat(Squad ^squad) {
	//Locate enemy ants. Done in the hasAllies function.
	//We have higher cumulative strength.
	findCombatPath(squad); //Calculates path to the closest enemy.
	int largestNumTurns = 0;
	for each(Ant ^ant in squad->getAnts()) {
		if (ant->mission) {
			ant->turnsToEnemy = ant->mission->path->Count - 1; //Need to subtract the tile where the ant is standing.
			if (ant->turnsToEnemy > largestNumTurns) {
				largestNumTurns = ant->turnsToEnemy;
			}
		}
	}

	for each(Ant ^ant in squad->getAnts()) {
		if (ant->turnsToEnemy < 1) continue;
		MapTile ^antTile = gameState->map->getTile(ant->position());
		MapTile ^attackTile = ant->mission->path[ant->turnsToEnemy - 1]; //Find next tile in the combat path.
		AntMove ^attackMove = gcnew AntMove(antTile, attackTile);
		if (largestNumTurns != 2) {		
			//} else {
			if (ant->turnsToEnemy == 1) {	//Need to back off and wait for friendlies to be synced.
				AntMove ^retreatMove = attackMove->getOpposite();
				if (isValidMove(retreatMove, true, true)) {
					ant->setMove(retreatMove);
				} else {
					//No backwards retreat route found.					
				}
			}// else if(ant->turnsToEnemy > 2) {}
		} else {	//We have synced all ants within 2 turns.
			ant->setMove(attackMove);
		}
	}
}

void AI::distributeHarvesters() {
	List<MapTile^> ^visitedTiles = gcnew List<MapTile^>();
	Queue<MapTile^> ^queue = gcnew Queue<MapTile^>(0); //Queue of tiles to check
	for each(Ant ^ant in gameState->myHarvesters) {
		if (ant->mission) continue; //Already has mission.
		MapTile ^antTile = gameState->map->getTile(ant->position());
		antTile->distanceX = 0;
		antTile->distanceY = 0;
		queue->Enqueue(antTile);
		visitedTiles->Add(antTile);
		MapTile ^bestTile = antTile;
		do {
			MapTile ^currentTile = queue->Dequeue();
			if (!currentTile->visited && !currentTile->isWall) {
				int squaredDistance = getSquareDistance(currentTile->distanceX, currentTile->distanceY);
				//int squaredDistance2 = getSquareDistance(antTile->position, currentTile->position);
				if (squaredDistance <= MAX_VIEW_DISTANCE) {
					int harvesterValue = MAX_VIEW_DISTANCE - squaredDistance;
					currentTile->deltaHarvesterValue = currentTile->harvesterValue - harvesterValue; //Remove yourself from the value.
					if (currentTile->deltaHarvesterValue < bestTile->deltaHarvesterValue) {
						bestTile = currentTile;
					}
					currentTile->visited = true;

					//Add neighbours if they are not visited.
					tileCheck(currentTile->north, currentTile, visitedTiles, queue, currentTile->distanceX, currentTile->distanceY + 1, true);
					tileCheck(currentTile->south, currentTile, visitedTiles, queue, currentTile->distanceX, currentTile->distanceY + 1, true);
					tileCheck(currentTile->east, currentTile, visitedTiles, queue, currentTile->distanceX + 1, currentTile->distanceY, true);
					tileCheck(currentTile->west, currentTile, visitedTiles, queue, currentTile->distanceX + 1, currentTile->distanceY, true);

				} 
			}
		} while (queue->Count > 0);

		Mission ^mission = gcnew Mission(antTile, bestTile, MissionType::Explore);
		gameState->antMissions->Add(mission);
		MapTile ^previous = bestTile->previous;
		while (previous) {
			mission->path->Add(previous);
			if (mission->path->Count > MAX_PATH) {
				log->write("Path too long!");
				log->write("Tile: " + previous->position.ToString());
				break;
			}
			previous = previous->previous;
		}

		if (mission->path->Count > 1) {
			MapTile ^nextTile = mission->path[mission->path->Count - 2];
			AntMove ^newMove = gcnew AntMove(antTile, nextTile);
			//if (!antTargets->Contains(newMove) && newMove->move != Move::Idle) {
			if (isValidMove(newMove, false, false)) {
				antTargets->Add(newMove);
				ant->setMove(newMove);
				ant->mission = mission;
				//Move the ant from one tile to another 
				antTile->ant = nullptr;
				mission->currentTile = nextTile;
				nextTile->ant = ant;
			}
		}

		//Reset visited state before checking next ant.
		for each (MapTile ^visitedTile in visitedTiles) {
			visitedTile->visited = false;
			visitedTile->previous = nullptr;
			visitedTile->deltaHarvesterValue = 999;
		}
	}	
}

void AI::explore() {
	if (borderTiles->Count < 1) return;
	for each (Ant ^ant in gameState->myAnts) {
		if (ant->mission || ant->hasMoved()) continue;

		//Send ant to border tile.
		int threshold = 6;
		MapTile ^antTile = gameState->map->getTile(ant->position());
		MapTile ^bestCloseTile = borderTiles[0];
		int currentBestDistance = 9999;
		for each (MapTile ^tile in borderTiles) {
			if (tile->isBehindWall || tile->isWall || tile->explorerOnTheWay) continue;
			int squareDistance = getSquareDistance(ant->position(), tile->position);
			if (squareDistance < (currentBestDistance + threshold) && tile->exploreValue >= bestCloseTile->exploreValue) {
				currentBestDistance = squareDistance;
				bestCloseTile = tile;
			}
		}
		if (bestCloseTile) {
			bestCloseTile->explorerOnTheWay = true;
			Mission ^mission = gcnew Mission(antTile, bestCloseTile, MissionType::Explore);
			findPathMission(mission, true);
			ant->mission = mission;
			gameState->antMissions->Add(mission);
			if (mission->path->Count > 1) {
				//Path found
				int nextTileIndex = mission->path->Count - 2;
				MapTile ^nextTile = mission->path[nextTileIndex];
				AntMove ^newMove = gcnew AntMove(mission->currentTile, nextTile);
				if (isValidMove(newMove, false, false)) {
					antTargets->Add(newMove);
					ant->setMove(newMove);
					ant->mission = mission;
					//Move the ant from one tile to another 
					mission->currentTile->ant = nullptr;
					mission->currentTile = nextTile;
					mission->currentIndex = nextTileIndex;
					nextTile->ant = ant;
				}
			}
		}
	}
}

void AI::giveRandomMove() {
	for each (Ant ^ant in gameState->myAnts) {
		if (!ant->hasMoved()) {
			for (int i = 0; i < 4; i++) {
				Move move = ant->randomMove();
				AntMove ^currentMove = gcnew AntMove(gameState->map->getTile(ant->position()), move);
				//if (!currentMove->toTile->isWall && !currentMove->toTile->ant && !antTargets->Contains(currentMove)) {
				if (isValidMove(currentMove, true, true)) {
					antTargets->Add(currentMove);
					ant->setMove(currentMove);
					currentMove->fromTile->ant = nullptr; //Ant is beeing moved away.
					break;
				}
			}
		}
	}
}

void AI::removeMission(Ant ^ant) {
	if (ant) {
		Mission ^antMission = ant->mission;
		ant->mission = nullptr;
		if (antMission) {
			gameState->antMissions->Remove(antMission);
		}
	}
}

bool AI::isValidMove(AntMove ^move, bool checkAnt, bool checkWall) {
	bool result = !antTargets->Contains(move) && move->move != Move::Idle;
	if (result && checkAnt) {
		result = result && !move->toTile->ant;
	}
	if (result && checkWall) {
		result = result && !move->toTile->isWall;
	}
	return result;
}

String^ AI::getSpawnMode() {
	String ^spawnMode = ANT_STANDARD;
	int countEnemyAnts = gameState->enemyAnts->Count;
	if (gameState->roundCounter < 50 && countEnemyAnts < 2) {
		//Spawn harvester when in early game.
		spawnMode = ANT_HARVESTER;
	} else {
		int countStandard = gameState->myStandardAnts->Count;
		int countHarvester = gameState->myHarvesters->Count;
		int countSoldier = gameState->mySoldiers->Count;
		int totalAnts = gameState->myAnts->Count;

		if (countEnemyAnts > (countStandard + countSoldier) && countSoldier < countStandard) {
			spawnMode = ANT_SOLDIER;
		} else {
			double harvesterFraction = (double)countHarvester / (double)totalAnts;
			double soldierFraction = (double)countSoldier / (double)totalAnts;
			double limit = 1.0 / 3.0;
			if (soldierFraction < limit) {
				spawnMode = ANT_SOLDIER;
			} else if (harvesterFraction < limit) {
				spawnMode = ANT_HARVESTER;
			}
		}
	}
	return spawnMode;
}

