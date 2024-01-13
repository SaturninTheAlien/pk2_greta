//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/types.hpp"
#include "engine/PFile.hpp"
#include "screen.hpp"
#include <vector>
#include <map>

class ScreensHandler{
public:
	ScreensHandler();
	~ScreensHandler();
	void Loop();
private:
	Screen* current_screen = nullptr;
	int current_screen_index = SCREEN_NOT_SET;
	int mFullScreenKeyDelay = 0;

	std::map<int, Screen*> screens_map;
};



/*extern int current_screen;
extern int next_screen;

extern uint chosen_menu_id;
extern uint selected_menu_id;*/

/*void Fade_Quit();

void Screen_Intro_Init();
void Screen_Menu_Init();
void Screen_Map_Init();
void Screen_InGame_Init();
void Screen_ScoreCount_Init();
void Screen_Ending_Init();
void Screen_LevelError_Init();

void Screen_Intro();
void Screen_Menu();
void Screen_Map();
void Screen_InGame();
void Screen_ScoreCount();
void Screen_Ending();
void Screen_LevelError();*/


//void Screen_First_Start();
//void Screen_Loop();