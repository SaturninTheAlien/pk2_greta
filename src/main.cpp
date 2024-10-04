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
#include "engine/Piste.hpp"
#include "engine/PFilesystem.hpp"
#include "version.hpp"

#include "screens/screens_handler.hpp"
#include "gfx/text.hpp"
#include "game/game.hpp"
#include "episode/episodeclass.hpp"
#include "episode/mapstore.hpp"
#include "save.hpp"
#include "gfx/touchscreen.hpp"
#include "system.hpp"
#include "language.hpp"

#include "settings/settings.hpp"
#include "settings/config_txt.hpp"

#include <filesystem>
#include <algorithm>

#include "game/prototypes_handler.hpp"

#include <cstring>
#include <algorithm>

#include <SDL.h>

#include "utils/file_converter.hpp"


static void start_test(const char* arg) {
	
	if (arg == NULL) return;

	/**
	 * @brief 
	 * TODO
	 * Not to load the whole episode while testing a level
	 */

	std::filesystem::path path(arg);

	//PFile::Path path(arg);
	episode_entry episode;
	episode.name = path.parent_path().filename().string();
	episode.is_zip = false;
	Episode = new EpisodeClass("test", episode);

	Game = new GameClass(path.filename().string());

	PLog::Write(PLog::DEBUG, "PK2", "Testing episode '%s' level '%s'", episode.name.c_str(), Game->map_file.c_str());

}

static void quit() {

	PSound::stop_music();

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

	PFilesystem::SetDefaultAssetsPath();
	
	#ifndef __ANDROID__

	#ifdef PK2_PORTABLE

	/**
	 * @brief 
	 * TODO
	 * Redesign it
	 */

	data_path =  (std::filesystem::path(PFilesystem::GetAssetsPath()) / "data").string() + "/";

	PFilesystem::CreateDirectory(data_path);
	PFilesystem::CreateDirectory(data_path + "scores");
	PFilesystem::CreateDirectory(data_path + "mapstore");

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

	/**
	 * @brief 
	 * TO DO Rewrite it and move to PFilesystem
	 */

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

void pk2_init(){

	set_paths();
	PLog::Init(PLog::ALL, true, true);
}

void pk2_main(bool _dev_mode, bool _show_fps, bool _test_level, const std::string& test_path){

	ScreensHandler *handler = nullptr;
	dev_mode = _dev_mode;
	show_fps = _show_fps;
	test_level = _test_level;
	try{
		log_data();
		
		Settings_Open();

		config_txt.readFile();

		if(!_test_level){
			Search_Episodes();
		}	

		std::optional<PFile::Path> iconPath = PFilesystem::FindVanillaAsset("icon_new.png", PFilesystem::GFX_DIR);
		if(!iconPath.has_value()){
			throw std::runtime_error("icon_new.png not found!");
		}

		Piste::init(screen_width, screen_height, PK2_NAME_STR, iconPath->c_str(),
		config_txt.audio_buffer_size);
		
		if (!Piste::is_ready()) {

			PLog::Write(PLog::FATAL, "PK2", "Failed to init PisteEngine");
			quit();
		}	
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

int main(int argc, char **argv) {

	bool test_level = false;
	bool dev_mode = false;
	bool show_fps = false;
	bool converting_sprite = false;
	bool updating_sprites = false;

	std::string filename_in;
	std::string filename_out;
	std::string test_path;

	int state = 0;
	for(int i=1;i<argc;++i){
		std::string arg = argv[i];
		switch (state)
		{
		case 0:{
			if(arg=="--help" || arg=="-h"){
				printf("Pekka Kana 2 (Pekka the Rooster 2) is a jump 'n run game made "
				"in the spirit of classic platformers such as Super Mario, SuperTux, Sonic the Hedgehog,"
				" Jazz Jackrabbit, Super Frog and so on.\n"
				"Available command arguments are:\n"
				"-h / --help -> print help,\n"
				"-v / --version -> print version string,\n"
				"-t / --test \"episode/level\" -> test/play particular level\n"
				"(e.g ./pekka-kana-2 --test \"rooster island 2/level13.map\"),\n"
				"-d / --dev -> enable the cheats and the debug tools,\n"
				"--fps -> enable the FPS counter.\n"
				);
				return 0;
			}
			else if(arg=="--version" || arg=="-v"){
				printf("%s\n", PK2_VERSION_STR);
				return 0;
			}
			else if(arg=="--dev" || arg=="-d" || arg=="dev"){
				dev_mode = true;
				//Piste::set_debug(true);
			}
			else if(arg=="--test" || arg=="-t" || arg=="test"){
				state = 1;				
			}
			else if(arg=="--path" || arg=="-p"){
				state = 2;
			}
			else if(arg=="--fps"){
				show_fps= true;
			}
			/*else if (arg=="--mobile") {
				PUtils::Force_Mobile();
			}*/
			else if	(arg=="--convert"){
				filename_in = "";
				filename_out = "";
				state=3;
				converting_sprite = true;
			}
			else if (arg=="--update-sprites"){
				filename_in = ".";
				filename_out = "";
				state=5;
				updating_sprites = true;
			}
			else {
				printf("Invalid arg\n");
				return 1;
			}
		}
		break;
		case 1:{
			test_path = arg;
			test_level = true;
			state = 0;
		}
		break;
		case 2:{
			if(!PFilesystem::SetAssetsPath(arg)){
				printf("Incorrect assets path \"%s\"!", arg.c_str());
				return 1;
			}
			state = 0;
		}
		break;
		case 3:{
			filename_in = arg;
			state = 4;			
		}
		break;
		case 4:{
			filename_out = arg;
			state = 0;
		}
		break;
		case 5:{
			filename_in = arg;
			state = 0;
		}
		break;

		default:
			printf("Invalid arg: \"%s\"\n", arg.c_str());
			return 1;
		}
	}

	pk2_init();

	if(updating_sprites){
		pk2_updateSprites(filename_in);
	}
	else if(converting_sprite){
		pk2_convertToNewFormat(filename_in, filename_out);
	}
	else{
		pk2_main(dev_mode, show_fps, test_level, test_path);
	}
	return 0;
}