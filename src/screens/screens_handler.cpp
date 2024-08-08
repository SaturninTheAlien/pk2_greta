//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "screens_handler.hpp"


#include "exceptions.hpp"
#include "sfx.hpp"
#include "settings/settings.hpp"
#include "gfx/touchscreen.hpp"
#include "language.hpp"
#include "gfx/text.hpp"
#include "save.hpp"
#include "system.hpp"

#include "episode/episodeclass.hpp"
#include "language.hpp"

#include "engine/Piste.hpp"
#include <sstream>
#include <ctime>

#include <time.h>


#include "intro_screen.hpp"
#include "menu_screen.hpp"
#include "map_screen.hpp"
#include "playing_screen.hpp"
#include "score_screen.hpp"
#include "ending_screen.hpp"

ScreensHandler::ScreensHandler():
	screens_map({
		{SCREEN_INTRO, new IntroScreen()},
		{SCREEN_MENU, new MenuScreen()},
		{SCREEN_MAP, new MapScreen()},
		{SCREEN_GAME, new PlayingScreen()},
		{SCREEN_SCORING, new ScoreScreen()},
		{SCREEN_END, new EndingScreen()},
	}){

	srand(time(nullptr));
	
	Calculate_SinCos();

	Fadetext_Init();

	if(PUtils::Is_Mobile())
		GUI_Load();

	tekstit = new PLang();
	if (Load_Language(Settings.language) != 0) {

		PLog::Write(PLog::ERR, "PK2", "Could not find %s!", Settings.language);
		strcpy(Settings.language, "english.txt");
		
		if(Load_Language(Settings.language) != 0) {
			throw PExcept::FileNotFoundException("english.txt", PExcept::MISSING_ENGLISH_TEXT);
		}

	}
	
	if (Load_Fonts(tekstit) != 0) {
		if (Load_Fonts(tekstit) != 0) {
			throw PExcept::PException("Couldn't load fonts!");
		}
	
	}

	langlist = PFile::Path("language" PE_SEP).scandir(".txt");
	
	Search_Episodes();
	
	PInput::SetVibration(Settings.vibration);

	if (Settings.using_controller == SET_TRUE)
		Input = &Settings.joystick;
	else if (Settings.using_controller == SET_FALSE)
		Input = &Settings.keyboard;
	else if (PInput::ControllerFound())
		Input = &Settings.joystick;
	else
		Input = &Settings.keyboard;

	int ret = Set_Screen_Mode(Settings.shader_type);
	if (ret != 0) {
		Settings.shader_type = SETTINGS_MODE_LINEAR;
		ret = Set_Screen_Mode(SETTINGS_MODE_LINEAR);
		Settings_Save();
	}
	
	PRender::set_fullscreen(Settings.isFullScreen);

	ret = -1;

	if (PUtils::Is_Mobile()) {
	
		ret = Piste::set_fps(-1);
	
	} else {

		if (Settings.fps == SETTINGS_VSYNC)
			ret = Piste::set_fps(-1);
		else if (Settings.fps == SETTINGS_30FPS)
			ret = Piste::set_fps(30);
		else if (Settings.fps == SETTINGS_60FPS)
			ret = Piste::set_fps(60);
		else if (Settings.fps == SETTINGS_85FPS)
			ret = Piste::set_fps(85);
		else if (Settings.fps == SETTINGS_120FPS)
			ret = Piste::set_fps(120);

	}

	if (ret != 0) {
		PLog::Write(PLog::ERR, "PK2", "FPS mode not supported, changing to 60fps");
		Piste::set_fps(60);
		Settings.fps = SETTINGS_60FPS;
		Settings_Save();
	}

	PDraw::image_load(game_assets, PFile::Path("gfx" PE_SEP "pk2stuff.bmp"), true);
	PDraw::image_load(game_assets2, PFile::Path("gfx" PE_SEP "pk2stuff2.bmp"), true);

	//PSound::load_overlay_music(PFile::Path("music" PE_SEP "super.xm"));  // why? what is so special about this one xm that it needs to be loaded at runtime?
	// I propose that we resort to start_music() for powerups that require special effects.	

	sfx_global.loadAll();
	//Load_SFX();

	Load_SaveFile();

	Fade_in(FADE_SLOW);
	PSound::set_musicvolume_now(Settings.music_max_volume);
}

ScreensHandler::~ScreensHandler(){

	for(std::pair<int, Screen*> p : this->screens_map){
		if(p.second!=nullptr){
			delete p.second;
			p.second = nullptr;
		}	
	}
	this->screens_map.clear();
	sfx_global.free();

	PDraw::image_delete(game_assets);
	PDraw::image_delete(game_assets2);
}


//Main Loop
void ScreensHandler::Loop() {

	if(Screen::next_screen != this->current_screen_index){
		this->current_screen_index = Screen::next_screen;

		auto it = this->screens_map.find(Screen::next_screen);
		if(it==this->screens_map.end()){
			std::ostringstream os;
			os<<"Screen with index "<<Screen::next_screen<<" not found!";
			throw PExcept::PException(os.str());
		}
		this->current_screen = it->second;		

		Fade_in(FADE_NORMAL);

		this->current_screen->Init();
	}

	if(PInput::Keydown(PInput::F11)){
		if(this->mFullScreenKeyDelay==0){
			Settings.isFullScreen = !Settings.isFullScreen;
			PRender::set_fullscreen(Settings.isFullScreen);
			this->mFullScreenKeyDelay = 20;			
		}
	}
	else if(this->mFullScreenKeyDelay>0){
		--this->mFullScreenKeyDelay;
	}
	

	PInput::UpdateMouse(this->current_screen->keys_move, Settings.isFullScreen);
	
	if (PUtils::Is_Mobile())
		GUI_Update();

	this->current_screen->Loop();

	if (key_delay > 0)
		key_delay--;

	if (Screen::closing_game && !Is_Fading()){
		Piste::stop();
	}
	// Fade and thunder
	Update_Colors();
}