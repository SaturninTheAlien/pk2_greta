//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

//TODO
//Move it to Mapstore or EpisodeClass

#include "episode/episodeclass.hpp"

#include "engine/platform.hpp"

const int SAVES_COUNT = 11;

// Save struct is not the ideal way because of byte order
struct PK2SAVE {

	bool  empty;
	u32   next_level;
	char  episode[128];
	char  name[20];
	u32   score;
	u8    level_status[EPISODI_MAX_LEVELS];
	
};

extern PK2SAVE saves_list[SAVES_COUNT];

void Load_SaveFile();
int Save_Record(int i);