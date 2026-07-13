// #########################
// Pekka Kana 2
// Copyright (c) 2003 Janne Kivilahti
// #########################
#include "game.hpp"

#include "physics.hpp"
#include "gifts.hpp"
#include "spriteclass.hpp"

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

#include <optional>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_map>


static const char * CHECKPOINT_DIR_NAME = "checkpoint";
static const char * QUICK_SAVE_DIR_NAME = "quicksave";
static const char * QUICK_SAVE_TEMP_DIR_NAME = "quicksave.tmp";
static const char * QUICK_SAVE_BACKUP_DIR_NAME = "quicksave.backup";
static constexpr int QUICK_SAVE_SCHEMA_VERSION = 1;

namespace fs = std::filesystem;

namespace {

bool isCompleteQuickSaveDirectory(const fs::path& directory) {
	return fs::is_directory(directory)
		&& fs::is_regular_file(directory / "level.map")
		&& fs::is_regular_file(directory / "game.dat");
}

nlohmann::json readQuickSavePayload(const fs::path& directory) {
	PFile::RW file = PFile::Path((directory / "game.dat").string()).GetRW2("r");
	nlohmann::json payload = file.readCBOR();
	file.close();
	return payload;
}

QuickSaveInfo quickSaveInfoFromJson(const nlohmann::json& payload) {
	QuickSaveInfo info;
	payload.at("schema_version").get_to(info.schemaVersion);
	payload.at("episode_name").get_to(info.episodeName);
	payload.at("player_name").get_to(info.playerName);
	payload.at("level_file").get_to(info.levelFile);
	payload.at("level_id").get_to(info.levelId);
	// Parse the source fields here as well so metadata inspection rejects a
	// truncated identity before the caller offers the save for loading.
	payload.at("episode_is_zip").get<bool>();
	payload.at("episode_path").get<std::string>();
	payload.at("episode_zipfile").get<std::string>();

	if (info.schemaVersion != QUICK_SAVE_SCHEMA_VERSION) {
		throw std::runtime_error("Unsupported quick-save version");
	}
	if (info.episodeName.empty() || info.levelFile.empty()) {
		throw std::runtime_error("Quick-save identity is incomplete");
	}
	if (!payload.contains("state") || !payload.at("state").is_object()) {
		throw std::runtime_error("Quick-save game state is missing");
	}
	return info;
}

void validateQuickSaveStateShape(const nlohmann::json& state,
	std::size_t sectorCount) {
	state.at("game_over").get<bool>();
	state.at("level_clear").get<bool>();
	state.at("repeating").get<bool>();
	state.at("exit_timer").get<u32>();
	state.at("timeout").get<int>();
	state.at("has_time").get<bool>();
	state.at("frame_count").get<u64>();
	state.at("tiles_animation_counter").get<int>();
	state.at("button_vibration").get<int>();
	state.at("button1").get<u32>();
	state.at("button2").get<u32>();
	state.at("button3").get<u32>();
	state.at("score").get<int>();
	state.at("score_increment").get<int>();
	state.at("apples_count").get<u32>();
	state.at("apples_got").get<u32>();
	state.at("vibration").get<int>();
	state.at("camera_x").get<int>();
	state.at("camera_y").get<int>();
	state.at("dcamera_x").get<double>();
	state.at("dcamera_y").get<double>();
	state.at("dcamera_a").get<double>();
	state.at("dcamera_b").get<double>();
	state.at("paused").get<bool>();
	state.at("gift_cooldown").get<int>();
	state.at("global_animation_degree").get<int>();
	state.at("global_animation_degree_temp").get<int>();
	state.at("music_stopped").get<bool>();
	state.at("keys").get<int>();
	state.at("enemies").get<int>();
	state.at("info_timer").get<int>();
	state.at("info_text").get<std::string>();
	state.at("item_panel_x").get<int>();
	state.at("change_skulls").get<bool>();
	state.at("event1").get<bool>();
	state.at("event2").get<bool>();
	if (!state.at("last_cp_id").is_null()) {
		state.at("last_cp_id").get<std::size_t>();
	}

	const nlohmann::json& levelRuntime = state.at("level_runtime");
	levelRuntime.at("arrows_block_degree").get<int>();
	levelRuntime.at("tiles_animation_timer").get<int>();
	levelRuntime.at("block_animation_frame").get<int>();
	levelRuntime.at("button1_timer").get<u32>();
	levelRuntime.at("button2_timer").get<u32>();
	levelRuntime.at("button3_timer").get<u32>();

	const nlohmann::json& spriteSectors = state.at("sprites");
	if (!spriteSectors.is_array() || spriteSectors.size() != sectorCount) {
		throw std::runtime_error("Quick-save sprite sectors are invalid");
	}
	static const char* requiredSpriteFields[] = {
		"id", "active", "removed", "prototype", "orig_x", "orig_y", "x", "y",
		"a", "b", "flip_x", "flip_y", "jump_timer", "coyote_timer",
		"jump_buffer_timer", "jump_input_held", "can_move_up", "can_move_down",
		"can_move_right", "can_move_left", "edge_on_the_left", "edge_on_the_right",
		"energy", "parent_id", "target_id", "weight", "weight_button", "crouched",
		"damage_timer", "invisible_timer", "super_mode_timer", "charging_timer",
		"attack1_timer", "attack2_timer", "in_water", "swimming",
		"max_speed_available", "hidden", "initial_weight", "damage_taken",
		"damage_taken_type", "enemy", "ammo1", "ammo2", "seen_player_x",
		"seen_player_y", "action_timer", "animation_index", "current_sequence",
		"frame_timer", "mutation_timer", "respawn_timer", "current_command",
		"command_timer", "self_destruction", "initial_update",
		"legacy_indestructible_ammo", "can_collect_bonuses", "can_push_bonuses",
		"original", "player_c"
	};
	for (const nlohmann::json& sectorSprites : spriteSectors) {
		if (!sectorSprites.is_array()) {
			throw std::runtime_error("Quick-save sprite list is invalid");
		}
		for (const nlohmann::json& sprite : sectorSprites) {
			if (!sprite.is_object()) {
				throw std::runtime_error("Quick-save sprite state is invalid");
			}
			for (const char* field : requiredSpriteFields) {
				if (!sprite.contains(field)) {
					throw std::runtime_error("Quick-save sprite state is incomplete");
				}
			}
		}
	}

	const nlohmann::json& progress = state.at("episode_progress");
	progress.at("level_statuses").get<std::vector<int>>();
	progress.at("best_scores").get<std::vector<int>>();
	progress.at("level_files").get<std::vector<std::string>>();
	progress.at("next_level").get<u32>();
	progress.at("completed").get<bool>();
	state.at("gifts");
}

bool isValidQuickSaveDirectory(const fs::path& directory) {
	if (!isCompleteQuickSaveDirectory(directory)) {
		return false;
	}

	try {
		const nlohmann::json payload = readQuickSavePayload(directory);
		quickSaveInfoFromJson(payload);
		const std::size_t sectors = LevelClass::validateVersion15Save(
			PFile::Path((directory / "level.map").string()));
		validateQuickSaveStateShape(payload.at("state"), sectors);
		return true;
	}
	catch (const std::exception&) {
		return false;
	}
}

fs::path findQuickSaveDirectory() {
	const fs::path dataPath = PFilesystem::GetDataPath();
	const fs::path current = dataPath / QUICK_SAVE_DIR_NAME;
	if (isValidQuickSaveDirectory(current)) {
		return current;
	}

	// A backup can remain if the process stopped between the two directory
	// renames used to publish a save. Validate it rather than allowing a corrupt
	// current generation to shadow the last known-good one.
	const fs::path backup = dataPath / QUICK_SAVE_BACKUP_DIR_NAME;
	if (isValidQuickSaveDirectory(backup)) {
		return backup;
	}

	return {};
}

}

GameClass *Game = nullptr;

GameClass::GameClass(int idx) : spritePrototypes(Episode)
{

	this->level_id = idx;
	this->level_file = Episode->getLevelFilename(idx, true);

	if (Episode->getLevelStatus(idx) & LEVEL_PASSED)
	{
		this->repeating = true;
	}
}

GameClass::GameClass(std::string level_file) : spritePrototypes(Episode)
{

	this->repeating = true;

	this->level_file = level_file;

	if (!test_level)
	{
		int level_id_tmp = Episode->findLevelbyFilename(level_file);
		if (level_id_tmp == -1)
		{
			PLog::Write(PLog::FATAL, "PK2", "Couldn't find %s on episode", level_file.c_str());
			throw PExcept::PException("Couldn't find test level on episode");
		}
		this->level_id = level_id_tmp;
	}
	else
	{
		this->level_id = -1;
	}
}

GameClass::~GameClass()
{

	PSound::stop_music();
	PDraw::palette_set(default_palette);
	level.clear();
	this->spritePrototypes.clear();

	if (this->lua != nullptr)
	{
		PK2lua::DestroyGameLuaVM(this->lua);
		this->lua = nullptr;
	}
}

void GameClass::exposePlayerToAIs(){
	if (this->playerSprite != nullptr && this->playerSprite->energy > 0)
	{
		AI_Functions::player_invisible = this->playerSprite;

		if (this->playerSprite->invisible_timer > 0)
		{
			AI_Functions::player = nullptr;
		}
		else
		{
			AI_Functions::player = this->playerSprite;
		}
	}
	else
	{
		AI_Functions::player = nullptr;
		AI_Functions::player_invisible = nullptr;
	}
}


void GameClass::onKeyPressed(const PInput::Key& key){

	const InputSettings& input = Settings.getInput();

	if(this->game_over || this->level_clear){
		if (this->exit_timer > 4)
			this->exit_timer = 4;

	} else {
		if(key==input.open_gift){

			if(this->giftCooldown==0){
				this->gifts.use(this->playerSprite->level_sector->sprites, this->playerSprite);	
				this->giftCooldown = 9;
			}	
		}
		else if(key==input.pauseGame){
			this->paused = !this->paused;
		}
		else if(key==input.commitSuicide && !this->paused){
			if (!config_txt.silent_suicide){
				this->playerSprite->damage_taken = this->playerSprite->energy;
				this->playerSprite->damage_taken_type = DAMAGE_SELF_DESTRUCTION;
				this->playerSprite->self_destruction = true;
			}else{
				this->playerSprite->energy = 0;
				this->playerSprite->removed = true;
			}
		}
		else if(key==input.cycleGifts){
			this->gifts.changeOrder();
		}
	}

	if (dev_mode){

		if(key==PInput::Key(SDL_SCANCODE_F)){
			show_fps = !show_fps;
		}
		else if(key==PInput::Key(SDL_SCANCODE_Z)){
			if (this->button1 < this->level.button1_time - 64)
				this->button1 = this->level.button1_time;
			if (this->button2 < this->level.button2_time - 64)
				this->button2 = this->level.button2_time;
			if (this->button3 < this->level.button3_time - 64)
				this->button3 = this->level.button3_time;
		}
		else if(key==PInput::Key(SDL_SCANCODE_X)){
			if (this->button1 > 64)
				this->button1 = 64;
			if (this->button2 > 64)
				this->button2 = 64;
			if (this->button3 > 64)
				this->button3 = 64;
		}
		else if(key==PInput::Key(SDL_SCANCODE_T)){
			Settings.double_speed = !Settings.double_speed;
		}
		else if(key==PInput::Key(SDL_SCANCODE_L)){
			this->keys = 0;
			this->openLocks();
		}
		else if(key==PInput::Key(SDL_SCANCODE_K)){
			this->change_skulls = true;
		}

		else if(key==PInput::Key(SDL_SCANCODE_R)){
			this->playerSprite->energy = 10;
			this->playerSprite->removed = false;
			this->game_over = false;

			double pos_x = 0;
			double pos_y = 0;

			u32 sector_id = 0;

			this->selectStart(pos_x, pos_y, sector_id);

			if (Episode->legacy_start_offset)
			{
				pos_y -= playerSprite->prototype->height / 2;
			}

			this->teleportPlayer(pos_x, pos_y, this->level.sectors[sector_id]);
		}

		else if(key==PInput::Key(SDL_SCANCODE_END)){
			this->finish();
		}
		else if(key==PInput::Key(SDL_SCANCODE_A)){
			if (this->initialPlayerPrototype != nullptr){
				PrototypeClass *ammo1 = this->playerSprite->ammo1;
				PrototypeClass *ammo2 = this->playerSprite->ammo2;

				this->playerSprite->energy = 10;
				this->playerSprite->transformTo(this->initialPlayerPrototype);
				Effect_Stars(this->playerSprite->x, this->playerSprite->y, COLOR_VIOLET);

				if (this->playerSprite->ammo1 == nullptr)
				{
					this->playerSprite->ammo1 = ammo1;
				}

				if (this->playerSprite->ammo2 == nullptr)
				{
					this->playerSprite->ammo2 = ammo2;
				}
			}
		}

		else if(key==PInput::Key(SDL_SCANCODE_V)){
			if (this->playerSprite->invisible_timer > 1){
				this->playerSprite->invisible_timer = 1;
			} else {
				this->playerSprite->invisible_timer = 3000;
			}
		}

		else if(key==PInput::Key(SDL_SCANCODE_S)){
			if (this->playerSprite->super_mode_timer > 1){
				this->playerSprite->super_mode_timer = 1;
			} else {
				if (Episode->supermode_music) {
					this->startSupermodeMusic();
				}
				this->playerSprite->super_mode_timer = 3000;
			}
		}
	}
}

void GameClass::update(int &debug_active_sprites)
{
	this->exposePlayerToAIs();

	if(dev_mode){

		const InputSettings& input = Settings.getInput();

		if(input.dev_fly.isPressed()){
			this->playerSprite->b = -10;
		}

		if(input.dev_heal.isPressed() && !config_txt.hardcore_mode){

			PrototypeClass* playerProto = this->playerSprite->prototype;

			int energy = playerProto->energy;
			if(playerProto->hasAI(AI_TRANSFORM_WHEN_ENERGY_OVER_1) && playerProto->transformation!=nullptr){
				energy = playerProto->transformation->energy;
			}


			this->playerSprite->energy = energy;
			this->game_over = false;
			this->exit_timer = 0;
		}
	}


	if(Settings.touchscreen_mode){

		static bool useGiftWasPressed = false;
		if(TouchScreenControls.gift && !useGiftWasPressed){
			this->gifts.use(this->playerSprite->level_sector->sprites, this->playerSprite);				
		}
		useGiftWasPressed = TouchScreenControls.gift;


		static bool cycleGiftsWasPressed = false;
		if(TouchScreenControls.tab && !cycleGiftsWasPressed){
			this->gifts.changeOrder();
		}
		cycleGiftsWasPressed = TouchScreenControls.tab;
	}

	if (!this->level_clear && (!this->has_time || this->timeout > 0))
	{
		this->level.setTilesAnimations(degree, this->tiles_animation_counter / 7, this->button1, this->button2, this->button3);
		this->tiles_animation_counter = 1 + this->tiles_animation_counter % 34;
	}

	this->updateCamera();
	Update_GameSFX();

	/***
	 * Execute events
	 */

	if (this->change_skulls)
	{
		this->change_skulls = false;

		this->vibrate(90);

		for (LevelSector *sector : this->level.sectors)
		{
			sector->changeSkulls(sector == this->playerSprite->level_sector);
			sector->sprites.onSkullBlocksChanged();
		}

		if (this->lua != nullptr)
		{
			PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_SKULL_BLOCKS_CHANGED);
		}
	}

	if (this->event1)
	{
		this->event1 = false;

		this->vibrate(90);

		for (LevelSector *sector : this->level.sectors)
		{
			sector->sprites.onEvent1();
		}

		if (this->lua != nullptr)
		{
			PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_1);
		}
	}

	if (this->event2)
	{
		this->event2 = false;

		for (LevelSector *sector : this->level.sectors)
		{
			sector->sprites.onEvent2();
		}

		if (this->lua != nullptr)
		{
			PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_2);
		}
	}

	if (!this->paused)
	{

		if(this->giftCooldown>0){
			--this->giftCooldown;
		}

		/**
		 * @brief
		 * Kill all the enemies mode
		 */

		if (this->level.game_mode == GAME_MODE_KILL_ALL &&
			this->enemies <= 0 &&
			this->playerSprite->damage_taken == 0 &&
			this->playerSprite->damage_timer == 0 &&
			this->playerSprite->energy > 0)
		{

			this->finish();
		}

		/**
		 * @brief
		 * Update Lua
		 */

		if (this->lua != nullptr)
		{
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

		if (!this->level_clear && (!this->has_time || this->timeout > 0))
		{
			debug_active_sprites = this->playerSprite->level_sector->sprites.onTickUpdate(this->camera_x, this->camera_y);
			this->frame_count++;
		}

		Fadetext_Update();

		this->moveBlocks();

		degree = (1 + degree) % 360;

		if (this->button1 > 0)
			this->button1--;

		if (this->button2 > 0)
			this->button2--;

		if (this->button3 > 0)
			this->button3--;

		if (this->info_timer > 0)
			this->info_timer--;

		if (this->score_increment > 0)
		{
			this->score++;
			this->score_increment--;
		}

		if (this->has_time && !this->level_clear)
		{
			if (this->timeout > 0)
				this->timeout--;
			else
				this->game_over = true;
		}
	}

	//SpriteClass *Player_Sprite = this->playerSprite;

	if (this->playerSprite->energy < 1)
	{
		this->game_over = true;
	}

	if (this->level_clear || this->game_over) {

		if (this->exit_timer == 0){
			this->exit_timer = 700; // 800;//2000;
		}
		else if (this->exit_timer > 1){
			this->exit_timer--;

			if(this->exit_timer==2) {
				if (this->game_over) {

					if (this->lastCheckpoint != nullptr && this->score >= Episode->checkpointPenalty && !config_txt.hardcore_mode) {

						fs::path dataPath = PFilesystem::GetDataPath();
						fs::path p3 = dataPath / CHECKPOINT_DIR_NAME / "score.dat";

						/**
						 * All the points collected after activating the checkpoint are cancelled.
						 * To prevent points farming by dying
						 */

						PFile::RW in = PFile::Path(p3.string()).GetRW2("r");
						in.read(this->score);
						in.close();
						this->score -= Episode->checkpointPenalty;

						/**
						 * To prevent negative scores
						 */
						if(this->score < 0){
							this->score = 0;
						}

						PFile::RW out = PFile::Path(p3.string()).GetRW2("w");
						out.write(this->score);
						out.close();

						/**
						 * For centering the player's position
						 */
						int checkpoint_x = this->lastCheckpoint->x;
						int checkpoint_y = this->lastCheckpoint->y;

						Fade_in(FADE_NORMAL);
						this->game_over = false;
						this->exit_timer = 0;

						int t_score = this->score;
						this->loadGameState();
						if(this->score > t_score)this->score = t_score;

						SpriteClass*player = this->playerSprite;
						if(player->player_c==1){						
							player->a = 0;
							player->b = 0;

							player->x = checkpoint_x;
							player->y = checkpoint_y;
						}

						LevelSector * sector = player->level_sector;
						// Change palette
						sector->background->setPalette();

						// Change weather
						BG_Particles::Init(sector->weather, sector->rain_color);
						Particles_Clear();

						sector->startMusic();
						// Change GFX texture
						this->gfxTexture = sector->gfxTexture;

					} else {
						Fade_out(FADE_NORMAL);
						PSound::set_musicvolume(0);
					}
				} else {
					Fade_out(FADE_NORMAL);
				}
			}
		}
	}

	
}

void GameClass::startSupermodeMusic()
{
	std::optional<PFile::Path> p = PFilesystem::FindAsset("super.xm", PFilesystem::MUSIC_DIR, ".ogg");
	if (p.has_value())
	{
		PSound::start_music(*p);
	}
	else
	{
		PLog::Write(PLog::ERR, "\"super.xm\" not found!");
	}
}

void GameClass::start(bool initializeLua)
{
	if (this->started)
		return;

	if (initializeLua && this->lua != nullptr)
	{
		PK2lua::DestroyGameLuaVM(this->lua);
		this->lua = nullptr;
	}

	Fadetext_Init(); // Reset fade text
	TouchScreenControls.reset();	
	
	std::optional<PFile::Path> levelPath = PFilesystem::FindEpisodeAsset(level_file, "");
	if (!levelPath.has_value())
	{
		throw PExcept::PException("Cannot find the level file: \"" + level_file + "\"!");
	}
	level.load(*levelPath, false);

	/**
	 * @brief
	 * Load lua
	 */
	if (initializeLua && this->level.lua_script != "")
	{
		this->lua = PK2lua::CreateGameLuaVM(this->level.lua_script);
	}
	else if (initializeLua)
	{
		PLog::Write(PLog::INFO, "PK2lua", "No Lua scripting in this level");
	}

	this->timeout = level.map_time * TIME_FPS;

	if (timeout > 0)
		this->has_time = true;
	else
		this->has_time = false;

	this->level.button1_time = SWITCH_INITIAL_VALUE;
	this->level.button2_time = SWITCH_INITIAL_VALUE;
	this->level.button3_time = SWITCH_INITIAL_VALUE;
	this->moveBlocks();

	this->placeSprites();

	Particles_Clear();

	LevelSector *sector = this->playerSprite->level_sector;

	BG_Particles::Init(sector->weather, sector->rain_color);
	sector->startMusic();

	//this->moveBlocks();

	PSound::set_musicvolume(Settings.music_max_volume);

	if (this->lua != nullptr)
	{
		PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_GAME_STARTED);
	}

	this->started = true;
}

void GameClass::finish()
{

	if (this->level_clear)
		return;

	this->level_clear = true;

	std::optional<PFile::Path> music_path = PFilesystem::FindAsset("hiscore.xm", PFilesystem::MUSIC_DIR, ".ogg");

	if (!music_path.has_value())
	{
		throw PExcept::PException("\"hiscore.xm\" not found!");
	}

	if (PSound::start_music(*music_path) == -1)
	{
		PLog::Write(PLog::ERR, "Can't play \"hiscore.xm\"");
	}

	if (!test_level)
	{
		u8 status = Episode->getLevelStatus(this->level_id);

		status |= LEVEL_PASSED;
		if (this->apples_count > 0)
		{
			status |= LEVEL_HAS_BIG_APPLES;
			if (this->apples_got >= this->apples_count)
			{
				status |= LEVEL_ALLAPPLES;
			}
		}

		Episode->updateLevelStatus(this->level_id, status);
	}

	PSound::set_musicvolume_now(Settings.music_max_volume);
}

void GameClass::placeSprites()
{
	std::array<PrototypeClass *, 255> mapping;

	// Load prototypes
	std::size_t prototypes_number = this->level.sprite_prototype_names.size();
	if (prototypes_number > mapping.size())
	{
		std::ostringstream os;
		os << "Too many sprite prototypes: " << prototypes_number << std::endl;
		os << mapping.size() << " is the current limit." << std::endl;
		os << "Dependency sprites (ammo, transformation and so on)"
			  " do not count into the limit";

		throw std::runtime_error(os.str());
	}

	for (std::size_t i = 0; i < mapping.size(); ++i)
	{
		mapping[i] = nullptr;
	}

	for (std::size_t i = 0; i < prototypes_number; ++i)
	{
		const std::string &name = level.sprite_prototype_names[i];
		if (!name.empty())
		{
			mapping[i] = this->spritePrototypes.loadPrototype(level.sprite_prototype_names[i]);
		}
	}

	// Load sprite assets
	this->spritePrototypes.loadSpriteAssets();

	// Player prototype
	int player_index = level.player_sprite_index;
	if (player_index < 0 || player_index >= int(mapping.size()))
	{
		std::ostringstream os;
		os << "Incorrect player sprite index: " << player_index;
		throw PExcept::PException(os.str());
	}

	PrototypeClass *player_prototype = mapping[player_index];
	if (player_prototype == nullptr)
	{
		throw PExcept::PException("Null player prototype is quite serious error!");
	}

	this->initialPlayerPrototype = player_prototype;

	// Add player
	{
		double pos_x = 0;
		double pos_y = 0;
		u32 sector_id = 0;

		this->selectStart(pos_x, pos_y, sector_id);
		this->playerSprite = this->level.sectors[sector_id]->sprites.addPlayer(player_prototype, pos_x, pos_y);

		if (Episode->legacy_start_offset)
		{
			this->playerSprite->y -= player_prototype->height / 2;
		}

		this->level.sectors[sector_id]->background->setPalette();
		this->setCamera(Episode->legacy_camera_offset);
	}

	// set GFX
	this->gfxTexture = this->playerSprite->level_sector->gfxTexture;

	// Add other sprites
	for (LevelSector *sector : this->level.sectors)
	{

		for (u32 x = 0; x < sector->getWidth(); x++)
		{
			for (u32 y = 0; y < sector->getHeight(); y++)
			{

				int sprite = sector->sprite_tiles[x + y * sector->getWidth()];
				if (sprite < 0 || sprite >= 255)
					continue;

				PrototypeClass *prototype = mapping[sprite];
				if (prototype == nullptr)
					continue;

				/**
				 * @brief
				 * Count big apples
				 */
				if (prototype->big_apple)
				{
					this->apples_count++;
				}

				/**
				 * @brief
				 * Count big apples in boxes
				 */
				if (prototype->bonus != nullptr && prototype->bonus->big_apple && prototype->bonus_always)
				{
					this->apples_count += prototype->bonuses_number;
				}

				/**
				 * @brief
				 * Count keys
				 */
				if (prototype->can_open_locks &&
					!prototype->indestructible)
				{
					this->keys++;
				}

				/**
				 * @brief
				 * Count enemies
				 */
				if (prototype->type == TYPE_GAME_CHARACTER && !prototype->indestructible && prototype->damage > 0 // to ignore switches, boxes and so on
					&& prototype->enemy)
				{
					this->enemies++;
				}

				sector->sprites.addLevelSprite(prototype, x * 32, y * 32 - prototype->height + 32);
			}
		}

		sector->sprites.sortBg();
	}
}

void GameClass::selectStart(double &pos_x, double &pos_y, u32 &sector)
{

	pos_x = 320;
	pos_y = 196;

	std::vector<BlockPosition> startSigns;

	for (u32 i = 0; i < level.sectors.size(); ++i)
	{
		level.sectors[i]->countStartSigns(startSigns, i);
	}

	int selected_start = 0;
	if (startSigns.size() > 1)
	{
		selected_start = rand() % startSigns.size();
	}

	if (startSigns.size() > 0)
	{
		pos_x = startSigns[selected_start].x * 32 + 17;
		pos_y = startSigns[selected_start].y * 32;
		sector = startSigns[selected_start].sector;
	}
}

SpriteClass *GameClass::selectTeleporter(SpriteClass *entryTelporter, PrototypeClass *exitPrototype)
{
	std::vector<SpriteClass *> teleporters;
	for (LevelSector *sector : this->level.sectors)
	{
		for (SpriteClass *sprite : sector->sprites.Sprites_List)
		{
			if (sprite->prototype == exitPrototype && sprite != entryTelporter)
			{
				teleporters.push_back(sprite);
			}
		}
	}

	if (teleporters.size() == 0)
		return nullptr;
	else if (teleporters.size() == 1)
		return teleporters[0];
	else
		return teleporters[rand() % teleporters.size()];
}

void GameClass::teleportPlayer(double x, double y, LevelSector *sector)
{
	this->playerSprite->x = x;
	this->playerSprite->y = y;

	/**
	 * @brief
	 * Change sector
	 */
	if (sector != nullptr && this->playerSprite->level_sector != sector)
	{

		LevelSector *previous_sector = this->playerSprite->level_sector;
		previous_sector->sprites.Sprites_List.remove(this->playerSprite);

		sector->sprites.Sprites_List.push_front(this->playerSprite);
		this->playerSprite->level_sector = sector;

		// Change palette
		sector->background->setPalette();

		// Change weather
		BG_Particles::Init(sector->weather, sector->rain_color);
		Particles_Clear();

		// Change music
		if (sector->music_name != previous_sector->music_name)
		{
			sector->startMusic();
		}

		// Change GFX texture
		this->gfxTexture = sector->gfxTexture;
	}

	Fade_in(FADE_NORMAL);

	this->setCamera();
}

void GameClass::openLocks()
{
	this->vibrate(90);

	showInfo(tekstit->Get_Text(PK_txt.game_locksopen));

	for (LevelSector *sector : this->level.sectors)
	{
		sector->openKeylocks(sector == this->playerSprite->level_sector);
	}

	if (this->lua != nullptr)
	{
		PK2lua::TriggerEventListeners(PK2lua::LUA_EVENT_KEYLOCKS_OPENED);
	}
}

void GameClass::showInfo(const std::string &text)
{

	if (info_text.compare(text) != 0 || info_timer == 0)
	{

		info_text = text;
		info_timer = INFO_TIME;
	}
}

void GameClass::drawInfoText()
{
	if (this->info_timer > 0)
	{
		std::pair<int, int> box_size = PDraw::font_get_text_size(fontti1, this->info_text);

		box_size.first += 8;
		box_size.second += 8;

		PDraw::RECT infoBG(screen_width / 2 - (box_size.first / 2), 60,
						   screen_width / 2 + (box_size.first / 2), 60 + box_size.second);

		int tmp = (box_size.second - this->info_timer) / 2;

		if (tmp > 0)
		{

			infoBG.y += tmp;
			infoBG.h -= tmp;
		}
		else if (this->info_timer > INFO_TIME - box_size.second)
		{
			// int tmp = 10 - (INFO_TIME - this->info_timer) / 2;
			tmp = (box_size.second - INFO_TIME + this->info_timer) / 2;

			infoBG.y += tmp;
			infoBG.h -= tmp;
		}

		PDraw::screen_fill(infoBG.x - 1, infoBG.y - 1, infoBG.w + 1, infoBG.h + 1, 51);
		PDraw::screen_fill(infoBG.x, infoBG.y, infoBG.w, infoBG.h, 38);

		// tmp = this->info_timer - 11
		tmp = this->info_timer - 1 - box_size.second;

		if (tmp >= 100)
			PDraw::font_write(fontti1, this->info_text, infoBG.x + 4, infoBG.y + 4);
		else if (tmp > 0)
			PDraw::font_writealpha_s(fontti1, this->info_text, infoBG.x + 4, infoBG.y + 4, tmp);
	}
}

void GameClass::setCamera(bool legacy_mode)
{

	LevelSector *sector = this->playerSprite->level_sector;

	if (legacy_mode)
	{
		this->camera_x = (int)this->playerSprite->x;
		this->camera_y = (int)this->playerSprite->y;
	}
	else
	{
		this->camera_x = (int)this->playerSprite->x - screen_width / 2;
		this->camera_y = (int)this->playerSprite->y - screen_height / 2;
	}

	const int maxCameraX = std::max(0, int(sector->getWidth() * 32) - screen_width);
	const int maxCameraY = std::max(0, int(sector->getHeight() * 32) - screen_height);
	this->camera_x = std::clamp(this->camera_x, 0, maxCameraX);
	this->camera_y = std::clamp(this->camera_y, 0, maxCameraY);

	this->dcamera_x = this->camera_x;
	this->dcamera_y = this->camera_y;

	this->dcamera_a = 0;
	this->dcamera_b = 0;
}

void GameClass::vibrate(int vibration){
	this->vibration = vibration;
	Settings.vibrateController(vibration);
}

void GameClass::updateCamera()
{
	LevelSector *sector = this->playerSprite->level_sector;
	const bool legacyCamera = Episode->legacy_camera_offset;

	double targetX = this->playerSprite->x - screen_width / 2.0;
	double targetY = this->playerSprite->y - screen_height / 2.0;

	if (!legacyCamera) {
		// Lead gently into motion so the player can see more of what they are
		// approaching. The bounded instantaneous velocity avoids stale momentum
		// when the player turns around or lands.
		const double maxLookAheadX = screen_width * 0.16;
		const double maxLookAheadY = screen_height * 0.08;
		targetX += std::clamp(this->playerSprite->a * 18.0,
			-maxLookAheadX, maxLookAheadX);
		targetY += std::clamp(this->playerSprite->b * 8.0,
			-maxLookAheadY, maxLookAheadY);
	}

	if (dev_mode && PInput::Key::MOUSE_LEFT.isPressed() && !Settings.touchscreen_mode){

		const Point2D& mousePos = PInput::InputSystem::instance().getMousePos();
		targetX += mousePos.x - screen_width / 2;
		targetY += mousePos.y - screen_height / 2;
	}

	const double maxCameraX = std::max(0.0, sector->getWidth() * 32.0 - screen_width);
	const double maxCameraY = std::max(0.0, sector->getHeight() * 32.0 - screen_height);
	targetX = std::clamp(targetX, 0.0, maxCameraX);
	targetY = std::clamp(targetY, 0.0, maxCameraY);

	if (legacyCamera) {
		this->dcamera_a = std::clamp((targetX - this->dcamera_x) / 15.0, -6.0, 6.0);
		this->dcamera_b = std::clamp((targetY - this->dcamera_y) / 15.0, -6.0, 6.0);
	}
	else {
		// Exponential tracking is frame-stable, monotonic, and cannot overshoot.
		constexpr double CAMERA_EASING = 0.14;
		this->dcamera_a = (targetX - this->dcamera_x) * CAMERA_EASING;
		this->dcamera_b = (targetY - this->dcamera_y) * CAMERA_EASING;
	}

	this->dcamera_x = std::clamp(this->dcamera_x + this->dcamera_a, 0.0, maxCameraX);
	this->dcamera_y = std::clamp(this->dcamera_y + this->dcamera_b, 0.0, maxCameraY);

	double shakeX = 0;
	double shakeY = 0;
	if (this->vibration > 0)
	{
		shakeX += (rand() % this->vibration - rand() % this->vibration) / 5.0;
		shakeY += (rand() % this->vibration - rand() % this->vibration) / 5.0;

		this->vibration--;
	}

	if (this->button_vibration > 0)
	{
		shakeX += rand() % 9 - rand() % 9;
		shakeY += rand() % 9 - rand() % 9;

		this->button_vibration--;
	}

	this->camera_x = std::clamp(int(std::lround(this->dcamera_x + shakeX)),
		0, int(maxCameraX));
	this->camera_y = std::clamp(int(std::lround(this->dcamera_y + shakeY)),
		0, int(maxCameraY));
}


void GameClass::saveGameState()const{

	PLog::Write(PLog::INFO, "PK2", "Saving checkpoint...");

	fs::path dataPath = PFilesystem::GetDataPath();
	fs::path p1 = dataPath / CHECKPOINT_DIR_NAME / "level.map";
	fs::path p2 = dataPath / CHECKPOINT_DIR_NAME / "game.dat";
	fs::path p3 = dataPath / CHECKPOINT_DIR_NAME / "score.dat";

	this->level.saveVersion15(PFile::Path(p1.string()));

	PFile::RW file2 = PFile::Path(p2.string()).GetRW2("w");
	file2.writeCBOR(this->toJson());
	file2.close();
	
	PLog::Write(PLog::DEBUG, "PK2", "Checkpoint saved!");

	PFile::RW file3 = PFile::Path(p3.string()).GetRW2("w");
	file3.write(this->score);
	file3.close();
}

void GameClass::loadGameState(){

	PLog::Write(PLog::INFO, "PK2", "Loading checkpoint...");

	fs::path dataPath = PFilesystem::GetDataPath();
	fs::path p1 = dataPath / CHECKPOINT_DIR_NAME / "level.map";
	fs::path p2 = dataPath / CHECKPOINT_DIR_NAME / "game.dat";
	fs::path p3 = dataPath / CHECKPOINT_DIR_NAME / "score.dat";


	this->level.clearSectors();
	this->level.load(PFile::Path(p1.string()), false);
	this->lastCheckpoint = nullptr;
	

	PFile::RW file = PFile::Path(p2.string()).GetRW2("r");

	this->fromJson(file.readCBOR());
	file.close();

	this->info_timer = 0;
	PLog::Write(PLog::DEBUG, "PK2", "Checkpoint loaded!");
}

std::optional<QuickSaveInfo> GameClass::getQuickSaveInfo() {
	const fs::path directory = findQuickSaveDirectory();
	if (directory.empty()) {
		return {};
	}

	return quickSaveInfoFromJson(readQuickSavePayload(directory));
}

void GameClass::saveQuickGameState() const {
	if (Episode == nullptr || this->playerSprite == nullptr || this->level.sectors.empty()) {
		throw std::runtime_error("The game is not ready to quick-save");
	}

	PLog::Write(PLog::INFO, "PK2", "Saving quick-save...");

	const fs::path dataPath = PFilesystem::GetDataPath();
	const fs::path destination = dataPath / QUICK_SAVE_DIR_NAME;
	const fs::path backup = dataPath / QUICK_SAVE_BACKUP_DIR_NAME;

	// A unique sibling avoids two game processes writing into the same staging
	// directory. create_directory is the atomic claim operation.
	fs::path temporary;
	const auto nonce = std::chrono::high_resolution_clock::now()
		.time_since_epoch().count();
	for (int attempt = 0; attempt < 100; ++attempt) {
		temporary = dataPath / (std::string(QUICK_SAVE_TEMP_DIR_NAME) + "."
			+ std::to_string(nonce) + "." + std::to_string(attempt));
		std::error_code createError;
		if (fs::create_directory(temporary, createError)) {
			break;
		}
		if (createError) {
			throw std::runtime_error("Could not prepare the quick-save directory: "
				+ createError.message());
		}
		temporary.clear();
	}
	if (temporary.empty() || !fs::is_directory(temporary)) {
		throw std::runtime_error("Could not reserve a quick-save directory");
	}

	try {
		this->level.saveVersion15(PFile::Path((temporary / "level.map").string()));

		nlohmann::json payload;
		payload["schema_version"] = QUICK_SAVE_SCHEMA_VERSION;
		payload["episode_name"] = Episode->entry.name;
		payload["episode_is_zip"] = Episode->entry.is_zip;
		payload["episode_path"] = Episode->entry.path;
		payload["episode_zipfile"] = Episode->entry.zipfile;
		payload["player_name"] = Episode->player_name;
		payload["level_file"] = this->level_file;
		payload["level_id"] = this->level_id;
		payload["state"] = this->toQuickJson();

		PFile::RW stateFile = PFile::Path((temporary / "game.dat").string()).GetRW2("w");
		stateFile.writeCBOR(payload);
		stateFile.close();

		if (!isValidQuickSaveDirectory(temporary)) {
			throw std::runtime_error("Quick-save files were not written completely");
		}
	}
	catch (...) {
		std::error_code ignored;
		fs::remove_all(temporary, ignored);
		throw;
	}

	// Publish both files together. Never discard the backup unless the current
	// generation has itself been validated, so a failed Windows rename always
	// leaves at least one discoverable complete save.
	const bool validDestination = isValidQuickSaveDirectory(destination);
	const bool validBackup = isValidQuickSaveDirectory(backup);
	try {
		if (fs::exists(destination) && !validDestination) {
			fs::remove_all(destination);
		}

		if (validDestination) {
			if (fs::exists(backup)) {
				fs::remove_all(backup);
			}
			fs::rename(destination, backup);
		}
		else if (!validBackup && fs::exists(backup)) {
			fs::remove_all(backup);
		}

		fs::rename(temporary, destination);
	}
	catch (...) {
		if (validDestination && !fs::exists(destination)
			&& isValidQuickSaveDirectory(backup)) {
			std::error_code restoreError;
			fs::rename(backup, destination, restoreError);
		}
		std::error_code ignored;
		fs::remove_all(temporary, ignored);
		throw;
	}

	std::error_code cleanupError;
	fs::remove_all(backup, cleanupError);
	if (cleanupError) {
		PLog::Write(PLog::WARN, "PK2", "Could not remove quick-save backup: %s",
			cleanupError.message().c_str());
	}

	PLog::Write(PLog::DEBUG, "PK2", "Quick-save saved!");
}

void GameClass::loadQuickGameState(bool initializeLua) {
	if (Episode == nullptr || !this->started) {
		throw std::runtime_error("The game is not ready to quick-load");
	}

	const fs::path directory = findQuickSaveDirectory();
	if (directory.empty()) {
		throw std::runtime_error("No quick save found");
	}

	PLog::Write(PLog::INFO, "PK2", "Loading quick-save...");
	nlohmann::json payload = readQuickSavePayload(directory);
	const QuickSaveInfo info = quickSaveInfoFromJson(payload);

	if (info.episodeName != Episode->entry.name) {
		throw std::runtime_error("Quick save is from another episode");
	}
	if (payload.at("episode_is_zip").get<bool>() != Episode->entry.is_zip
		|| payload.at("episode_path").get<std::string>() != Episode->entry.path
		|| payload.at("episode_zipfile").get<std::string>() != Episode->entry.zipfile) {
		throw std::runtime_error("Quick save is from another episode source");
	}
	if (info.playerName != Episode->player_name) {
		throw std::runtime_error("Quick save belongs to another player");
	}
	if (info.levelId != this->level_id || info.levelFile != this->level_file) {
		throw std::runtime_error("Quick save is from another level");
	}

	const nlohmann::json& state = payload.at("state");
	const nlohmann::json& progress = state.at("episode_progress");
	const std::vector<int> levelStatuses =
		progress.at("level_statuses").get<std::vector<int>>();
	const std::vector<int> bestScores =
		progress.at("best_scores").get<std::vector<int>>();
	const std::vector<std::string> savedLevelFiles =
		progress.at("level_files").get<std::vector<std::string>>();
	const u32 savedNextLevel = progress.at("next_level").get<u32>();
	const bool savedCompleted = progress.at("completed").get<bool>();
	if (levelStatuses.size() != Episode->getLevelsNumber()
		|| bestScores.size() != Episode->getLevelsNumber()
		|| savedLevelFiles.size() != Episode->getLevelsNumber()) {
		throw std::runtime_error("Quick-save episode progress does not match this episode");
	}
	for (std::size_t i = 0; i < savedLevelFiles.size(); ++i) {
		if (savedLevelFiles[i] != Episode->getLevelEntries()[i].fileName) {
			throw std::runtime_error("Quick-save episode levels have changed");
		}
	}
	for (int status : levelStatuses) {
		if (status < 0 || status > 255) {
			throw std::runtime_error("Quick-save episode progress is invalid");
		}
	}

	if (!initializeLua && this->lua != nullptr) {
		throw std::runtime_error("Cannot defer an already initialized Lua state");
	}

	if (initializeLua && this->lua != nullptr) {
		PK2lua::DestroyGameLuaVM(this->lua);
		this->lua = nullptr;
	}

	try {
		this->level.clearSectors();
		this->level.load(PFile::Path((directory / "level.map").string()), false);
		this->lastCheckpoint = nullptr;
		this->fromJson(state, true);

		if (initializeLua && !this->level.lua_script.empty()) {
			this->lua = PK2lua::CreateGameLuaVM(this->level.lua_script);
		}
	}
	catch (...) {
		if (initializeLua && this->lua != nullptr) {
			PK2lua::DestroyGameLuaVM(this->lua);
			this->lua = nullptr;
		}
		throw;
	}

	// Restoring progression is intentionally in-memory only. Normal save slots
	// are still written only by their existing explicit workflow.
	for (std::size_t i = 0; i < levelStatuses.size(); ++i) {
		Episode->updateLevelStatus(int(i), u8(levelStatuses[i]));
		Episode->updateLevelBestScore(int(i), bestScores[i]);
	}
	Episode->next_level = savedNextLevel;
	Episode->completed = savedCompleted;

	PLog::Write(PLog::DEBUG, "PK2", "Quick-save loaded!");
}

void GameClass::refreshPresentationState() {
	Fadetext_Init();
	Particles_Clear();

	if (this->playerSprite == nullptr || this->playerSprite->level_sector == nullptr) {
		return;
	}

	LevelSector* sector = this->playerSprite->level_sector;
	sector->background->setPalette();
	BG_Particles::Init(sector->weather, sector->rain_color);
	this->gfxTexture = sector->gfxTexture;
	if (this->playerSprite->super_mode_timer > 0 && Episode->supermode_music) {
		this->startSupermodeMusic();
	}
	else {
		sector->startMusic();
	}
	PSound::set_musicvolume_now(this->music_stopped ? 0 : Settings.music_max_volume);
	this->exposePlayerToAIs();
}

void GameClass::restartLuaForCurrentState() {
	if (this->lua != nullptr) {
		PK2lua::DestroyGameLuaVM(this->lua);
		this->lua = nullptr;
	}

	if (!this->level.lua_script.empty()) {
		this->lua = PK2lua::CreateGameLuaVM(this->level.lua_script);
	}
}


nlohmann::json GameClass::toJson() const
{
	nlohmann::json j;
	j["game_over"] = this->game_over;
	j["level_clear"] = this->level_clear;
	j["repeating"] = this->repeating;
	j["exit_timer"] = this->exit_timer;
	j["timeout"] = this->timeout;
	j["has_time"] = this->has_time;
	j["frame_count"] = this->frame_count;
	j["tiles_animation_counter"] = this->tiles_animation_counter;
	j["button_vibration"] = this->button_vibration;
	j["button1"] = this->button1;
	j["button2"] = this->button2;
	j["button3"] = this->button3;
	j["score"] = this->score + this->score_increment;
	//j["score_increment"] = this->score_increment;
	j["apples_count"] = this->apples_count;
	j["apples_got"] = this->apples_got;
	j["vibration"] = this->vibration;
	/*j["camera_x"] = this->camera_x;
	j["camera_y"] = this->camera_y;
	j["dcamera_x"] = this->dcamera_x;
	j["dcamera_y"] = this->dcamera_y;
	j["dcamera_a"] = this->dcamera_a;
	j["dcamera_b"] = this->dcamera_b;
	j["paused"] = this->paused;*/
	j["music_stopped"] = this->music_stopped;
	j["keys"] = this->keys;
	j["enemies"] = this->enemies;
	j["info_timer"] = this->info_timer;
	j["info_text"] = this->info_text;
	j["item_panel_x"] = this->item_panel_x;
	j["change_skulls"] = this->change_skulls;
	j["event1"] = this->event1;
	j["event2"] = this->event2;

	if(this->lastCheckpoint==nullptr){
		j["last_cp_id"] = nullptr;
	}
	else{
		j["last_cp_id"] = this->lastCheckpoint->id;
	}

	std::vector<nlohmann::json> sprites;
	for (const LevelSector *sector : this->level.sectors)
	{
		sprites.emplace_back(sector->sprites.toJson());
	}

	j["sprites"] = sprites;
	j["gifts"] = this->gifts.toJson();

	return j;
}

nlohmann::json GameClass::toQuickJson() const
{
	nlohmann::json j = this->toJson();

	// Checkpoints intentionally consolidate a pending score animation. A quick
	// save instead preserves both values so the HUD and score resume exactly.
	j["score"] = this->score;
	j["score_increment"] = this->score_increment;
	j["camera_x"] = this->camera_x;
	j["camera_y"] = this->camera_y;
	j["dcamera_x"] = this->dcamera_x;
	j["dcamera_y"] = this->dcamera_y;
	j["dcamera_a"] = this->dcamera_a;
	j["dcamera_b"] = this->dcamera_b;
	j["paused"] = this->paused;
	j["gift_cooldown"] = this->giftCooldown;
	j["global_animation_degree"] = degree;
	j["global_animation_degree_temp"] = degree_temp;
	j["level_runtime"] = this->level.runtimeStateToJson();

	std::vector<nlohmann::json> sprites;
	for (const LevelSector* sector : this->level.sectors) {
		sprites.emplace_back(sector->sprites.toJson(true));
	}
	j["sprites"] = sprites;

	nlohmann::json episodeProgress;
	std::vector<int> statuses;
	std::vector<int> bestScores;
	std::vector<std::string> levelFiles;
	statuses.reserve(Episode->getLevelsNumber());
	bestScores.reserve(Episode->getLevelsNumber());
	levelFiles.reserve(Episode->getLevelsNumber());
	for (const LevelEntry& entry : Episode->getLevelEntries()) {
		statuses.push_back(int(entry.status));
		bestScores.push_back(entry.best_score);
		levelFiles.push_back(entry.fileName);
	}
	episodeProgress["level_statuses"] = statuses;
	episodeProgress["best_scores"] = bestScores;
	episodeProgress["level_files"] = levelFiles;
	episodeProgress["next_level"] = Episode->next_level;
	episodeProgress["completed"] = Episode->completed;
	j["episode_progress"] = episodeProgress;

	return j;
}

void GameClass::fromJson(const nlohmann::json &j, bool restoreExactState)
{
	// Restore basic game state
	j.at("game_over").get_to(this->game_over);
	j.at("level_clear").get_to(this->level_clear);
	j.at("repeating").get_to(this->repeating);
	j.at("exit_timer").get_to(this->exit_timer);
	j.at("timeout").get_to(this->timeout);
	j.at("has_time").get_to(this->has_time);
	j.at("frame_count").get_to(this->frame_count);
	j.at("tiles_animation_counter").get_to(this->tiles_animation_counter);
	j.at("button_vibration").get_to(this->button_vibration);
	j.at("button1").get_to(this->button1);
	j.at("button2").get_to(this->button2);
	j.at("button3").get_to(this->button3);
	j.at("score").get_to(this->score);
	if (restoreExactState) {
		j.at("score_increment").get_to(this->score_increment);
	}
	else {
		this->score_increment = 0;
	}

	j.at("apples_count").get_to(this->apples_count);
	j.at("apples_got").get_to(this->apples_got);
	j.at("vibration").get_to(this->vibration);
	if (restoreExactState) {
		j.at("camera_x").get_to(this->camera_x);
		j.at("camera_y").get_to(this->camera_y);
		j.at("dcamera_x").get_to(this->dcamera_x);
		j.at("dcamera_y").get_to(this->dcamera_y);
		j.at("dcamera_a").get_to(this->dcamera_a);
		j.at("dcamera_b").get_to(this->dcamera_b);
		j.at("paused").get_to(this->paused);
		j.at("gift_cooldown").get_to(this->giftCooldown);
		j.at("global_animation_degree").get_to(degree);
		j.at("global_animation_degree_temp").get_to(degree_temp);
		this->level.runtimeStateFromJson(j.at("level_runtime"));
	}
	j.at("music_stopped").get_to(this->music_stopped);
	j.at("keys").get_to(this->keys);
	j.at("enemies").get_to(this->enemies);
	j.at("info_timer").get_to(this->info_timer);
	j.at("info_text").get_to(this->info_text);
	j.at("item_panel_x").get_to(this->item_panel_x);
	j.at("change_skulls").get_to(this->change_skulls);
	j.at("event1").get_to(this->event1);
	j.at("event2").get_to(this->event2);
	// Restore sprites for each sector
	const auto &sprites_json = j.at("sprites");
	if (!sprites_json.is_array() || sprites_json.size() != this->level.sectors.size()) {
		throw std::runtime_error("Saved sprite sectors do not match the level");
	}

	std::optional<std::size_t> lastCheckpointId;
	if(!j.at("last_cp_id").is_null()){
		lastCheckpointId = j.at("last_cp_id").get<std::size_t>();
	}

	this->lastCheckpoint = nullptr;
	this->playerSprite = nullptr;
	std::unordered_map<std::size_t, SpriteClass*> spritesById;

	for (size_t i = 0; i < sprites_json.size() && i < this->level.sectors.size(); ++i) {

		LevelSector* sector =  this->level.sectors.at(i);
		SpritesHandler& sprites = sector->sprites;
		sprites.fromJSON(sprites_json[i], this->spritePrototypes, sector);
		for (SpriteClass* sprite : sprites.Sprites_List) {
			if (!spritesById.emplace(sprite->id, sprite).second) {
				throw std::runtime_error("Duplicate sprite ID in saved game");
			}
		}

		SpriteClass* player = sprites.findPlayer();
		if(player!=nullptr){
			this->playerSprite = player;
		}

	}

	for (LevelSector* sector : this->level.sectors) {
		sector->sprites.resolveReferences(spritesById, restoreExactState);
	}
	if (lastCheckpointId.has_value()) {
		auto checkpoint = spritesById.find(*lastCheckpointId);
		if (checkpoint != spritesById.end()) {
			this->lastCheckpoint = checkpoint->second;
		}
		else if (restoreExactState) {
			throw std::runtime_error("Saved checkpoint sprite ID was not found");
		}
	}

	if(this->playerSprite==nullptr){
		throw std::runtime_error("The player sprite wasn't found while loading saved game!");
	}

	this->gifts.fromJson(j.at("gifts"), this->spritePrototypes);

	
	// Checkpoint loading retains its historical camera recentering. Quick-load
	// restores the saved integer and smoothed camera coordinates verbatim.
	if (!restoreExactState) {
		this->setCamera();
	}
	this->gfxTexture = this->playerSprite->level_sector->gfxTexture;
	this->exposePlayerToAIs();
}
