//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include <string>
#include <vector>

#include "mapstore.hpp"
#include "scores_table.hpp"

#include "engine/platform.hpp"
#include "engine/PFile.hpp"
#include "engine/PLang.hpp"
#include "engine/PZip.hpp"

#include "sfx.hpp"

#define EPISODI_MAX_LEVELS 100
//50

class LevelEntry{
public:
	std::string fileName;
	std::string levelName;

	int map_x = 0;
	int map_y = 0;

	int icon_id = 0;
	u32 number = 0;
	u8 status = 0;
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
		//u32 level_count = 0;

		bool glows = false;
		bool hide_numbers = false;
		bool ignore_collectable = false;
		bool require_all_levels = false;
		bool no_ending = false;
		std::string collectable_name = "big apple";
		bool transformation_offset = false;

		ScoresTable scoresTable;

		PLang infos;

		EpisodeClass(int save);
		EpisodeClass(const std::string& player_name, episode_entry entry);
		~EpisodeClass();

		void load();

		
		void loadAssets();

		void  saveScores();
		

		SfxHandler sfx;

		std::size_t getLevelsNumber()const{
			return this->levels_list_v.size();
		}

		u8 getLevelStatus(int level_id)const;
		void updateLevelStatus(int level_id, u8 status);

		std::string getLevelFilename(int level_id)const;
		int findLevelbyFilename(const std::string& levelFilename)const;

		const std::vector<LevelEntry>& getLevelEntries()const{
			return this->levels_list_v;
		}
		
		u32 getHighestLevelNumber()const{
			return this->highestLevelNumber;
		}

		bool isCompleted()const{
			return this->completed;
		}

		bool completed = false;
	private:
		void loadInfo();
		void loadLevels();
		u32 highestLevelNumber = 0;
		void updateNextLevel();

		std::vector<LevelEntry> levels_list_v;

		std::string getScoresPath()const;		
		void openScores();
};

extern EpisodeClass* Episode;
