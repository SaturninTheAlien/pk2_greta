//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//-------------------------
//It can be started with the "dev" argument to start the
//cheats and "test" follown by the episode and level to
//open directely on the level.
//	Exemple:
//	"./pekka-kana-2 dev test rooster\ island\ 2/level13.map"
//	Starts the level13.map on dev mode
//#########################
#include "pk2_main.hpp"

#include "engine/Piste.hpp"
#include "version.hpp"

#include "screens/screens_handler.hpp"
#include "gfx/text.hpp"
#include "game/game.hpp"
#include "episode/episodeclass.hpp"
#include "episode/mapstore.hpp"
#include "save.hpp"
#include "gui.hpp"
#include "system.hpp"
#include "language.hpp"
#include "settings.hpp"
#include <filesystem>
#include <algorithm>

#include "game/prototypes_handler.hpp"

#include <cstring>
#include <algorithm>

#include <SDL.h>

static const char default_config[] = 
"\r\n -- Silent suicide"
"\r\n -- If set to \"yes\" the player has no destruction effect"
"\r\n -- after commiting suicide by pressing delete key"
"\r\n -- Some players requested it for \"psychological reasons\"."
"\r\n -- disabled by default"
"\r\n*silent_suicide:  no"
"\r\n"
"\r\n"
"\r\n-- Audio Buffer Size"
"\r\n-- low value = low audio latency; high value = less cpu usage"
"\r\n-- Default is 1024"
"\r\n-- Prefer a power of 2: 512 1024 2048 4096 default"
"\r\n---------------"
"\r\n*audio_buffer_size:    default"
"\r\n"
"\r\n"
"\r\n-- Multi thread audio"
"\r\n-- Probably causes \"Segmentation Fault\", don't turn it on"
"\r\n-- Change frequency in another thread"
"\r\n-- Default is no"
"\r\n---------------"
"\r\n*audio_multi_thread_x:    no"
"\r\n"
"\r\n"
"\r\n-- For the compatibility with some older episodes"
"\r\n-- Don't turn it on without a good reason!"
"\r\n-- On the legacy PK2, the player used to move a bit upwards after using a transformation potion."
"\r\n-- If you are a mapmaker, please don't use it intentionally."
"\r\n--"
"\r\n-- This feature may be removed completely in the future."
"\r\n---------------"
"\r\n*potion_transformation_offset:    no"
"\r\n"
"\r\n"
"\r\n";


static void read_config() {

	PLang conf = PLang();
	PFile::Path path = PFile::Path(data_path + "config.txt");
	
	bool ok = conf.Read_File(path);
	if (!ok) {
		
		PFile::RW rw = path.GetRW2("w");
		rw.write(default_config, sizeof(default_config) - 1);
		rw.close();
		return;
	}

	PLog::Write(PLog::DEBUG, "PK2", "Found config file");

	int idx = conf.Search_Id("audio_buffer_size");
	if (idx != -1) {
		const char* txt = conf.Get_Text(idx);
		int val = atoi(txt);

		if (val > 0) {
			configuration.audio_buffer_size = val;
			

		}
	}
	PLog::Write(PLog::DEBUG, "PK2", "Audio buffer size set to %i", configuration.audio_buffer_size);

	idx = conf.Search_Id("audio_multi_thread_x");
	if (idx != -1) {
		const char* txt = conf.Get_Text(idx);

		if (strcmp(txt, "default") == 0)
			configuration.audio_multi_thread = false;
		else if (strcmp(txt, "yes") == 0)
			configuration.audio_multi_thread = true;
		else if (strcmp(txt, "no") == 0)
			configuration.audio_multi_thread = false;

		
	}
	idx = conf.Search_Id("potion_transformation_offset");
	if (idx != -1) {
		const char* txt = conf.Get_Text(idx);

		if (strcmp(txt, "default") == 0)
			configuration.transformation_offset = false;
		else if (strcmp(txt, "yes") == 0)
			configuration.transformation_offset = true;
		else if (strcmp(txt, "no") == 0)
			configuration.transformation_offset = false;	
	}
	idx = conf.Search_Id("silent_suicide");
	if(idx != -1){
		const char* txt = conf.Get_Text(idx);

		if (strcmp(txt, "default") == 0)
			configuration.silent_suicide = false;
		else if (strcmp(txt, "yes") == 0)
			configuration.silent_suicide = true;
		else if (strcmp(txt, "no") == 0)
			configuration.silent_suicide = false;	
	}

	PLog::Write(PLog::DEBUG, "PK2", "Audio multi thread is %s", configuration.audio_multi_thread? "ON" : "OFF");

}

static void start_test(const char* arg) {
	
	if (arg == NULL) return;

	PFile::Path path(arg);

	episode_entry episode;
	episode.name = path.GetDirectory();
	episode.is_zip = false;
	Episode = new EpisodeClass("test", episode);

	Game = new GameClass(path.GetFileName());

	PLog::Write(PLog::DEBUG, "PK2", "Testing episode '%s' level '%s'", episode.name.c_str(), Game->map_file.c_str());

}

// Clean memory (needed?)
static void quit() {

	//Settings_Save();

	PSound::stop_music();

	if (PUtils::Is_Mobile())
		GUI_Exit();

	if (Game!=nullptr){
		delete Game;
		Game = nullptr;
	}
	
	if (Episode!=nullptr){
		delete Episode;
		Episode = nullptr;
	}
	
	if(tekstit!=nullptr){
		delete tekstit;
		tekstit = nullptr;
	}

	Piste::terminate();
	PLog::Write(PLog::DEBUG, "PK2", "Terminated");
	PLog::Exit();

}


static void set_paths() {

	PFile::SetDefaultAssetsPath();
	
	#ifndef __ANDROID__

	#ifdef PK2_PORTABLE

	data_path = "data" PE_SEP;
	PFile::CreateDirectory(data_path);

	#else

	char* data_path_p = SDL_GetPrefPath(NULL, PK2_NAME);
	if (data_path_p == NULL) {

		PLog::Write(PLog::FATAL, "PK2", "Failed to init data path");
		quit();
		//return 1;

	}

	data_path = data_path_p;
	SDL_free(data_path_p);

	#endif //PK2_PORTABLE

	#else //__ANDROID__

	const char* ptr = SDL_AndroidGetExternalStoragePath(); //TODO 1.5 - external path must be a writeable path
	if (ptr) {
		External_Path = ptr;
		SDL_free((void*)ptr);
	} else {
		PLog::Write(PLog::ERR, "PK2", "Couldn't find External Path");
	}

	ptr = SDL_AndroidGetInternalStoragePath();
	if (ptr) {
		Internal_Path = ptr;
		SDL_free((void*)ptr);
	} else {
		PLog::Write(PLog::ERR, "PK2", "Couldn't find Internal Path");
	}

	External_Path += PE_SEP;
	Internal_Path += PE_SEP;

	PLog::Write(PLog::DEBUG, "PK2", "External %s", External_Path.c_str());
	PLog::Write(PLog::DEBUG, "PK2", "Internal %s", Internal_Path.c_str());

	PLog::Write(PLog::DEBUG, "PK2", "Allow %i", SDL_AndroidGetExternalStorageState);


	// Choose between internal or external path on Android
	// Prioritize internal
	if (PUtils::ExternalWriteable()) {

		PLog::Write(PLog::DEBUG, "PK2", "External access allowed");

		PFile::Path settings_f = PFile::Path("settings.ini");
		settings_f.SetPath(Internal_Path);
		PLog::Write(PLog::DEBUG, "PK2", "%s", settings_f.c_str());
		if (!settings_f.Find()) {

			PLog::Write(PLog::DEBUG, "PK2", "Settings not found on internal");

			settings_f.SetPath(External_Path);
			if (settings_f.Find()) {

				PLog::Write(PLog::DEBUG, "PK2", "Settings found on external");
				external_dir = true;
			
			}
			else {

				PLog::Write(PLog::DEBUG, "PK2", "Settings not found on external");
				external_dir = false;

			}
		} else {

			PLog::Write(PLog::DEBUG, "PK2", "Settings found on internal");
			external_dir = false;

		}
	} else {

		PLog::Write(PLog::DEBUG, "PK2", "External access not allowed");
		external_dir = false;

	}

	if (external_dir)
		data_path = External_Path;
	else
		data_path = Internal_Path;

	#endif //__ANDROID__

}


static void log_data() {

	PLog::Write(PLog::DEBUG, "PK2", "Pekka Kana 2 started!");
	PLog::Write(PLog::DEBUG, "PK2", "Game version: %s", PK2_VERSION_STR);
	PLog::Write(PLog::DEBUG, "PK2", "Data path - %s", data_path.c_str());

}
bool pk2_setAssetsPath(const std::string& path){
	PFile::SetAssetsPath(path);
	return true;
}

std::string pk2_get_version(){
	return PK2_VERSION_STR;
}

void pk2_init(){

	set_paths();
	PLog::Init(PLog::ALL, PFile::Path(data_path + "log.txt"));
	Prepare_DataPath();
}

void pk2_main(bool _dev_mode, bool _show_fps, bool _test_level, const std::string& test_path){
	
	dev_mode = _dev_mode;
	show_fps = _show_fps;
	test_level = _test_level;

	log_data();
	
	Settings_Open();
	
	read_config();

	Piste::init(screen_width, screen_height, PK2_NAME, "gfx" PE_SEP "icon_new.png",
	configuration.audio_buffer_size);
	
	if (!Piste::is_ready()) {

		PLog::Write(PLog::FATAL, "PK2", "Failed to init PisteEngine");
		quit();
	}

	ScreensHandler *handler = nullptr;
	try{
		handler = new ScreensHandler();
		Screen::next_screen = SCREEN_INTRO;
		if(dev_mode){
			Screen::next_screen = SCREEN_MENU;
		}
		
		if(test_level){
			start_test(test_path.c_str());
			Screen::next_screen = SCREEN_GAME;
		}

		Piste::loop(std::bind(&ScreensHandler::Loop, handler)); //The game loop
	}
	catch(const std::exception& e){
		PLog::Write(PLog::FATAL, "PK2", e.what());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error!", e.what(), nullptr);
	}

	if(handler!=nullptr){
		delete handler;
		handler = nullptr;
	}

	quit();
}

void replaceObsoleteAIs(PrototypeClass* prototype){

	bool is_info = false;
	std::vector<int>& vec = prototype->AI_v;

	for(const int& ai: vec){
		if(ai>=AI_LEGACY_INFOS_BEGIN && ai<=AI_LEGACY_INFOS_END){
			prototype->info_id = ai - AI_LEGACY_INFOS_BEGIN + 1;
			is_info = true;
		}
	}

	if(is_info){
		vec.erase(std::remove_if(vec.begin(), vec.end(), [](int ai) { return ai>=201 && ai<=302; }), vec.end());
		vec.push_back(302);
	}
}

void convertToSpr2(const std::string& filename_in, const std::string& filename_out){
	PrototypesHandler handler(false, false, nullptr);
	try{
		PrototypeClass* prototype = handler.loadPrototype(filename_in);
		replaceObsoleteAIs(prototype);
		nlohmann::json j = *prototype;
		handler.savePrototype(prototype, filename_out);
		PLog::Write(PLog::INFO, "PK2", "Sprite %s converted to %s\n", filename_in.c_str(), filename_out.c_str());
	}
	catch(const std::exception&e){
		printf("%s\n", e.what());
	}
}

void convertLevel(const std::string& filename_in, const std::string& filename_out, bool bg_tiles){
	try{
		LevelClass level;
		level.load(PFile::Path(filename_in), false);
		printf("Converting level \"%s\" to the new experimental format.\n", level.name.c_str());
		/*if(bg_tiles){
			level.tileset_bg_name = level.tileset_name.substr(0, level.tileset_name.size() - 4) + "_bg.bmp";
			printf("Waring, assuming _bg tileset: %s!\n", level.tileset_bg_name.c_str());
		}*/

		level.SaveVersion15(PFile::Path(filename_out));
		printf("Done!\n");
	}
	catch(const std::exception& e){
		printf("%s\n", e.what());
	}
}

bool pk2_convertToNewFormat(const std::string& filename_in, const std::string& filename_out, bool bg_tiles){

	PLog::Init(PLog::ALL, PFile::Path(data_path + "log.txt"));
	if(filename_in.empty()){
		printf("You have to specify the sprite to convert!");
		return false;
	}

	
	/**
	 * @brief 
	 * Sprite
	 */
	if(filename_in.size()>4 && filename_in.substr(filename_in.size()-4,4)==".spr"){
		std::string filename_out2;
		if(filename_out.empty()){
			filename_out2 = filename_in + "2";
		}
		else{
			filename_out2 = filename_out;
		}
		convertToSpr2(filename_in, filename_out2);
		return true;
	}
	else if(filename_in.size()>5 && filename_in.substr(filename_in.size()-5,5)==".spr2"){
		std::string filename_out2;
		if(filename_out.empty()){
			filename_out2 = filename_in;
		}
		convertToSpr2(filename_in, filename_out2);
		return true;
	}

	/**
	 * @brief 
	 * Level
	 */
	else if(filename_in.size()>4 && filename_in.substr(filename_in.size()-4,4)==".map"){
		std::string filename_out2;
		if(filename_out.empty()){
			filename_out2 = filename_in.substr(0,filename_in.size()-4) + ".pk2lev";
		}
		else{
			filename_out2 = filename_out;
		}
		convertLevel(filename_in, filename_out2, bg_tiles);
		return true;
	}

	printf("Unsupported file format to convert");
	
	return false;
}

/**
 * @brief 
 * A temporary function for updating sprites.
 */

void pk2_updateSprites(const std::string& dir){
	printf("%s\n", dir.c_str());
	
	for (const auto & entry : std::filesystem::directory_iterator(dir)){
        if(entry.is_regular_file()){
            std::filesystem::path p = entry.path();
            std::string s = p.string();
			printf("%s\n", s.c_str());
            if(s.size()>5 && s.substr(s.size()-5,5)==".spr2"){
				convertToSpr2(s, s);	
			}
		}
	}
}