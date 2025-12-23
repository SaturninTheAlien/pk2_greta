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
