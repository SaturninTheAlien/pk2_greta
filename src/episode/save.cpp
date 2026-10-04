//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

/**
 * @brief 
 * New saving system by Saturnin
 */
#include "save.hpp"
#include "save_slots.hpp"

#include "episodeclass.hpp"

#include "engine/PFile.hpp"
#include "engine/PFilesystem.hpp"
#include "engine/PJson.hpp"
#include "engine/PString.hpp"

#include <filesystem>

namespace fs = std::filesystem;
namespace PK2save{

static fs::path getPathForSaveFile(const EpisodeClass* episode, bool createDirs){
    fs::path p =  PFilesystem::GetDataPathP() / "saves" / fs::u8path(episode->player_name);

    if(createDirs)PFilesystem::CreateDirectoryP(p);

    if(episode->entry.is_zip){
        p = p / fs::u8path(PString::removeSuffix(episode->entry.zipfile, ".zip"));
        if(createDirs)PFilesystem::CreateDirectoryP(p);
    }

    return p / (episode->entry.name + ".dat");
}

void LoadModern(EpisodeClass* episode){

    fs::path p = getPathForSaveFile(episode, false);
    if(!fs::exists(p) || fs::is_directory(p)) return;

    PFile::RW rw = PFile::File(p).getRW("r");
    nlohmann::json j = rw.readCBOR();
    rw.close();

    episode->next_level = j["next"].get<u32>();
    episode->completed = j["completed"].get<bool>();

    std::vector<PK2SaveLevelEntry> saveEntries = j["levels"].get<std::vector<PK2SaveLevelEntry>>();

    int level_id = 0;
    for(const LevelEntry& levelEntry: episode->getLevelEntries()){

        for(const PK2SaveLevelEntry& saveEntry: saveEntries){
            if(saveEntry.level_name==levelEntry.fileName){
                episode->updateLevelStatus(level_id, saveEntry.status);
                episode->updateLevelBestScore(level_id, saveEntry.best_score);
                break;
            }
        }

        ++level_id;
    }
}

void SaveModern(const EpisodeClass* episode){
      

    nlohmann::json j;

    j["next"] = episode->next_level;
    j["completed"] = episode->isCompleted();

    std::vector<PK2SaveLevelEntry> saveEntries;
    int level_id = 0;
    for(const LevelEntry& levelEntry: episode->getLevelEntries()){
        PK2SaveLevelEntry saveEntry;
        saveEntry.level_name = levelEntry.fileName;
        saveEntry.status = episode->getLevelStatus(level_id);
        saveEntry.best_score = episode->getLevelBestScore(level_id);

        ++level_id;

        saveEntries.emplace_back(saveEntry);
    }

    j["levels"] = saveEntries;

    PFile::RW rw = PFile::File(getPathForSaveFile(episode, true)).getRW("w");
    rw.writeCBOR(j);
    rw.close();
}

}