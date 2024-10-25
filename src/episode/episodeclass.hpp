//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "mapstore.hpp"
#include "scores_table.hpp"

#include "engine/platform.hpp"
#include "engine/PFile.hpp"
#include "engine/PLang.hpp"
#include "engine/PZip.hpp"

#include "sfx.hpp"

const int EPISODI_MAX_LEVELS = 100; //50;

struct PK2LEVEL {

	char  tiedosto[PE_PATH_SIZE];
	char  nimi[40];
	int   x, y;
	u32   order;
	u32   icon;
	
};

enum LEVEL_STATUS {

	LEVEL_PASSED    = 0b001,
	LEVEL_ALLAPPLES = 0b010,
	LEVEL_HAS_BIG_APPLES = 0b100
};

class EpisodeClass {
	public:	
		episode_entry entry;
		PZip::PZip source_zip;

		std::string player_name;
		int player_score = 0;

		u32 next_level = 1;
		u32 level_count = 0;

		bool glows = false;
		bool hide_numbers = false;
		bool ignore_collectable = false;
		bool require_all_levels = false;
		bool no_ending = false;
		std::string collectable_name = "big apple";
		bool transformation_offset = false;

		PK2LEVEL levels_list[EPISODI_MAX_LEVELS];
		u8 level_status[EPISODI_MAX_LEVELS];

		ScoresTable scoresTable;

		PLang infos;

		EpisodeClass(int save);
		EpisodeClass(const std::string& player_name, episode_entry entry);
		~EpisodeClass();

		void Load();

		void Load_Info();
		void Load_Assets();

		void  saveScores();
		void Update_NextLevel();

		SfxHandler sfx;
	private:

		std::string getScoresPath()const;		
		void openScores();
};

extern EpisodeClass* Episode;
