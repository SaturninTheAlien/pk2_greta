//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

//TODO
//Move it to Mapstore or EpisodeClass

#include "episode/episodeclass.hpp"

#include "engine/platform.hpp"

#define SAVE_SLOTS_NUMBER 11

namespace PK2save{
// Save struct is not the ideal way because of byte order
struct PK2SAVE {

	bool  empty;
	u32   next_level;
	char  episode[128];
	char  name[20];
	u32   score;
	u8    level_status[EPISODI_MAX_LEVELS];
	
};

extern PK2SAVE saves_slots[SAVE_SLOTS_NUMBER];

void LoadSaveSlots();
int Save_Record(int i);

}