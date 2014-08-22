#include "GameState.h"
#include "../include/datatypes.h"

GameState::GameState() {
	myPlayerId = -1;
	numPlayers = -1;
	roundCounter = 0;
	Point mapSize = Point(0);
	map = gcnew Map();
	myAnts = gcnew List<Ant^>();
	myStandardAnts = gcnew List<Ant^>();
	myHarvesters = gcnew List<Ant^>();
	mySoldiers = gcnew List<Ant^>();
	enemyAnts = gcnew List<Ant^>();
	mySpawners = gcnew List<MapTile^>();
	enemySpawners = gcnew List<MapTile^>();
	food = gcnew List<MapTile^>();
	antMissions = gcnew List<Mission^>();
	log = _GUIlog::getInstance();
}

bool GameState::process(String ^data) {
	if (data->Contains("\"status\": ")) {
		int colonIndex = data->IndexOf(":") + 1;
		int endIndex = data->IndexOf("}", colonIndex);
		String ^status = data->Substring(colonIndex, endIndex - colonIndex);
		if (status->Trim() != "\"OK\"") {
			log->write("Server is not OK: " + status, Color::Red);
		}
		return false;
	} else {
		++roundCounter;
		for each (MapTile ^tile in food) {
			tile->hasFood = false;
		}
		food->Clear();
		myAnts->Clear();
		myStandardAnts->Clear();
		myHarvesters->Clear();
		mySoldiers->Clear();
		enemyAnts->Clear();
		map->reset();
		int mapStart = data->IndexOf("\"map\": [");
		int mapEnd = getBracketEnd('[', ']', data, mapStart + 8) - 1;
		//int mapEnd = data->IndexOf("}],");
		String ^mapData = data->Substring(mapStart + 1, mapEnd - mapStart);
		int mapSizeStart = data->IndexOf("\"map_size\": [");
		int mapSizeEnd = getBracketEnd('[', ']', data, mapSizeStart + 13) + 1;
		String ^mapSize = data->Substring(mapSizeStart, mapSizeEnd - mapSizeStart);
		int numPlayerStart = data->IndexOf("\"num_players\": ");
		int numPlayerEnd = data->IndexOf(",", numPlayerStart);
		String ^numPlayers = data->Substring(numPlayerStart, numPlayerEnd - numPlayerStart);
		String ^playerId = data->Substring(0, numPlayerStart);
		getPlayerId(playerId->Trim());
		getMapSize(mapSize->Trim());
		getNumPlayers(numPlayers);
		updateMapData(mapData);
		return true;
	}
	/*int firstComma = data->IndexOf(',');
	String ^playerId = data->Substring(1, firstComma);
	if (playerId->StartsWith("\"player_id\":")) {
		myPlayerId = datatypes::String2int(playerId->Substring(12, firstComma - 12));
	}*/


}

void GameState::updateMapData(String ^mapDataStr) {
	do {
		int firstBracketStart = mapDataStr->IndexOf('{');
		if (firstBracketStart < 0) {
			break;
		}
		firstBracketStart++;
		int firstBracketEnd = getBracketEnd('{', '}', mapDataStr, firstBracketStart);
		if (firstBracketEnd < 0) {
			break;
		}
		String ^line = mapDataStr->Substring(firstBracketStart, firstBracketEnd - firstBracketStart);
		parseMapLine(line);
		if (mapDataStr->Length > firstBracketEnd) {
			mapDataStr = mapDataStr->Remove(0, firstBracketEnd);
		} else {
			break;
		}
	} while (mapDataStr->Length > 0);
}

void GameState::parseMapLine(String ^line) {
	int positionIndex = line->IndexOf("\"position\":");
	Point currentPosition = Point(0, 0);
	if (positionIndex >= 0) {
		int bracketStart = line->IndexOf("[", positionIndex);
		int bracketEnd = line->IndexOf("]", bracketStart);
		if (bracketStart > 0 && bracketEnd > 0) {
			bracketStart++;
			String ^sizeStr = line->Substring(bracketStart, bracketEnd - bracketStart);
			int valX = datatypes::String2int(sizeStr->Split(',')[0]);
			int valY = datatypes::String2int(sizeStr->Split(',')[1]);
			if (valX >= 0 && valX < map->width() && valY >= 0 && valY < map->height()) {
				currentPosition = Point(valX, valY);
			}
		}
	}

	MapTile ^currentTile = map->getTile(currentPosition);
	int wallIndex = line->IndexOf("\"is_wall\":");
	if (wallIndex >= 0) {
		int colonIndex = line->IndexOf(":", wallIndex);
		int commaIndex = line->IndexOf(",", colonIndex);
		if (colonIndex >= 0 && commaIndex >= 0) {
			colonIndex++;
			String ^wallStr = line->Substring(colonIndex, commaIndex - colonIndex);
			currentTile->isWall = datatypes::s2b(wallStr);
		}
	}

	
	//{"position":[19, 24], "has_food" : true}],
	int foodIndex = line->IndexOf("\"has_food\":");
	if (foodIndex >= 0) {
		int colonIndex = line->IndexOf(":", foodIndex);
		if (colonIndex > 0) {
			colonIndex++;
			String ^foodStr = line->Substring(colonIndex, line->Length - colonIndex);
			currentTile->hasFood = datatypes::s2b(foodStr);
			if (currentTile->hasFood) {
				food->Add(currentTile);
			}
		}
	}

	int spawnerIndex = line->IndexOf("\"spawner\":");
	if (spawnerIndex >= 0) {
		int bracketStart = line->IndexOf("{", spawnerIndex);
		int bracketEnd = line->IndexOf("}", bracketStart);
		if (bracketStart > 0 && bracketEnd > 0) {
			bracketStart++;
			String ^dataStr = line->Substring(bracketStart, bracketEnd - bracketStart);
			int ownerIndex = dataStr->IndexOf("\"owner\":");
			if (ownerIndex >= 0) {
				int colonIndex = dataStr->IndexOf(":", ownerIndex);
				int commaIndex = dataStr->IndexOf(",", colonIndex);
				if (colonIndex >= 0 && commaIndex >= 0) {
					colonIndex++;
					String ^ownerIdStr = dataStr->Substring(colonIndex, commaIndex - colonIndex);
					currentTile->spawnerOwner = datatypes::String2int(ownerIdStr);										
				}
			}

			int destroyedIndex = dataStr->IndexOf("\"destroyed\":");
			if (destroyedIndex >= 0) {
				int colonIndex = dataStr->IndexOf(":", destroyedIndex);
				if (colonIndex >= 0) {
					colonIndex++;
					String ^destroyedStr = dataStr->Substring(colonIndex, dataStr->Length - colonIndex);
					destroyedStr = destroyedStr->Replace("\"", "");
					currentTile->isSpawnerDestroyed = datatypes::s2b(destroyedStr);
				}
			}
			addSpawner(currentTile);
		}
	}

	int unitIndex = line->IndexOf("\"unit\":");
	if (unitIndex >= 0) {
		int bracketStart = line->IndexOf("{", unitIndex);
		int bracketEnd = line->IndexOf("}", bracketStart);
		if (bracketStart > 0 && bracketEnd > 0) {
			Ant ^newAnt = gcnew Ant();
			newAnt->setPosition(currentPosition);
			bracketStart++;
			String ^dataStr = line->Substring(bracketStart, bracketEnd - bracketStart);
			int ownerIndex = dataStr->IndexOf("\"owner\":");
			if (ownerIndex >= 0) {
				int colonIndex = dataStr->IndexOf(":", ownerIndex);
				int commaIndex = dataStr->IndexOf(",", colonIndex);
				if (colonIndex >= 0 && commaIndex >= 0) {
					colonIndex++;
					String ^ownerIdStr = dataStr->Substring(colonIndex, commaIndex - colonIndex);
					newAnt->setOwner(datatypes::String2int(ownerIdStr));
				}
			}

			int typeIndex = dataStr->IndexOf("\"type\":");
			if (typeIndex >= 0) {
				int colonIndex = dataStr->IndexOf(":", typeIndex);
				if (colonIndex >= 0) {
					colonIndex++;
					String ^typeStr = dataStr->Substring(colonIndex, dataStr->Length - colonIndex);
					newAnt->setType(typeStr);
				}
			}
			currentTile->ant = newAnt;
			addAnt(newAnt);
		}
	}
}

void GameState::addSpawner(MapTile ^newSpawner) {
	if (newSpawner) {
		if (newSpawner->spawnerOwner == myPlayerId) {
			if (!mySpawners->Contains(newSpawner)) {
				mySpawners->Add(newSpawner);
			}
		} else if (!enemySpawners->Contains(newSpawner)) {
			enemySpawners->Add(newSpawner);
		}
	}
}

void GameState::addAnt(Ant ^newAnt) {
	if (newAnt) {
		if (newAnt->owner() == myPlayerId) {
			if (newAnt->type() == AntType::Harvester && !myHarvesters->Contains(newAnt)) {
				myHarvesters->Add(newAnt);
			} else if (newAnt->type() == AntType::Soldier && !mySoldiers->Contains(newAnt)) {
				mySoldiers->Add(newAnt);
			} else if (!myAnts->Contains(newAnt)) {
				myStandardAnts->Add(newAnt);
			} 
			if (!myAnts->Contains(newAnt)) {
				myAnts->Add(newAnt);
			}
		} else if (!enemyAnts->Contains(newAnt)) {
			enemyAnts->Add(newAnt);
		}
	}
}

void GameState::getPlayerId(String ^playerIdStr) {
	int playerIdIndex = playerIdStr->IndexOf("\"player_id\":");
	if (playerIdIndex >= 0) {
		int colonIndex = playerIdStr->IndexOf(":", playerIdIndex);
		int commaIndex = playerIdStr->IndexOf(",", colonIndex);
		if (colonIndex >= 0 && commaIndex >= 0) {
			colonIndex++;
			String ^idStr = playerIdStr->Substring(colonIndex, commaIndex - colonIndex);
			myPlayerId = datatypes::String2int(idStr);
		}
	}
}

void GameState::getMapSize(String ^mapSizeStr) {
	int mapSizeIndex = mapSizeStr->IndexOf("\"map_size\":");
	if (mapSizeIndex >= 0) {
		int bracketStart = mapSizeStr->IndexOf("[", mapSizeIndex);
		int bracketEnd = mapSizeStr->IndexOf("]", bracketStart);
		if (bracketStart > 0 && bracketEnd > 0) {
			bracketStart++;
			String ^sizeStr = mapSizeStr->Substring(bracketStart, bracketEnd - bracketStart);
			int width = datatypes::String2int(sizeStr->Split(',')[0]);
			int height = datatypes::String2int(sizeStr->Split(',')[1]);
			map->setSize(width, height);
		}
	}
}

void GameState::getNumPlayers(String ^numPlayersStr) {
	int numPlayersId = numPlayersStr->IndexOf("\"num_players\":");
	if (numPlayersId >= 0) {
		int colonIndex = numPlayersStr->IndexOf(":", numPlayersId);
		if (colonIndex >= 0) {
			colonIndex++;
			String ^idStr = numPlayersStr->Substring(colonIndex, numPlayersStr->Length - colonIndex);
			numPlayers = datatypes::String2int(idStr);
		}
	}
}

int GameState::getBracketEnd(Char bracketStart, Char bracketEnd, String ^str, int startIndex) {
	int counter = 0;
	for (int i = startIndex; i < str->Length; ++i) {
		Char currentChar = str[i];
		if (currentChar == bracketStart) {
			counter++;
		}
		if (currentChar == bracketEnd) {
			counter--;
		}
		if (counter < 0) {
			return i;
		}
	}
	return -1;
}



