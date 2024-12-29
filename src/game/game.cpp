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
#include "gfx/bg_particles.hpp"

#include "episode/episodeclass.hpp"

#include "settings/settings.hpp"
#include "settings/config_txt.hpp"


#include "gfx/touchscreen.hpp"
#include "language.hpp"
#include "exceptions.hpp"

#include "engine/PSound.hpp"
#include "engine/PLog.hpp"
#include "engine/PDraw.hpp"
#include "engine/PInput.hpp"
#include "engine/PFilesystem.hpp"

#include "lua/pk2_lua.hpp"
#include "lua/lua_game_events.hpp"

#include <cstring>

GameClass* Game = nullptr;

GameClass::GameClass(int idx):
spritePrototypes(Episode){

	this->level_id = idx;
	this->level_file =  Episode->getLevelFilename(idx);

	if(Episode->getLevelStatus(idx) & LEVEL_PASSED ){
		this->repeating = true;
	}
}

GameClass::GameClass(std::string level_file):
spritePrototypes(Episode){

	this->repeating = true;

	this->level_file = level_file;

	if(!test_level){
		int level_id_tmp = Episode->findLevelbyFilename(level_file);
		if(level_id_tmp==-1){
			PLog::Write(PLog::FATAL, "PK2", "Couldn't find %s on episode", level_file.c_str());
			throw PExcept::PException("Couldn't find test level on episode");
		}
		this->level_id = level_id_tmp;
	}
	else{
		this->level_id = -1;
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

void GameClass::update(int& debug_active_sprites){
	if(this->playerSprite!=nullptr && this->playerSprite->energy>0){
		AI_Functions::player_invisible = this->playerSprite;

		if(this->playerSprite->invisible_timer>0){
			AI_Functions::player = nullptr;
		}
		else{
			AI_Functions::player = this->playerSprite;
		}

	}
	else{
		AI_Functions::player = nullptr;
		AI_Functions::player_invisible = nullptr;
	}

	LevelSector* sector = this->playerSprite->level_sector;


	if (!this->level_clear && (!this->has_time || this->timeout > 0)) {
		this->level.setTilesAnimations(degree, this->palikka_animaatio/7, this->button1, this->button2, this->button3);
		this->palikka_animaatio = 1 + this->palikka_animaatio % 34;
	}

	this->updateCamera();
	Update_GameSFX();

	/***
	 * Execute events
	 */

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

	if (!this->paused) {

		/**
		 * @brief 
		 * Kill all the enemies mode
		 */

		if(this->level.game_mode == GAME_MODE_KILL_ALL &&
			this->enemies <= 0 &&
			this->playerSprite->damage_taken==0 && 
			this->playerSprite->damage_timer==0 && 
			this->playerSprite->energy>0){

			this->finish();
		}

		/**
		 * @brief 
		 * Update Lua
		 */

		if(this->lua!=nullptr){
			PK2lua::UpdateLua();
		}

		/**
		 * @brief 
		 * Update particles
		 */

		BG_Particles::Update(this->camera_x, this->camera_y);
		Particles_Update();

		/***
		 * Update sprites
		 */

		if (!this->level_clear && (!this->has_time || this->timeout > 0)) {
			debug_active_sprites = sector->sprites.onTickUpdate(this->camera_x, this->camera_y);
			this->frame_count++;
		}

		Fadetext_Update();

		this->moveBlocks();

		degree = (1 + degree) % 360;

		if (this->button1 > 0)
			this->button1 --;

		if (this->button2 > 0)
			this->button2 --;

		if (this->button3 > 0)
			this->button3 --;

		if (this->info_timer > 0)
			this->info_timer--;

		if (this->score_increment > 0){
			this->score++;
			this->score_increment--;
		}

		if (this->has_time && !this->level_clear) {
			if (this->timeout > 0)
				this->timeout--;
			else
				this->game_over = true;
			
		}
	}
	
	SpriteClass * Player_Sprite = this->playerSprite;

	if (Player_Sprite->energy < 1) {
		this->game_over = true;
	}

	if (this->level_clear || this->game_over) {

		if (this->exit_timer > 1)
			this->exit_timer--;

		if (this->exit_timer == 0)
			this->exit_timer = 700;//800;//2000;

		if (PInput::Keydown(Input->attack1) || PInput::Keydown(Input->attack2) ||
			PInput::Keydown(Input->jump) || Clicked() ||
			TouchScreenControls.any)
			if (this->exit_timer > 2 && this->exit_timer < 500/*600*//*1900*/ && key_delay == 0)
				this->exit_timer = 2;

		if (this->exit_timer == 2) {
			
			Fade_out(FADE_NORMAL);
			if (this->game_over)
				PSound::set_musicvolume(0);
		
		}
	}

	if (key_delay == 0) {
		if (!this->game_over && !this->level_clear) {
			if (PInput::Keydown(Input->open_gift) || TouchScreenControls.gift) {
				Gifts_Use(sector->sprites);
				key_delay = 10;
			}
			
			if (PInput::Keydown(PInput::P)) {
				this->paused = !this->paused;
				key_delay = 20;
			}
			
			if (PInput::Keydown(PInput::DEL) && !this->paused) {
				if(!config_txt.silent_suicide){
					Player_Sprite->damage_taken = Player_Sprite->energy;
					Player_Sprite->damage_taken_type = DAMAGE_SELF_DESTRUCTION;
					Player_Sprite->self_destruction = true;
				}
				else{
					Player_Sprite->energy = 0;
					Player_Sprite->removed = true;
				}
			}

			if (PInput::Keydown(PInput::TAB) || PInput::Keydown(PInput::JOY_GUIDE) || TouchScreenControls.tab){
				Gifts_ChangeOrder();
				key_delay = 10;
			}			
		}			
	}

	if (dev_mode){ //Debug
		if (key_delay == 0) {
			if (PInput::Keydown(PInput::F)) {
				show_fps = !show_fps;
				key_delay = 20;
			}
			if (PInput::Keydown(PInput::Z)) {
				if (this->button1 < this->level.button1_time - 64) this->button1 = this->level.button1_time;
				if (this->button2 < this->level.button2_time - 64) this->button2 = this->level.button2_time;
				if (this->button3 < this->level.button3_time - 64) this->button3 = this->level.button3_time;
				key_delay = 20;
			}
			if (PInput::Keydown(PInput::X)) {
				if (this->button1 > 64) this->button1 = 64;
				if (this->button2 > 64) this->button2 = 64;
				if (this->button3 > 64) this->button3 = 64;
				key_delay = 20;
			}
			if (PInput::Keydown(PInput::T)) {
				Settings.double_speed = !Settings.double_speed;
				key_delay = 20;
			}
			/*if (PInput::Keydown(PInput::G)) {
				Settings.draw_transparent = !Settings.draw_transparent;
				key_delay = 20;
			}*/
			if (PInput::Keydown(PInput::L)) {
				this->keys = 0;
				this->openLocks();
				key_delay = 20;
			}
			if (PInput::Keydown(PInput::K)) {
				this->change_skulls = true;
				key_delay = 20;
			}
			if (PInput::Keydown(PInput::R)) {

				Player_Sprite->energy = 10;
				Player_Sprite->removed = false;
				this->game_over = false;

				double pos_x = 0;
				double pos_y = 0;

				u32 sector_id = 0;

				this->selectStart(pos_x, pos_y, sector_id);
				pos_x += 17;

				this->teleportPlayer(pos_x, pos_y, this->level.sectors[sector_id]);
				
				key_delay = 20;
			}
			if (PInput::Keydown(PInput::END)) {
				key_delay = 20;
				this->finish();
			}
			if (PInput::Keydown(PInput::A)) {
				key_delay = 20;
				if(this->initialPlayerPrototype!=nullptr){

					PrototypeClass* ammo1 = Player_Sprite->ammo1;
					PrototypeClass* ammo2 = Player_Sprite->ammo2;

					Player_Sprite->energy = 10;
					Player_Sprite->transformTo(this->initialPlayerPrototype);
					Effect_Stars(Player_Sprite->x, Player_Sprite->y, COLOR_VIOLET);

					if(Player_Sprite->ammo1==nullptr){
						Player_Sprite->ammo1 = ammo1;
					}

					if(Player_Sprite->ammo2==nullptr){
						Player_Sprite->ammo2 = ammo2;
					}

				}
			}
		}
		if (PInput::Keydown(PInput::U))
			Player_Sprite->b = -10;
		if (PInput::Keydown(PInput::E)) {
			Player_Sprite->energy = 10;
			this->game_over = false;
		} if (PInput::Keydown(PInput::V))
			Player_Sprite->invisible_timer = 3000;
		if (PInput::Keydown(PInput::S)) {
			this->startSupermodeMusic();			
			Player_Sprite->super_mode_timer = 490;
			key_delay = 30;
		}

	}
}


void GameClass::startSupermodeMusic(){
	std::optional<PFile::Path> p = PFilesystem::FindAsset("super.xm", PFilesystem::MUSIC_DIR, ".ogg");
	if(p.has_value()){
		PSound::start_music(*p);
	}
	else{
		PLog::Write(PLog::ERR, "\"super.xm\" not found!");
	}
}

void GameClass::start() {

	if (this->started)
		return;
	
	if(this->lua!=nullptr){
		PK2lua::DestroyGameLuaVM(this->lua);
		this->lua = nullptr;
	}

	Gifts_Clean(); //Reset gifts
	Fadetext_Init(); //Reset fade text
	TouchScreenControls.reset();

	if (this->Open_Map() == 1)
		throw PExcept::PException("Can't load level");

	this->moveBlocks();

	PSound::set_musicvolume(Settings.music_max_volume);

	if(this->lua!=nullptr){
		PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_GAME_STARTED);
	}

	this->started = true;
}

void GameClass::finish() {

	if (this->level_clear)
		return;
	
	this->level_clear = true;

	std::optional<PFile::Path> music_path = PFilesystem::FindAsset("hiscore.xm", PFilesystem::MUSIC_DIR, ".ogg");

	if(!music_path.has_value()){
		throw PExcept::PException("\"hiscore.xm\" not found!");
	}

	if (PSound::start_music(*music_path) == -1){
		PLog::Write(PLog::ERR, "Can't play \"hiscore.xm\"");
	}


	if(!test_level){
		u8 status = Episode->getLevelStatus(this->level_id);

		status |= LEVEL_PASSED;
		if(this->apples_count > 0){
			status |= LEVEL_HAS_BIG_APPLES;
			if(this->apples_got >= this->apples_count){
				status |= LEVEL_ALLAPPLES;
			}
		}

		Episode->updateLevelStatus(this->level_id, status);	
	}
	
	PSound::set_musicvolume_now(Settings.music_max_volume);
}

int GameClass::Open_Map() {

	std::optional<PFile::Path> levelPath = PFilesystem::FindEpisodeAsset(level_file, "");
	if(!levelPath.has_value()){
		throw PExcept::PException("Cannot find the level file: \""+level_file+"\"!");
	}
	level.load(*levelPath, false);

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

	//if (!Episode->use_button_timer) {
	level.button1_time = SWITCH_INITIAL_VALUE;
	level.button2_time = SWITCH_INITIAL_VALUE;
	level.button3_time = SWITCH_INITIAL_VALUE;
	//}

	this->placeSprites();
	this->level.calculateBlockTypes();
	for(LevelSector* sector: this->level.sectors){
		sector->calculateEdges();
	}

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

	this->initialPlayerPrototype = player_prototype;

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

		for (u32 x = 0; x < sector->getWidth(); x++) {
			for (u32 y = 0; y < sector->getHeight(); y++){		

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

void GameClass::selectStart(double& pos_x, double& pos_y, u32& sector) {

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

void GameClass::openLocks() {
	//Put in game
	this->vibration = 90;//60
	PInput::Vibrate(1000);

	showInfo(tekstit->Get_Text(PK_txt.game_locksopen));

	for(LevelSector* sector: this->level.sectors){
		sector->openKeylocks(sector==this->playerSprite->level_sector);
	}

	if(this->lua!=nullptr){
		PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_KEYLOCKS_OPENED);
	}
}

void GameClass::showInfo(const std::string& text) {

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
	
	if(dev_mode && PInput::MouseLeft() && !Settings.touchscreen_mode) {
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


