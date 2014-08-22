#include "GameStateRenderer.h"

#include "Ant.h"
#include "MapTile.h"
#include "Mission.h"

GameStateRenderer::GameStateRenderer() {
	canvas = nullptr;
	log = _GUIlog::getInstance();
	mapSize = Point(0, 0);
	overLayBrush = gcnew SolidBrush(Color::FromArgb(40, 255, 255, 255));
	overLayBrushGreen = gcnew SolidBrush(Color::FromArgb(40, 128, 255, 128));
	overLayBrushRed = gcnew SolidBrush(Color::FromArgb(40, 255, 128, 128));
	drawFont = gcnew System::Drawing::Font("Arial", 6);
}

GameStateRenderer ^ GameStateRenderer::getInstance() {
	if (!singleton) {
		singleton = gcnew GameStateRenderer();
		return singleton;
	} else {
		return singleton;
	}
}

void GameStateRenderer::render(GameState ^gameState) {
	if (mapSize.X <= 0 || mapSize.Y <= 0) {
		mapSize = gameState->map->mapSize();
		log->write("Error with mapsize, canceling render.");
		return;
	}
	int tileSize = canvas->Size.Width / mapSize.X;
	int tileSizeY = canvas->Size.Height / mapSize.Y;
	if (tileSizeY < tileSize) {
		//Use the lowest available tileSize
		tileSize = tileSizeY;
	}

	Bitmap ^buffer = gcnew Bitmap(canvas->Size.Width, canvas->Size.Height);
	Graphics ^painter = Graphics::FromImage(buffer);
	painter->Clear(Color::Black);

	for (int x = 0; x < mapSize.X; ++x) {
		for (int y = 0; y < mapSize.Y; ++y) {
			MapTile ^currentTile = gameState->map->getTile(x, y);
			if (currentTile) {
				if (currentTile->isWall) {
					painter->FillRectangle(Brushes::Gray, x * tileSize, y * tileSize, tileSize, tileSizeY);
				} else if (currentTile->hasFood) {
					painter->FillRectangle(Brushes::Brown, x * tileSize, y * tileSize, tileSize, tileSizeY);
				}
				if (currentTile->isVisible) {
					painter->FillRectangle(overLayBrush, x * tileSize, y * tileSize, tileSize, tileSizeY);
					//painter->DrawString("" + currentTile->exploreValue, drawFont, Brushes::White, (float)tileSize * x + 2, (float)tileSize * y + 2 * 3);
				}
				if (currentTile->isBorderTile) {
					painter->FillRectangle(Brushes::LightSteelBlue, x * tileSize, y * tileSize, tileSize, tileSizeY);
				}
				/*if (currentTile->isBehindWall) {
					painter->FillRectangle(Brushes::LightSalmon, x * tileSize, y * tileSize, tileSize, tileSizeY);
				}*/
			}
		}
	}

	for each (MapTile ^spawner in gameState->mySpawners) {
		painter->DrawRectangle(Pens::AliceBlue, spawner->position.X * tileSize, spawner->position.Y * tileSize, tileSize, tileSize);
		if (spawner->isSpawnerDestroyed) {
			painter->DrawLine(Pens::AliceBlue, spawner->position.X * tileSize, spawner->position.Y * tileSize, spawner->position.X * tileSize + tileSize, spawner->position.Y * tileSize);
			painter->DrawLine(Pens::AliceBlue, spawner->position.X * tileSize, spawner->position.Y * tileSize + tileSize, spawner->position.X * tileSize + tileSize, spawner->position.Y * tileSize + tileSize);
		}
	}

	for each (MapTile ^spawner in gameState->enemySpawners) {
		painter->DrawRectangle(Pens::Red, spawner->position.X * tileSize, spawner->position.Y * tileSize, tileSize, tileSize);
		if (spawner->isSpawnerDestroyed) {
			painter->DrawLine(Pens::Red, spawner->position.X * tileSize, spawner->position.Y * tileSize, spawner->position.X * tileSize + tileSize, spawner->position.Y * tileSize + tileSize);
			painter->DrawLine(Pens::Red, spawner->position.X * tileSize, spawner->position.Y * tileSize + tileSize, spawner->position.X * tileSize + tileSize, spawner->position.Y * tileSize);
		}
	}

	for each (Ant ^ant in gameState->myAnts) {
		painter->FillEllipse(Brushes::AliceBlue, ant->position().X * tileSize, ant->position().Y * tileSize, tileSize, tileSize);
	}

	for each (Ant ^ant in gameState->myHarvesters) {
		painter->DrawLine(Pens::Black, ant->position().X * tileSize + tileSize / 2, ant->position().Y * tileSize, ant->position().X * tileSize + tileSize / 2, ant->position().Y * tileSize + tileSize);
	}

	for each (Ant ^ant in gameState->mySoldiers) {
		painter->DrawLine(Pens::Black, ant->position().X * tileSize, ant->position().Y * tileSize + tileSize / 2, ant->position().X * tileSize + tileSize, ant->position().Y * tileSize + tileSize / 2);
	}

	for each (Ant ^ant in gameState->enemyAnts) {
		painter->FillEllipse(Brushes::Red, ant->position().X * tileSize, ant->position().Y * tileSize, tileSize, tileSize);
		if (ant->type() == AntType::Harvester) {
			painter->DrawLine(Pens::Black, ant->position().X * tileSize + tileSize / 2, ant->position().Y * tileSize, ant->position().X * tileSize + tileSize / 2, ant->position().Y * tileSize + tileSize);
		} else  if (ant->type() == AntType::Soldier) {
			painter->DrawLine(Pens::Black, ant->position().X * tileSize, ant->position().Y * tileSize + tileSize / 2, ant->position().X * tileSize + tileSize, ant->position().Y * tileSize + tileSize / 2);
		}
	}

	for each (Mission ^mission in gameState->antMissions) {
		if (mission->path->Count > 0 && (mission->type == MissionType::Attack || mission->type == MissionType::Reinforce) && mission->currentTile->ant && mission->currentTile->ant->type() != AntType::Harvester) {			
			for each (MapTile ^tile in mission->path) {
				painter->FillRectangle(overLayBrushGreen, tile->position.X * tileSize, tile->position.Y * tileSize, tileSize, tileSizeY);
			}

			/*if (mission->visitedTiles) {
				for each (MapTile ^tile in mission->visitedTiles) {
					painter->FillRectangle(overLayBrushRed, tile->position.X * tileSize, tile->position.Y * tileSize, tileSize, tileSizeY);
				}
			}*/
		}
	}

	painter->Save();
	canvas->Image = buffer;
}