//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "episode/episodeclass.hpp"

#include "episode/mapstore.hpp"
#include "game/levelclass.hpp"
#include "language.hpp"
#include "system.hpp"
#include "save.hpp"

#include "engine/PLog.hpp"
#include "engine/PUtils.hpp"
#include "engine/PFile.hpp"
#include "engine/PFilesystem.hpp"

#include "engine/PDraw.hpp"

#include "lua/pk2_lua.hpp"

#include <algorithm>
#include <cstring>
#include <string>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

EpisodeClass* Episode = nullptr;

#define VERSION "1.1"


EpisodeClass::~EpisodeClass(){
	PFilesystem::SetEpisode("", nullptr);
}

std::string EpisodeClass::getScoresPath()const{
	return (fs::path(data_path) / "scores" / (this->entry.name + ".dat")).string();
}

void EpisodeClass::Clear_Scores() {

	memset(&this->scores, 0, sizeof(this->scores));

}

void EpisodeClass::openScores() {

	try{
		char versio[4];
		this->Clear_Scores();
		PFile::Path path(this->getScoresPath());
		if(!path.exists())return;

		PFile::RW file = path.GetRW2("r");
		file.read(versio, 4);
		if (strncmp(versio, VERSION, 4) == 0) {

			PLog::Write(PLog::INFO, "PK2", "Loading scores v1.1");
			
			u32 count;
			file.read(count);

			if (count > this->level_count)
				count = this->level_count;

			for (u32 i = 0; i < count; i++) {
				
				file.read(this->scores.has_score[i]);
				file.read(this->scores.best_score[i]);
				file.read(this->scores.top_player[i], 20);
				file.read(this->scores.max_apples[i]);
				
				file.read(this->scores.has_time[i]);
				file.read(this->scores.best_time[i]);
				file.read(this->scores.fastest_player[i], 20);

			}

			file.read(this->scores.episode_top_score);
			file.read(this->scores.episode_top_player, 20);

			file.close();

		} else if (strncmp(versio, "1.0", 4) == 0) {

			PLog::Write(PLog::INFO, "PK2", "Loading scores v1.0");

			PK2EPISODESCORES10 temp;
			file.read(&temp, sizeof(temp));
			file.close();

			for (int i = 0; i < EPISODI_MAX_LEVELS; i++) {

				this->scores.has_score[i] = (temp.best_score[i] != 0) ? 1 : 0;
				
				this->scores.best_score[i] = temp.best_score[i];
				strncpy(this->scores.top_player[i], temp.top_player[i], 20);
				
				this->scores.has_time[i] = (temp.best_time[i] != 0) ? 1 : 0;
				this->scores.max_apples[i] = 0;

				this->scores.best_time[i] = (s32)temp.best_time[i] * 100; //FRAME = (dec)conds * 100
				strncpy(this->scores.fastest_player[i], temp.fastest_player[i], 20);

				this->Save_Scores();

			}

			this->scores.episode_top_score = temp.episode_top_score;
			strncpy(this->scores.episode_top_player, temp.episode_top_player, 20);

		} else {

			PLog::Write(PLog::INFO, "PK2", "Can't read this scores version");

			file.close();

		}
	}
	catch(const std::exception&e){
		PLog::Write(PLog::WARN, "PK2", e.what());
		PLog::Write(PLog::INFO, "PK2", "Can't load scores files");

	}
}

// Version 1.1
void EpisodeClass::Save_Scores() {
	
	char versio[4] = VERSION;

	PFile::Path path(this->getScoresPath());

	try{
		PFile::RW file = path.GetRW2("w");

		file.write(versio, 4);
		file.write(level_count);

		for (u32 i = 0; i < level_count; i++) {
			
			file.write(this->scores.has_score[i]);
			file.write(this->scores.best_score[i]);
			file.write(this->scores.top_player[i], 20);
			file.write(this->scores.max_apples[i]);
			
			file.write(this->scores.has_time[i]);
			file.write(this->scores.best_time[i]);
			file.write(this->scores.fastest_player[i], 20);

		}

		file.write(this->scores.episode_top_score);
		file.write(this->scores.episode_top_player, 20);
		file.close();

	}
	catch(const std::exception& e){
		PLog::Write(PLog::ERR, "PK2", e.what());
		PLog::Write(PLog::ERR, "PK2", "Can't save scores");
	}
}

//TODO - Load info from different languages
void EpisodeClass::Load_Info() {

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
void EpisodeClass::Load_Assets() {

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

void EpisodeClass::Load() {
	
	if (entry.is_zip){
		this->source_zip.open( (fs::path(data_path)/"mapstore"/entry.zipfile).string());
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

	this->level_count = namelist.size();

	// Read levels plain data
	for (u32 i = 0; i < this->level_count; i++) {

		try{
			LevelClass temp;
			char* mapname = this->levels_list[i].tiedosto;
			strncpy(mapname, namelist[i].c_str(), PE_PATH_SIZE);
			
			temp.load(pathlist[i], true);

			strncpy(this->levels_list[i].nimi, temp.name.c_str(), 40);
			this->levels_list[i].nimi[39] = '\0';

			this->levels_list[i].x = temp.icon_x;// 142 + i*35;
			this->levels_list[i].y = temp.icon_y;// 270;
			this->levels_list[i].order = temp.level_number;
			this->levels_list[i].icon = temp.icon_id;
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

			id = config.Search_Id("require_all_levels");
			if (id != -1) {
				PLog::Write(PLog::INFO, "PK2", "Episode require all levels is ON");
				this->require_all_levels = true;
			}

			id = config.Search_Id("no_ending");
			if (id != -1) {
				PLog::Write(PLog::INFO, "PK2", "Episode no ending is ON");
				this->no_ending = true;
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
	std::stable_sort(this->levels_list, this->levels_list + this->level_count,
	[](const PK2LEVEL& a, const PK2LEVEL& b) {
		return a.order < b.order;
	});

	// Set positions
	for (u32 i = 0; i < this->level_count; i++) {	
	
		if (levels_list[i].x == 0)
			levels_list[i].x = 172 + i*30;

		if (levels_list[i].y == 0)
			levels_list[i].y = 270;
	
	}

	this->sfx.loadAllForEpisode(sfx_global, this);

	this->openScores();
	this->Load_Info();

	this->Update_NextLevel();
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
	
	strcpy(this->player_name, saves_list[save].name);
	this->next_level = saves_list[save].next_level;
	this->player_score = saves_list[save].score;

	for (int j = 0; j < EPISODI_MAX_LEVELS; j++) {

		this->level_status[j] = saves_list[save].level_status[j];

	}

	this->Load();

}

EpisodeClass::EpisodeClass(const char* player_name, episode_entry entry) {

	this->entry = entry;
	strncpy(this->player_name, player_name, 20);

	for (int j = 0; j < EPISODI_MAX_LEVELS; j++)
		this->level_status[j] = 0;
	
	this->Load();
	
}
/**
 * Remove this
 */
PFile::Path EpisodeClass::Get_Dir(const std::string& file)const {

	fs::path p = fs::path(PFilesystem::GetAssetsPath()) / PFilesystem::EPISODES_DIR / entry.name / file;

	/*PFilesystem::GetAssetsPath

	std::string path("episodes" PE_SEP);
	path += entry.name + PE_SEP + file;*/

/*#ifdef PK2_USE_ZIP
	if (this->entry.is_zip)
		return PFile::Path( &this->source_zip, path);
#endif*/

	return PFile::Path(p.string());

}

void EpisodeClass::Update_NextLevel() {

	if (require_all_levels) {

		next_level = UINT32_MAX;
		for (u32 i = 0; i < level_count; i++)
			if (!(level_status[i] & LEVEL_PASSED) && levels_list[i].order < next_level) {
				next_level = levels_list[i].order;
				break;
			}

	} else {

		next_level = 1;
		for (u32 i = 0; i < level_count; i++) {
			
			if (levels_list[i].order > next_level)
				break;

			if (level_status[i] & LEVEL_PASSED)
				next_level = levels_list[i].order + 1;
		}
		
		// Clear levels before next level
		bool ended = true;
		for (u32 i = 0; i < level_count; i++) {
			if (levels_list[i].order < next_level) {
				level_status[i] |= LEVEL_PASSED;
			}
			if (!(level_status[i] & LEVEL_PASSED))
				ended = false;
		}

		if (ended)
			next_level = UINT32_MAX;
		
	}


}
