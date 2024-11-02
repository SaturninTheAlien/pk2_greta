//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "episode/episodeclass.hpp"

#include "episode/mapstore.hpp"
#include "game/levelclass.hpp"
#include "language.hpp"
#include "system.hpp"
#include "save_legacy.hpp"

#include "engine/PLog.hpp"
#include "engine/PFile.hpp"
#include "engine/PFilesystem.hpp"

#include "engine/PDraw.hpp"

#include "lua/pk2_lua.hpp"

#include <algorithm>
#include <cstring>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

EpisodeClass* Episode = nullptr;


EpisodeClass::~EpisodeClass(){
	PFilesystem::SetEpisode("", nullptr);
}

std::string EpisodeClass::getScoresPath()const{
	return (fs::path(PFilesystem::GetDataPath()) / "scores" / (this->entry.name + ".dat")).string();
}

void EpisodeClass::openScores() {
	try{
		scoresTable.load(this->getScoresPath());
	}
	catch(const std::exception&e){
		PLog::Write(PLog::WARN, "PK2", e.what());
		PLog::Write(PLog::INFO, "PK2", "Can't load scores files");

	}
}

// Version 1.1
void EpisodeClass::saveScores() {
	PFile::Path path(this->getScoresPath());

	try{
		scoresTable.save(path);
	}
	catch(const std::exception& e){
		PLog::Write(PLog::ERR, "PK2", e.what());
		PLog::Write(PLog::ERR, "PK2", "Can't save scores");
	}
}

//TODO - Load info from different languages
void EpisodeClass::loadInfo() {

	std::optional<PFile::Path> infofile = PFilesystem::FindEpisodeAsset("infosign.txt", "");
	if(infofile.has_value()){
		if (this->infos.Read_File(*infofile)){
			PLog::Write(PLog::DEBUG, "PK2", "%s loaded", infofile->c_str());
		}
		else{
			PLog::Write(PLog::ERR, "PK2", "Cannot load %s", infofile->c_str());
		}		
	}
}

//TODO - don't load the same image again
void EpisodeClass::loadAssets() {

	std::optional<PFile::Path> path = PFilesystem::FindAsset("pk2stuff.png",
		PFilesystem::GFX_DIR, ".bmp");

	if(!path.has_value()){
		PLog::Write(PLog::ERR, "PK2", "Can't load pk2stuff"); //"Can't load map bg"
	}
	else{
		PDraw::image_load(game_assets, *path, true);
	}

	path = PFilesystem::FindAsset("pk2stuff2.png",
		PFilesystem::GFX_DIR, ".bmp");

	if(!path.has_value()){
		PLog::Write(PLog::ERR, "PK2", "Can't load pk2stuff2"); //"Can't load map bg"
	}
	else{
		PDraw::image_load(game_assets2, *path, true);
	}
}

void EpisodeClass::load() {
	
	if (entry.is_zip){
		this->source_zip.open( (fs::path(PFilesystem::GetDataPath())/"mapstore"/entry.zipfile).string());
		PFilesystem::SetEpisode(entry.name, &this->source_zip);
	}
	else{
		PFilesystem::SetEpisode(entry.name, nullptr);
	}

	std::string dir = PFilesystem::GetEpisodeDirectory();

	std::vector<std::string> namelist;
	std::vector<PFile::Path> pathlist;


	if(entry.is_zip){
		std::vector<PZip::PZipEntry> v = this->source_zip.scanDirectory(
			std::string("episodes/")+this->entry.name, ".map");
		for(const PZip::PZipEntry& en: v){
			pathlist.emplace_back(PFile::Path(&this->source_zip, en));
			namelist.emplace_back(en.name);
		}
	}
	else{
		namelist = PFilesystem::ScanDirectory_s(dir, ".map");
		for(const std::string& name: namelist){
			pathlist.emplace_back(PFile::Path((fs::path(dir) / name).string()));
		}

	}

	std::size_t levelsNumber = namelist.size();

	// Read levels plain data
	for (u32 i = 0; i < levelsNumber; i++) {

		try{
			LevelEntry levelEntry;

			LevelClass temp;

			levelEntry.fileName = namelist[i];

			temp.load(pathlist[i], true);

			levelEntry.levelName = temp.name;
			levelEntry.map_x = temp.icon_x;
			levelEntry.map_y = temp.icon_y;

			levelEntry.number = temp.level_number;
			levelEntry.icon_id = temp.icon_id;

			this->levels_list_v.emplace_back(levelEntry);
		}
		catch(const std::exception& e){
			PLog::Write(PLog::ERR, "PK2 level", e.what());
		}
		
	}

	std::optional<PFile::Path> config_path = PFilesystem::FindEpisodeAsset("config.txt", "");
	if(config_path.has_value()){

		PLang config(*config_path);
		if (config.loaded) {

			int id = config.Search_Id("glow_effect");
			if (id != -1) {
				PLog::Write(PLog::INFO, "PK2", "Episode glow is ON");
				this->glows = true;
			}

			id = config.Search_Id("hide_numbers");
			if (id != -1) {
				PLog::Write(PLog::INFO, "PK2", "Episode hide numbers is ON");
				this->hide_numbers = true;
			}

			id = config.Search_Id("ignore_collectable");
			if (id != -1) {
				PLog::Write(PLog::INFO, "PK2", "Episode ignore apples is ON");
				this->ignore_collectable = true;
			}

			id = config.Search_Id("collectable_name");
			if (id != -1) {
				this->collectable_name = config.Get_Text(id);
				PLog::Write(PLog::INFO, "PK2", "Collectable name:");
				PLog::Write(PLog::INFO, "PK2", this->collectable_name.c_str());
			}

			/*id = config.Search_Id("require_all_levels");
			if (id != -1) {
				PLog::Write(PLog::INFO, "PK2", "Episode require all levels is ON");
				this->require_all_levels = true;
			}*/

			id = config.Search_Id("no_ending");
			if (id != -1) {
				PLog::Write(PLog::INFO, "PK2", "Episode no ending is ON");
				this->no_ending = true;
			}

			id = config.Search_Id("potion_transformation_offset");
			if (id != -1) {
				PLog::Write(PLog::INFO, "PK2", "Potion transformation offset is ON");

				const char* txt = config.Get_Text(id);

				if (strcmp(txt, "default") == 0)
					this->transformation_offset = false;
				else if (strcmp(txt, "yes") == 0)
					this->transformation_offset = true;
				else if (strcmp(txt, "no") == 0)
					this->transformation_offset = false;	
			}



			// Is it really an episode issue?
			// For me, it's a level issue
			/*id = config.Search_Id("use_button_timer");
			if (id != -1) {
				PLog::Write(PLog::INFO, "PK2", "Episode use button timer is ON");
				this->use_button_timer = true;
			}*/
		}
		else{
			PLog::Write(PLog::ERR, "PK2", "Cannot open episode config file.");
		}
	}

	// Sort levels
	std::stable_sort(this->levels_list_v.begin(), this->levels_list_v.end(),
	[](const LevelEntry& a, const LevelEntry& b) {
		return a.number < b.number;
	});

	// Set positions
	int i = 0;
	for(LevelEntry& entry: this->levels_list_v){
		if (entry.map_x == 0){
			entry.map_x = 172 + i*30;
		}
		if (entry.map_y == 0){
			entry.map_y = 270;
		}
		++i;
	}
	
	this->sfx.loadAllForEpisode(sfx_global, this);

	this->openScores();
	this->loadInfo();

	this->updateNextLevel();
}

EpisodeClass::EpisodeClass(int save) {

	//Search the id
	int sz = episodes.size();
	bool set = false;
	for (int i = 0; i < sz; i++) {

		if (episodes[i].name.compare(saves_list[save].episode) == 0) {
			if (set)
				PLog::Write(PLog::WARN, "PK2", "Episode conflict on %s, choosing the first one", saves_list[save].episode);
			else {
				this->entry = episodes[i];
				set = true;
			}
		}
	}
	saves_list[save].name[19] = '\0';
	this->player_name = saves_list[save].name;
	this->next_level = saves_list[save].next_level;
	this->player_score = saves_list[save].score;

	

	this->load();

	int size = this->levels_list_v.size();
	if(size > EPISODI_MAX_LEVELS){
		size = EPISODI_MAX_LEVELS;
	}

	for (int j = 0; j < size; j++) {

		this->levels_list_v[j].status = saves_list[save].level_status[j];
	}

}

u8 EpisodeClass::getLevelStatus(int level_id)const{
	if(level_id >= 0 && level_id < (int)this->levels_list_v.size()){
		return this->levels_list_v[level_id].status;
	}
	else{
		return 0;
	}		
}

void EpisodeClass::updateLevelStatus(int level_id, u8 status){
	if(level_id >= 0 && level_id < (int)this->levels_list_v.size()){
		this->levels_list_v[level_id].status = status;
		this->updateNextLevel();
	}
}

std::string EpisodeClass::getLevelFilename(int level_id)const{
	if(level_id >= 0 && level_id < (int)this->levels_list_v.size()){
		return this->levels_list_v[level_id].fileName;
	}
	throw std::runtime_error(std::string("Level with id=")+std::to_string(level_id)+" not found!");
}

int EpisodeClass::findLevelbyFilename(const std::string& levelFilename)const{

	for(std::size_t i=0; i < this->levels_list_v.size();++i){
		if(levelFilename == this->levels_list_v[i].fileName){
			return (int)i;
		}
	}
	
	return -1;
}

EpisodeClass::EpisodeClass(const std::string& player_name, episode_entry entry):
entry(entry), player_name(player_name){	
	this->load();
}

void EpisodeClass::updateNextLevel() {
	for(const LevelEntry& entry: this->levels_list_v){
		u8 status = entry.status;

		if((status & LEVEL_PASSED) != 0 && entry.number + 1 > next_level ){
			next_level = entry.number + 1;
		}
	}
}
