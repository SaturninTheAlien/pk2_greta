//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/types.hpp"
#include "engine/PFile.hpp"

enum SCREEN_TYPE {
	SCREEN_NOT_SET,
	SCREEN_FIRST_START,
	SCREEN_INTRO,
	SCREEN_MENU,
	SCREEN_MAP,
	SCREEN_GAME,
	SCREEN_SCORING,
	SCREEN_END,

	SCREEN_LEVEL_ERROR,
};

class Screen{
public:
	bool keys_move = false;

	static unsigned int chosen_menu_id;
	static unsigned int selected_menu_id;
	
	static std::size_t next_screen;
	virtual ~Screen(){}
	virtual void Init()=0;
	virtual void Loop()=0;
	static bool closing_game;
protected:
	static bool Draw_Menu_Text(const char *teksti, int x, int y, char end='\0');
	static void Fade_Quit();	
};