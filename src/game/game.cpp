//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "game/game.hpp"

#include "game/gifts.hpp"
#include "system.hpp"
#include "gfx/text.hpp"
#include "gfx/particles.hpp"
#include "gfx/effect.hpp"
#include "episode/episodeclass.hpp"
#include "settings.hpp"
#include "gui.hpp"
#include "language.hpp"
#include "exceptions.hpp"

#include "engine/PSound.hpp"
#include "engine/PLog.hpp"
#include "engine/PDraw.hpp"
#include "engine/PInput.hpp"
#include "lua/pk2_lua.hpp"
#include "lua/lua_game.hpp"

#include <cstring>

GameClass* Game = nullptr;

GameClass::GameClass(int idx) {

	this->level_id = idx;
	this->map_file = Episode->levels_list[idx].tiedosto;

	if (Episode->level_status[idx] & LEVEL_PASSED)
		this->repeating = true;

}

GameClass::GameClass(std::string map_file) {

	this->map_file = map_file;

	bool found = false;
	for (uint i = 0; i < Episode->level_count; i++) {
		if (map_file == Episode->levels_list[i].tiedosto) {
			this->level_id = i;
			found = true;
		}
	}

	if (!found) {
		PLog::Write(PLog::FATAL, "PK2", "Couldn't find %s on episode", map_file.c_str());
		throw PExcept::PException("Couldn't find test level on episode");
	}

}

GameClass::~GameClass(){

	PSound::stop_music();
	this->spritesHandler.clearAll();

	if(this->lua!=nullptr){
		PK2lua::DestroyGameLuaVM(this->lua);
		this->lua = nullptr;
	}
}

int GameClass::Start() {

	if (this->started)
		return 1;
	
	this->spritesHandler.clearAll(); //Reset prototypes and sprites

	if(this->lua!=nullptr){
		PK2lua::DestroyGameLuaVM(this->lua);
		this->lua = nullptr;
	}

	Gifts_Clean(); //Reset gifts
	Fadetext_Init(); //Reset fade text
	GUI_Reset(); //Reset GUI

	if (this->Open_Map() == 1)
		throw PExcept::PException("Can't load level");

	this->moveBlocks();

	PSound::set_musicvolume(Settings.music_max_volume);

	if(this->lua!=nullptr){
		PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_GAME_STARTED);
	}

	this->started = true;
	
	return 0;
	
}

int GameClass::Finish() {

	if (this->level_clear)
		return -1;
	
	this->level_clear = true;

	if (PSound::start_music(PFile::Path("music" PE_SEP "hiscore.xm")) == -1)
		throw PExcept::PException("Can't find hiscore.xm");

	if(!test_level){
		Episode->level_status[this->level_id] |= LEVEL_PASSED;
	
		if (this->apples_count > 0)
			if (this->apples_got >= this->apples_count)
				Episode->level_status[this->level_id] |= LEVEL_ALLAPPLES;

		Episode->Update_NextLevel();
	}
	
	PSound::set_musicvolume_now(Settings.music_max_volume);

	return 0;

}

int GameClass::Open_Map() {
	
	PFile::Path path = Episode->Get_Dir(map_file);
	level.Load(path);

	/**
	 * @brief 
	 * Load lua
	 */
	if(this->level.lua_script!=""){
		this->lua = PK2lua::CreateGameLuaVM(this->level.lua_script);
	}
	else{
		PLog::Write(PLog::INFO, "PK2lua", "No Lua scripting in this level");
	}


	timeout = level.map_time * TIME_FPS;

	if (timeout > 0)
		has_time = true;
	else
		has_time = false;

	if (!Episode->use_button_timer) {
		level.button1_time = SWITCH_INITIAL_VALUE;
		level.button2_time = SWITCH_INITIAL_VALUE;
		level.button3_time = SWITCH_INITIAL_VALUE;
	}


	spritesHandler.loadAllLevelPrototypes(this->level);

	spritesHandler.prototypesHandler.loadSpriteAssets();

	Place_Sprites();

	if (this->chick_mode)
		PLog::Write(PLog::DEBUG, "PK2", "Chick mode on");
	
	this->keys = Count_Keys();

	Particles_Clear();
	Particles_LoadBG(&level);

	this->StartMusic();
	
	return 0;
}

void GameClass::StartMusic(){
	if (!level.music_name.empty()) {

		PFile::Path music_path = Episode->Get_Dir(level.music_name);

		if (!FindAsset(&music_path, "music" PE_SEP)) {

			PLog::Write(PLog::ERR, "PK2", "Can't find music \"%s\", trying \"song01.xm\"", music_path.GetFileName().c_str());
			music_path = PFile::Path("music" PE_SEP "song01.xm");

		}
		
		if (PSound::start_music(music_path) == -1)
			PLog::Write(PLog::FATAL, "PK2", "Can't load any music file");

	}
	else{
		PSound::stop_music();
	}
}

void GameClass::Place_Sprites() {
	PrototypeClass * proto = this->spritesHandler.getLevelPrototype(level.player_sprite_index);

	if(proto==nullptr){
		throw std::runtime_error("Null player prototype is quite serious error!");
	}

	//this->spritesHandler.addSprite(proto, 1, 0, 0, nullptr, false);
	this->spritesHandler.addPlayer(proto, 0, 0);
	this->Select_Start();

	for (u32 x = 0; x < PK2MAP_MAP_WIDTH; x++) {
		for (u32 y = 0; y < PK2MAP_MAP_HEIGHT; y++) {

			int sprite = level.sprite_tiles[x+y*PK2MAP_MAP_WIDTH];
			if(sprite<0||sprite>=255) continue;

			PrototypeClass* protot = this->spritesHandler.getLevelPrototype(sprite);
			if(protot==nullptr) continue;

			if(sprite!=255){ // Why is this index skipped ????
				
				if (protot->big_apple)
				this->apples_count++;

				if (protot->HasAI(AI_CHICK) || protot->HasAI(AI_CHICKBOX))
					this->chick_mode = true;

				this->spritesHandler.addLevelSprite(protot, x*32, y*32 - protot->height+32);
				//this->spritesHandler.addSprite(protot, 0, x*32, y*32 - protot->height+32, nullptr, false);
			}
		}
	}

	this->spritesHandler.sortBg();
}

void GameClass::Select_Start() {

	double  pos_x = 320;
	double  pos_y = 196;

	std::vector<u32> starts;

	for (u32 i = 0; i < PK2MAP_MAP_SIZE; i++)
		if (level.foreground_tiles[i] == BLOCK_START)
			starts.push_back(i);

	if (starts.size() > 0) {
		u32 i = starts[rand() % starts.size()];

		u32 x = i % PK2MAP_MAP_WIDTH;
		u32 y = i / PK2MAP_MAP_WIDTH;

		pos_x = x*32;
		pos_y = y*32;

	}

	SpriteClass * Player_Sprite = this->spritesHandler.Player_Sprite;
	Player_Sprite->x = pos_x + Player_Sprite->prototype->width/2;
	Player_Sprite->y = pos_y - Player_Sprite->prototype->height/2;

	this->camera_x = (int)Player_Sprite->x;
	this->camera_y = (int)Player_Sprite->y;
	this->dcamera_x = this->camera_x;
	this->dcamera_y = this->camera_y;

}

int GameClass::Count_Keys() {

	int keys = 0;

	for (u32 x=0; x < PK2MAP_MAP_SIZE; x++){
		u8 sprite = level.sprite_tiles[x];

		PrototypeClass*proto = this->spritesHandler.getLevelPrototype(sprite);
		if(proto==nullptr) continue;

		if (sprite != 255) // Why is this index skipped ????
			if (proto->can_open_locks && 
				!proto->indestructible)

				keys++;
	}

	return keys;
}

void GameClass::ExecuteEventsIfNeeded(){
	if(this->change_skulls){
		this->Change_SkullBlocks();
		this->change_skulls = false;

		if(this->lua!=nullptr){
			PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_SKULL_BLOCKS_CHANGED);
		}
	}

	if(this->event1){
		this->vibration = 90;
		PInput::Vibrate(1000);

		this->spritesHandler.onEvent1();
		this->event1 = false;

		if(this->lua!=nullptr){
			PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_1);
		}
	}

	if(this->event2){
		this->spritesHandler.onEvent2();
		this->event2 = false;

		if(this->lua!=nullptr){
			PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_2);
		}
	}
}

void GameClass::Change_SkullBlocks() {

	for (u32 x = 0; x < PK2MAP_MAP_WIDTH; x++)
		for (u32 y = 0; y < PK2MAP_MAP_HEIGHT; y++){
			
			u8 front = level.foreground_tiles[x+y*PK2MAP_MAP_WIDTH];
			u8 back  = level.background_tiles[x+y*PK2MAP_MAP_WIDTH];

			if (front == BLOCK_SKULL_FOREGROUND){
				level.foreground_tiles[x+y*PK2MAP_MAP_WIDTH] = 255;
				if (back != BLOCK_SKULL_FOREGROUND)
					Effect_SmokeClouds(x*32+24,y*32+6);

			}

			if (back == BLOCK_SKULL_BACKGROUND && front == 255)
				level.foreground_tiles[x+y*PK2MAP_MAP_WIDTH] = BLOCK_SKULL_FOREGROUND;
		}

	//Put in game
	this->vibration = 90;//60
	PInput::Vibrate(1000);

	level.Calculate_Edges();
	this->spritesHandler.onSkullBlocksChanged();
	//Sprites_changeSkullBlocks();
}

void GameClass::Open_Locks() {

	for (u32 x = 0; x < PK2MAP_MAP_WIDTH; x++)
		for (u32 y = 0; y < PK2MAP_MAP_HEIGHT; y++){
			
			u8 block = level.foreground_tiles[x+y*PK2MAP_MAP_WIDTH];
			
			if (block == BLOCK_LOCK){
				level.foreground_tiles[x+y*PK2MAP_MAP_WIDTH] = 255;
				Effect_SmokeClouds(x*32+24,y*32+6);
			}
		}

	//Put in game
	this->vibration = 90;//60
	PInput::Vibrate(1000);

	Show_Info(tekstit->Get_Text(PK_txt.game_locksopen));

	level.Calculate_Edges();

}

void GameClass::Show_Info(const std::string& text) {

	if (info_text.compare(text) != 0 || info_timer == 0) {

		info_text = text;
		info_timer = INFO_TIME;
	
	}
}

bool GameClass::isStarted() {

	return started;

}