//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "game/game.hpp"

#include "game/gifts.hpp"
#include "system.hpp"
#include "gfx/text.hpp"
#include "gfx/particles.hpp"
#include "gfx/bg_particles.hpp"
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
#include "lua/lua_game_events.hpp"

#include <cstring>

GameClass* Game = nullptr;

GameClass::GameClass(int idx):
spritePrototypes(true, true, Episode){

	this->level_id = idx;
	this->map_file = Episode->levels_list[idx].tiedosto;

	if (Episode->level_status[idx] & LEVEL_PASSED)
		this->repeating = true;

}

GameClass::GameClass(std::string map_file):
spritePrototypes(true, true, Episode){

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
	PDraw::palette_set(default_palette);
	level.clear();
	this->spritePrototypes.clear();

	if(this->lua!=nullptr){
		PK2lua::DestroyGameLuaVM(this->lua);
		this->lua = nullptr;
	}
}

int GameClass::Start() {

	if (this->started)
		return 1;
	
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
	level.load(path, false);

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

	this->placeSprites();
	this->level.calculateBlockTypes();
	for(LevelSector* sector: this->level.sectors){
		sector->calculateEdges();
	}

	//Place_Sprites();
	/***
	 * TO DO
	*/
	Particles_Clear();

	BG_Particles::Init(this->playerSprite->level_sector->weather);
	this->playerSprite->level_sector->startMusic();
	
	return 0;
}

void GameClass::placeSprites() {
	std::array<PrototypeClass*, 255> mapping;

	//Load prototypes
	std::size_t prototypes_number = this->level.sprite_prototype_names.size();
	if(prototypes_number > mapping.size()){
		std::ostringstream os;
		os<<"Too many sprite prototypes: "<<prototypes_number<<std::endl;
		os<<mapping.size()<<" is the current limit."<<std::endl;
		os<< "Dependency sprites (ammo, transformation and so on)"
		" do not count into the limit";

		throw std::runtime_error(os.str());
	}

	for(std::size_t i=0;i<mapping.size();++i){
		mapping[i] = nullptr;
	}

	for(std::size_t i=0;i<prototypes_number;++i){
		const std::string& name = level.sprite_prototype_names[i];
		if(!name.empty()){
			mapping[i] =  this->spritePrototypes.loadPrototype(level.sprite_prototype_names[i]);
		}
	}

	// Load sprite assets
	this->spritePrototypes.loadSpriteAssets();

	// Player prototype
	int player_index = level.player_sprite_index;
	if(player_index<0 || player_index >= int(mapping.size())){
		std::ostringstream os;
		os<<"Incorrect player sprite index: "<<player_index;
		throw PExcept::PException(os.str());
	}


	PrototypeClass* player_prototype = mapping[player_index];
	if(player_prototype==nullptr){
		throw PExcept::PException("Null player prototype is quite serious error!");
	}

	// Add player
	{
		double pos_x = 0;
		double pos_y = 0;
		u32 sector_id = 0;

		this->selectStart(pos_x, pos_y, sector_id);
		pos_x += 17;

		this->playerSprite= this->level.sectors[sector_id]->sprites.addPlayer(player_prototype, pos_x, pos_y);

		this->level.sectors[sector_id]->background->setPalette();
		this->setCamera();
	}

	// Add other sprites
	for(LevelSector* sector: this->level.sectors){


		for (u32 y = 0; y < sector->getHeight(); y++){
			for (u32 x = 0; x < sector->getWidth(); x++) {
			

				int sprite = sector->sprite_tiles[x+y*sector->getWidth()];
				if(sprite<0||sprite>=255) continue;

				PrototypeClass* prototype = mapping[sprite];
				if(prototype==nullptr) continue;

				/**
				 * @brief 
				 * Count big apples
				 */
				if(prototype->big_apple){
					this->apples_count++;
				}

				/**
				 * @brief 
				 * Count keys
				 */
				if (prototype->can_open_locks && 
					!prototype->indestructible){
					this->keys++;
				}

				/**
				 * @brief 
				 * Count enemies
				 */
				if(prototype->type == TYPE_GAME_CHARACTER
				&& !prototype->indestructible
				&& prototype->damage > 0 //to ignore switches, boxes and so on
				&& prototype->enemy){
					this->enemies++;
				}

				sector->sprites.addLevelSprite(prototype, x*32, y*32 - prototype->height+32);
			}
		}

		sector->sprites.sortBg();
	}
}

void GameClass::selectStart(double& pos_x, double& pos_y, u32 sector) {

	pos_x = 320;
	pos_y = 196;

	std::vector<BlockPosition> startSigns;

	for(u32 i=0; i<level.sectors.size(); ++i){
		level.sectors[i]->countStartSigns(startSigns, i);
	}

	int selected_start = 0;
	if(startSigns.size()>1){
		selected_start = rand() % startSigns.size();
	}


	if(startSigns.size()>0){
		pos_x = startSigns[selected_start].x * 32;
		pos_y = startSigns[selected_start].y * 32;
		sector = startSigns[selected_start].sector;
	}
}

SpriteClass* GameClass::selectTeleporter(SpriteClass* entryTelporter, PrototypeClass* exitPrototype){
	std::vector<SpriteClass*> teleporters;
	for(LevelSector* sector: this->level.sectors){
		for(SpriteClass* sprite: sector->sprites.Sprites_List){
			if(sprite->prototype== exitPrototype && sprite!=entryTelporter){
				teleporters.push_back(sprite);
			}
		}
	}

	if(teleporters.size()==0)return nullptr;
	else if(teleporters.size()==1) return teleporters[0];
	else return teleporters[rand()% teleporters.size()];
}

void GameClass::teleportPlayer(double x, double y, LevelSector*sector){
	this->playerSprite->x = x;
	this->playerSprite->y = y;

	/**
	 * @brief 
	 * Change sector
	 */
	if(sector!=nullptr && this->playerSprite->level_sector!=sector){

		LevelSector* previous_sector = this->playerSprite->level_sector;
		previous_sector->sprites.Sprites_List.remove(this->playerSprite);

		sector->sprites.Sprites_List.push_front(this->playerSprite);
		this->playerSprite->level_sector=sector;

		//Change palette
		sector->background->setPalette();

		//Change weather
		BG_Particles::Init(sector->weather);
		Particles_Clear();

		//Change music
		if(sector->music_name!=previous_sector->music_name){
			sector->startMusic();
		}
	}

	Fade_in(FADE_NORMAL);

	this->setCamera();
}

void GameClass::ExecuteEventsIfNeeded(){
	if(this->change_skulls){
		this->change_skulls = false;

		this->vibration = 90;
		PInput::Vibrate(1000);

		for(LevelSector* sector: this->level.sectors){
			sector->changeSkulls(sector==this->playerSprite->level_sector);
			sector->sprites.onSkullBlocksChanged();
		}

		if(this->lua!=nullptr){
			PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_SKULL_BLOCKS_CHANGED);
		}
	}

	if(this->event1){
		this->event1 = false;

		this->vibration = 90;
		PInput::Vibrate(1000);

		for(LevelSector* sector: this->level.sectors){
			sector->sprites.onEvent1();
		}
		

		if(this->lua!=nullptr){
			PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_1);
		}
	}

	if(this->event2){
		this->event2 = false;

		for(LevelSector* sector: this->level.sectors){
			sector->sprites.onEvent2();
		}
		

		if(this->lua!=nullptr){
			PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_2);
		}
	}

	if(this->level.game_mode == GAME_MODE_KILL_ALL &&
		this->enemies <= 0 &&
		this->playerSprite->damage_taken==0 && 
		this->playerSprite->damage_timer==0 && 
		this->playerSprite->energy>0){

		this->Finish();
	}

	if(this->lua!=nullptr && !this->paused){
		PK2lua::UpdateLua();
	}
}

void GameClass::Open_Locks() {
	//Put in game
	this->vibration = 90;//60
	PInput::Vibrate(1000);

	Show_Info(tekstit->Get_Text(PK_txt.game_locksopen));

	for(LevelSector* sector: this->level.sectors){
		sector->openKeylocks(sector==this->playerSprite->level_sector);
	}
}

void GameClass::Show_Info(const std::string& text) {

	if (info_text.compare(text) != 0 || info_timer == 0) {

		info_text = text;
		info_timer = INFO_TIME;
	
	}
}

void GameClass::setCamera(){

	LevelSector* sector = this->playerSprite->level_sector;

	this->camera_x = (int)this->playerSprite->x - screen_width/2;
	this->camera_y = (int)this->playerSprite->y - screen_height/2;
	
	if (this->camera_x < 0)
		this->camera_x = 0;

	if (this->camera_y < 0)
		this->camera_y = 0;

	if (this->camera_x > int(sector->getWidth() -screen_width/32)*32)
		this->camera_x = int(sector->getWidth()-screen_width/32)*32;

	if (this->camera_y > int(sector->getHeight()-screen_height/32)*32)
		this->camera_y = int(sector->getHeight()-screen_height/32)*32;

	this->dcamera_x = this->camera_x;
	this->dcamera_y = this->camera_y;

	this->dcamera_a = 0;
	this->dcamera_b = 0;
}

void GameClass::updateCamera(){
	this->camera_x = (int) this->playerSprite->x-screen_width / 2;
	this->camera_y = (int) this->playerSprite->y-screen_height / 2;

	LevelSector* sector = playerSprite->level_sector;
	
	if(dev_mode && PInput::MouseLeft() /*&& !PUtils::Is_Mobile()*/) {
		this->camera_x += PInput::mouse_x - screen_width / 2;
		this->camera_y += PInput::mouse_y - screen_height / 2;
	}

	if (this->vibration > 0) {
		this->dcamera_x += (rand()%this->vibration-rand()%this->vibration)/5;
		this->dcamera_y += (rand()%this->vibration-rand()%this->vibration)/5;

		this->vibration--;
	}

	if (this->button_vibration > 0) {
		this->dcamera_x += (rand()%9-rand()%9);//3
		this->dcamera_y += (rand()%9-rand()%9);

		this->button_vibration--;
	}

	if (this->dcamera_x != this->camera_x)
		this->dcamera_a = (this->camera_x - this->dcamera_x) / 15;

	if (this->dcamera_y != this->camera_y)
		this->dcamera_b = (this->camera_y - this->dcamera_y) / 15;

	if (this->dcamera_a > 6)
		this->dcamera_a = 6;

	if (this->dcamera_a < -6)
		this->dcamera_a = -6;

	if (this->dcamera_b > 6)
		this->dcamera_b = 6;

	if (this->dcamera_b < -6)
		this->dcamera_b = -6;

	this->dcamera_x += this->dcamera_a;
	this->dcamera_y += this->dcamera_b;

	this->camera_x = (int)this->dcamera_x;
	this->camera_y = (int)this->dcamera_y;

	if (this->camera_x < 0)
		this->camera_x = 0;

	if (this->camera_y < 0)
		this->camera_y = 0;

	if (this->camera_x > int(sector->getWidth() -screen_width/32)*32)
		this->camera_x = int(sector->getWidth()-screen_width/32)*32;

	if (this->camera_y > int(sector->getHeight()-screen_height/32)*32)
		this->camera_y = int(sector->getHeight()-screen_height/32)*32;
}


