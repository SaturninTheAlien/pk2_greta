//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/PLang.hpp"

#include <vector>
#include <string>
#include <array>

extern PLang *tekstit;

const uint MAX_INFOS = 100;

class LANGUAGE {
public:
	LANGUAGE(){
		this->infos.fill(-1);
	}

	int setup_options = -1,
		setup_videomodes = -1,
		setup_music_and_sounds = -1,
		setup_music = -1,
		setup_sounds = -1,
		setup_language = -1,
		setup_play = -1,
		setup_exit = -1,

		intro_presents = -1,
		intro_a_game_by = -1,
		intro_original = -1,
		intro_tested_by = -1,
		intro_thanks_to = -1,
		intro_translation = -1,
		intro_translator = -1,

		mainmenu_new_game = -1,
		mainmenu_continue = -1,
		mainmenu_load_game = -1,
		mainmenu_save_game = -1,
		mainmenu_controls = -1,
		mainmenu_graphics = -1,
		mainmenu_sounds = -1,
		mainmenu_exit = -1,

		mainmenu_return = -1,

		loadgame_title = -1,
		loadgame_info = -1,
		loadgame_episode = -1,
		loadgame_level = -1,

		savegame_title = -1,
		savegame_info = -1,
		savegame_episode = -1,
		savegame_level = -1,

		controls_title = -1,
		controls_moveleft = -1,
		controls_moveright = -1,
		controls_jump = -1,
		controls_duck = -1,
		controls_walkslow = -1,
		controls_eggattack = -1,
		controls_doodleattack = -1,
		controls_useitem = -1,
		controls_edit = -1,
		controls_kbdef = -1,
		controls_gp4def = -1,
		controls_gp6def = -1,

		gfx_title = -1,
		
		// gfx_tfx_on = -1,
		// gfx_tfx_off = -1,

		gfx_tmenus_on = -1,
		gfx_tmenus_off = -1,
		
		gfx_items_on = -1,
		gfx_items_off = -1,

		// gfx_weather_on = -1,
		// gfx_weather_off = -1,

		// gfx_bgsprites_on = -1,
		// gfx_bgsprites_off = -1,

		gfx_speed_normal = -1,
		gfx_speed_double = -1,

		sound_title = -1,
		sound_sfx_volume = -1,
		sound_music_volume = -1,
		sound_more = -1,
		sound_less = -1,

		playermenu_type_name = -1,
		playermenu_continue = -1,
		playermenu_clear = -1,
		player_default_name = -1,

		episodes_choose_episode = -1,
		episodes_no_maps = -1,
		episodes_get_more = -1,

		map_total_score = -1,
		map_next_level = -1,
		map_episode_best_player = -1,
		map_episode_hiscore = -1,
		map_level_best_player = -1,
		map_level_hiscore = -1,
		map_level_fastest_player = -1,
		map_level_best_time = -1,

		score_screen_title = -1,
		score_screen_level_score = -1,
		score_screen_bonus_score = -1,
		score_screen_time_score = -1,
		score_screen_energy_score = -1,
		score_screen_item_score = -1,
		score_screen_total_score = -1,
		score_screen_new_level_hiscore = -1,
		score_screen_new_level_best_time = -1,
		score_screen_new_episode_hiscore = -1,
		score_screen_continue = -1,

		game_score = -1,
		game_time = -1,
		game_energy = -1,
		game_items = -1,
		game_attack1 = -1,
		game_attack2 = -1,
		game_keys = -1,
		game_clear = -1,
		game_timebonus = -1,
		game_ko = -1,
		game_timeout = -1,
		game_tryagain = -1,
		game_locksopen = -1,
		game_newdoodle = -1,
		game_newegg = -1,
		game_newitem = -1,
		game_loading = -1,
		game_paused = -1,

		end_congratulations = -1,
		end_chickens_saved = -1,
		end_the_end = -1;

		std::array<int, MAX_INFOS> infos;
		//infos[MAX_INFOS];
	
};

extern LANGUAGE PK_txt;

extern std::vector<std::string> langlist;

const char* GetDefaultLanguageName();
int Load_Language(const std::string& language);