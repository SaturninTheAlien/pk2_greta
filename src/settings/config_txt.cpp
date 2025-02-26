//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "config_txt.hpp"
#include "engine/PLang.hpp"
#include "engine/PFilesystem.hpp"
#include <iostream>

//TODO Remove this dependency
#include "system.hpp"

static const char default_config[] = 
"\r\n-- Audio Buffer Size"
"\r\n-- low value = low audio latency; high value = less cpu usage"
"\r\n-- Default is 1024"
"\r\n-- Prefer a power of 2: 512 1024 2048 4096 default"
"\r\n---------------"
"\r\n*audio_buffer_size:    default"
"\r\n"
"\r\n-- To determine how the links menu should look like"
"\r\n-- Available options are: main_menu, bottom, none"
"\r\n*links_menu:    main_menu"
"\r\n"
"\r\n"
"\r\n"
"\r\n-- To use legacy saving system with \"slots\""
"\r\n*use_save_slots:    yes"
"\r\n"
"\r\n"
"\r\n"
"\r\n-- Current player"
"\r\n-- Used to save the game progress if \"save slots\" are disabled."
"\r\n*player:	pekka"
"\r\n"
"\r\n"
"\r\n -- Silent suicide"
"\r\n -- If set to \"yes\" the player has no destruction effect"
"\r\n -- after committing suicide by pressing delete key"
"\r\n -- Some players requested it for \"psychological reasons\"."
"\r\n -- disabled by default"
"\r\n*silent_suicide:  no"
"\r\n";

void Config_txt::readFile(){
    PLang conf = PLang();
	PFile::Path path = PFilesystem::GetDataFileW("config.txt");
	
	bool ok = conf.Read_File(path);
	if (!ok) {
		
		PFile::RW rw = path.GetRW2("w");
		rw.write(default_config, sizeof(default_config) - 1);
		rw.close();
		return;
	}


	int idx = conf.Search_Id("links_menu");
	if(idx!=-1){
		std::string val = conf.getString("links_menu", "default");
		if(val=="default"){
			this->links_menu = LINKS_MENU_MAIN;
		}
		else if(val=="main_menu"){
			this->links_menu = LINKS_MENU_MAIN;
		}
		else if(val=="bottom"){
			this->links_menu = LINKS_MENU_BOTTOM;
		}
		else if(val=="none"){
			this->links_menu = LINKS_MENU_NONE;
		}
	}
	
	this->audio_buffer_size = conf.getInteger("audio_buffer_size", 1024);
	this->save_slots = conf.getBoolean("use_save_slots", true);
	this->player = conf.getString("player", "Pekka");
	this->silent_suicide = conf.getBoolean("silent_suicide", false);
}


Config_txt config_txt;