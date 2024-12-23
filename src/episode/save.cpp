//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

/**
 * @brief 
 * New saving system by Saturnin
 */
#include "episodeclass.hpp"

#include "save.hpp"
#include "episodeclass.hpp"

#include "engine/PFile.hpp"
#include "engine/PFilesystem.hpp"
#include "engine/PJson.hpp"
#include "engine/PString.hpp"

#include <filesystem>

namespace fs = std::filesystem;
namespace PK2save{

class SaveEntry{
public:
    std::string level_name;
    u8 status = 0;    
};

void to_json(nlohmann::json& j,const SaveEntry& entry){
    j["n"] = entry.level_name;
    j["s"] = entry.status; 
}

void from_json(const nlohmann::json& j, SaveEntry& entry){
    j.at("n").get_to(entry.level_name);
    j.at("s").get_to(entry.status);
}

static fs::path getPathForSaveFile(const EpisodeClass* episode, bool createDirs){
    fs::path p = fs::path(PFilesystem::GetDataPath()) / "saves" / episode->player_name;
    if(createDirs)PFilesystem::CreateDirectory(p);

    if(episode->entry.is_zip){
        p = p / PString::removeSuffix(episode->entry.zipfile, ".zip");
        if(createDirs)PFilesystem::CreateDirectory(p);
    }

    return p / (episode->entry.name + ".dat");
}

void LoadModern(EpisodeClass* episode){

    fs::path p = getPathForSaveFile(episode, false);
    if(!fs::exists(p) || fs::is_directory(p)) return;

    PFile::RW rw = PFile::Path(p.string()).GetRW2("r");
    nlohmann::json j = rw.readCBOR();
    rw.close();

    episode->next_level = j["next"].get<u32>();
    episode->completed = j["completed"].get<bool>();

    std::vector<SaveEntry> saveEntries = j["levels"].get<std::vector<SaveEntry>>();

    int level_id = 0;
    for(const LevelEntry& levelEntry: episode->getLevelEntries()){

        for(const SaveEntry& saveEntry: saveEntries){
            if(saveEntry.level_name==levelEntry.fileName){
                episode->updateLevelStatus(level_id, saveEntry.status);
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

    std::vector<SaveEntry> saveEntries;
    int level_id = 0;
    for(const LevelEntry& levelEntry: episode->getLevelEntries()){
        SaveEntry saveEntry;
        saveEntry.level_name = levelEntry.fileName;
        saveEntry.status = episode->getLevelStatus(level_id);
        ++level_id;

        saveEntries.emplace_back(saveEntry);
    }

    j["levels"] = saveEntries;

    //std::cout<<j.dump(4)<<std::endl;

    PFile::RW rw = PFile::Path(getPathForSaveFile(episode, true).string()).GetRW2("w");
    rw.writeCBOR(j);
    rw.close();
}

}