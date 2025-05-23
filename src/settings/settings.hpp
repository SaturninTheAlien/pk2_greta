//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/platform.hpp"

#include "engine/PFile.hpp"
#include <string>

enum {
	SETTINGS_VSYNC,
	SETTINGS_60FPS,
	SETTINGS_85FPS,
	SETTINGS_120FPS,
	SETTINGS_30FPS
};

enum {
	SETTINGS_MODE_NEAREST,
	SETTINGS_MODE_LINEAR,
	SETTINGS_MODE_CRT,
	SETTINGS_MODE_HQX
};

enum {
	SET_FALSE,
	SET_TRUE,
	NOT_SET,
};

#define SETTINGS_VERSION "1.8"

class GAME_CONTROLS {
public:
	u32 left;
	u32 right;
	u32 up;
	u32 down;

	u32 jump;
	u32 walk_slow;
	u32 attack1;
	u32 attack2;
	u32 open_gift;

	friend void from_json(const nlohmann::json& j, GAME_CONTROLS& controls);
	friend void to_json(nlohmann::json& j, const GAME_CONTROLS& controls);

};

class PK2SETTINGS {
public:	
	u32  id;

	std::string language;

	bool  transparent_text = false;

	bool  draw_gui = false;
	bool  touchscreen_mode = false;
	
	int   fps;  // desired fps target (default 60)
	bool  isFullScreen = true;
	bool  double_speed = false;
	u8    shader_type;

	// Controls
	GAME_CONTROLS keyboard;
	GAME_CONTROLS joystick;
	u8 using_controller;
	u16 vibration;
	
	// Audio
	u8  music_max_volume;
	u8  sfx_max_volume;

	// GUI
	//bool gui;
	bool show_fps;

	friend void from_json(const nlohmann::json& j, PK2SETTINGS& settings);
	friend void to_json(nlohmann::json& j, const PK2SETTINGS& settings); 
};

extern PK2SETTINGS Settings;
extern GAME_CONTROLS* Input;

//int Settings_GetId(PFile::Path path, u32& id);

void Settings_Open();
void Settings_Save();