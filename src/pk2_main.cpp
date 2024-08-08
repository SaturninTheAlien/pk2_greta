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

#include "settings/settings.hpp"
#include "settings/config_txt.hpp"

#include <filesystem>
#include <algorithm>

#include "game/prototypes_handler.hpp"

#include <cstring>
#include <algorithm>

#include <SDL.h>




static void start_test(const char* arg) {
	
	if (arg == NULL) return;

	PFile::Path path(arg);

	/**
	 * @brief 
	 * TODO
	 * Not to load the whole episode while testing a level
	 */

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

	config_txt.readFile();

	Piste::init(screen_width, screen_height, PK2_NAME, "gfx" PE_SEP "icon_new.png",
	config_txt.audio_buffer_size);
	
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

		level.saveVersion15(PFile::Path(filename_out));
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


int main(int argc, char **argv) {

	bool test_level = false;
	bool dev_mode = false;
	bool show_fps = false;
	bool converting_sprite = false;
	bool converting_level_with_bg_tiles = false;
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
			else if(arg=="--convert-bg"){
				converting_level_with_bg_tiles = true;
				filename_in = "";
				filename_out = "";
				state=3;
				converting_sprite = true;
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
			if(!pk2_setAssetsPath(arg)){
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
			printf("Invalid arg\n");
			return 1;
		}
	}

	pk2_init();

	if(updating_sprites){
		pk2_updateSprites(filename_in);
	}
	else if(converting_sprite){
		pk2_convertToNewFormat(filename_in, filename_out, converting_level_with_bg_tiles);
	}
	else{
		pk2_main(dev_mode, show_fps, test_level, test_path);
	}
	return 0;
}