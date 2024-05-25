//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include <array>
#include "engine/types.hpp"

#include "sprites_handler.hpp"
#include "levelclass.hpp"

const int TIME_FPS = 100; //(dec)conds * TIME_FPS = FRAMES
const int INFO_TIME = 700;

namespace sol{
	class state;
}

class GameClass {

	public:

		u32 level_id = -1;
		LevelClass level;
		std::string map_file;	
		
		bool game_over = false;
		bool level_clear = false;
		bool repeating = false;

		bool chick_mode = false;

		u32 exit_timer = 0;

		int  timeout = 0; //timeout in frames
		bool has_time = false;
		u64  frame_count = 0; //flames elapsed

		int palikka_animaatio = 0;

		int button_vibration = 0;
		u32 button1 = 0;
		u32 button2 = 0;
		u32 button3 = 0;

		u32 score = 0;
		u32 score_increment = 0;

		u32 apples_count = 0;
		u32 apples_got = 0;

		int vibration = 0;

		int camera_x;
		int camera_y;
		double dcamera_x;
		double dcamera_y;
		double dcamera_a;
		double dcamera_b;

		bool paused = false;
		bool music_stopped = false;

		int keys = 0;

		int info_timer = 0;
		std::string info_text;

		int item_pannel_x = -215;

		GameClass(int idx);
		GameClass(std::string map_file);
		~GameClass();

		int Start();
		int Finish();

		void moveBlocks(){
			this->level.moveBlocks(this->button1, this->button2, this->button3);
		}


		void Show_Info(const std::string& text);

		bool isStarted();

		void Place_Sprites();
		void Select_Start();
		int Count_Keys();
		void Open_Locks();

		bool change_skulls=false;
		bool event1 = false;
		bool event2 = false;

		SpritesHandler spritesHandler;

		void Change_SkullBlocks();
		void ExecuteEventsIfNeeded();
		void StartMusic();
		
		sol::state * lua = nullptr;
	private:		
		bool started = false;
		int Open_Map();

};

extern GameClass* Game;
