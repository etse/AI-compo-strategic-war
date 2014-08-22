#pragma once

#include "GameState.h"
#include "../include/gui_class.h"

using namespace System::Windows::Forms;

public ref class GameStateRenderer {
protected:
	_GUIlog ^log;
	static GameStateRenderer ^singleton;
	GameStateRenderer();
	Point mapSize;
	SolidBrush^ overLayBrush;
	SolidBrush^ overLayBrushGreen;
	SolidBrush^ overLayBrushRed;
	Font ^drawFont;

	delegate void RenderEvent(GameState ^gameState);
	
public:
	PictureBox ^canvas;
	static GameStateRenderer ^getInstance();
	void render(GameState ^gameState);


};

