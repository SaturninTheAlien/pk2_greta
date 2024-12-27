//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include <string>

class Config_txt{
public:

    void readFile();

    bool save_slots = true;
    bool silent_suicide = false;
    std::string player = "pekka";
	int audio_buffer_size = 1024;
};

extern Config_txt config_txt;
