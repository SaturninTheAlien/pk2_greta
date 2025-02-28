//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "settings.hpp"

#include "language.hpp"
#include "system.hpp"

#include "engine/PLog.hpp"
#include "engine/PInput.hpp"
#include "engine/PJson.hpp"
#include "engine/PFilesystem.hpp"

#include <ctime>
#include <cstring>
#include <string>
#include <fstream>

#include <SDL_timer.h>

const char* SETTINGS_FILE = "settings.json";

PK2SETTINGS Settings;
GAME_CONTROLS* Input = nullptr;

void Settings_Init() {

	u64 counter = SDL_GetPerformanceCounter();
	u32 counter32 = counter ^ (counter >> 32);
	counter32 ^= counter32 << 10;

	u32 timer = time(NULL);
	timer ^= timer << 1;
	
	u32 seed = counter32 ^ timer;

	srand(seed);
	Settings.id = rand();
	Settings.id <<= 1;
	Settings.id ^= rand();

	Settings.language = GetDefaultLanguageName();

	Settings.transparent_text = false;
	Settings.draw_gui = true;

#ifdef __ANDROID__
	Settings.touchscreen_mode = true;
#else
	Settings.touchscreen_mode = false;
#endif

	Settings.fps = SETTINGS_60FPS;//SETTINGS_VSYNC;
	Settings.isFullScreen = true;
	Settings.double_speed = false;
	Settings.shader_type = SETTINGS_MODE_LINEAR;

	Settings.keyboard.left      = PInput::LEFT;
	Settings.keyboard.right     = PInput::RIGHT;
	Settings.keyboard.up        = PInput::UP;
	Settings.keyboard.down      = PInput::DOWN;
	Settings.keyboard.jump      = PInput::UP;
	Settings.keyboard.walk_slow = PInput::LALT;
	Settings.keyboard.attack1   = PInput::LCONTROL;
	Settings.keyboard.attack2   = PInput::LSHIFT;
	Settings.keyboard.open_gift = PInput::SPACE;

	Settings.using_controller   = NOT_SET;
	Settings.vibration          = 0xFFFF/2;
	Settings.joystick.left      = PInput::JOY_LEFT;
	Settings.joystick.right     = PInput::JOY_RIGHT;
	Settings.joystick.up        = PInput::JOY_UP;
	Settings.joystick.down      = PInput::JOY_DOWN;
	Settings.joystick.jump      = PInput::JOY_UP;
	Settings.joystick.walk_slow = PInput::JOY_Y;
	Settings.joystick.attack1   = PInput::JOY_A;
	Settings.joystick.attack2   = PInput::JOY_B;
	Settings.joystick.open_gift = PInput::JOY_LEFTSHOULDER;

	Settings.music_max_volume = 35; //30;
	Settings.sfx_max_volume = 85; //95;

	Id_To_String(Settings.id, id_code, 8);

}


void from_json(const nlohmann::json& j, GAME_CONTROLS& controls){
	j.at("left").get_to(controls.left);
	j.at("right").get_to(controls.right);
	j.at("up").get_to(controls.up);
	j.at("down").get_to(controls.down);

	j.at("jump").get_to(controls.jump);
	j.at("walk_slow").get_to(controls.walk_slow);
	j.at("attack1").get_to(controls.attack1);
	j.at("attack2").get_to(controls.attack2);
	j.at("open_gift").get_to(controls.open_gift);
	
}

void to_json(nlohmann::json& j, const GAME_CONTROLS& controls){
	j["left"] = controls.left;
	j["right"] = controls.right;
	j["up"] = controls.up;
	j["down"] = controls.down;

	j["jump"] = controls.jump;
	j["walk_slow"] = controls.walk_slow;
	j["attack1"] = controls.attack1;
	j["attack2"] = controls.attack2;

	j["open_gift"] = controls.open_gift;	
}


void from_json(const nlohmann::json& j, PK2SETTINGS& s){
	j.at("language").get_to(s.language);
	j.at("gui").get_to(s.draw_gui);
	j.at("touchscreen").get_to(s.touchscreen_mode);
	j.at("fps").get_to(s.fps);
	j.at("fullscreen").get_to(s.isFullScreen);
	j.at("double_speed").get_to(s.double_speed);

	j.at("keyboard").get_to(s.keyboard);
	j.at("using_controller").get_to(s.using_controller);
	j.at("vibration").get_to(s.vibration);

	j.at("joystick").get_to(s.joystick);

	j.at("music_max_volume").get_to(s.music_max_volume);

	j.at("sfx_max_volume").get_to(s.sfx_max_volume);

	j.at("transparent_text").get_to(s.transparent_text);

	j.at("filtering").get_to(s.shader_type);
}

void to_json(nlohmann::json& j, const PK2SETTINGS& s){
	j["language"] = s.language;
	j["gui"] = s.draw_gui;

	j["touchscreen"] = s.touchscreen_mode;
	j["fps"] = s.fps;

	j["fullscreen"] = s.isFullScreen;
	j["double_speed"] = s.double_speed;

	j["keyboard"] = s.keyboard;
	j["using_controller"] = s.using_controller;


	j["vibration"] = s.vibration;
	j["joystick"] = s.joystick;

	j["music_max_volume"] = s.music_max_volume;
	j["sfx_max_volume"] = s.sfx_max_volume;
	j["transparent_text"] = s.transparent_text;

	j["filtering"] = s.shader_type;
}


void Settings_Open() {

	PFile::Path path = PFilesystem::GetDataFileW(SETTINGS_FILE);

	try{
		if(path.exists()){
			Settings = path.GetJSON().get<PK2SETTINGS>();
		}
		else{
			PLog::Write(PLog::DEBUG, "PK2", "No settings found");
			Settings_Init();
			Settings_Save();
		}
		//
	}
	catch(const PFile::PFileException& e){
		PLog::Write(PLog::DEBUG, "PK2", "No settings found");
		Settings_Init();
		Settings_Save();		
		
	}
	catch(const std::exception& e){
		PLog::Write(PLog::ERR, "PK2", e.what());
		Settings_Init();
		Settings_Save();
	}

}

void Settings_Save() {

	PFile::Path path = PFilesystem::GetDataFileW(SETTINGS_FILE);
	nlohmann::json j = Settings;
	std::ofstream f(path.c_str());

	f << j.dump(4)<<std::endl;
	f.close();
}