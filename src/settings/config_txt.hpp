//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include <string>

enum{
    LINKS_MENU_NONE,
    LINKS_MENU_MAIN,
    LINKS_MENU_BOTTOM
};

class Config_txt{
public:

    void readFile();

    int links_menu = LINKS_MENU_MAIN;
    bool save_slots = true;
    bool silent_suicide = false;
    std::string player = "pekka";
	int audio_buffer_size = 1024;
    bool panic_when_missing_assets = true;
};

extern Config_txt config_txt;
