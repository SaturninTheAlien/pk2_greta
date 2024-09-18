//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

/**
 * @brief 
 * New filesystem utils by SaturninTheAlien to replace obsolete ones from PFile.cpp
 */

#include "PFilesystem.hpp"
#include "PString.hpp"
#include "engine/PLog.hpp"

#include <SDL.h>

#include <iostream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;
namespace PFilesystem{

const std::string EPISODES_DIR = "episodes";
const std::string GFX_DIR = "gfx";

const std::string TILES_DIR = (fs::path(GFX_DIR) / "tiles").string();
const std::string SCENERY_DIR = (fs::path(GFX_DIR) / "scenery").string();

const std::string LANGUAGE_DIR = "language";
const std::string FONTS_DIR = (fs::path(LANGUAGE_DIR) / "fonts").string();

const std::string SFX_DIR = "sfx";
const std::string SPRITES_DIR = "sprites";
const std::string MUSIC_DIR = "music";

const std::string LUA_DIR = "lua";
const std::string LIFE_DIR = "rle";

static fs::path mAssetsPath;
static std::string mEpisodeName;
static bool mAssetsPathSet = false;

void CreateDirectory(const std::string& path_in){
    fs::path p(path_in);
    if(!p.is_absolute()){
        p = mAssetsPath / p;
    }
	if(!std::filesystem::exists(p) || !std::filesystem::is_directory(p)){
		std::filesystem::create_directory(p);
	}
}




bool SetAssetsPath(const std::string& name){

    //TODO Remove this
    PFile::SetAssetsPath(name);

    fs::path p = name;

    fs::path p1 = p / "gfx" / "pk2stuff.bmp";

    if(fs::exists(p1)){

        mAssetsPath = p;
        return true;
    }

    fs::path p2 = p / "res" / "gfx" / "pk2stuff.bmp";
    if(fs::exists(p2)){
        mAssetsPath = p / "res";
        return true;
    }

    return false;
}


void SetDefaultAssetsPath() {
	if(mAssetsPathSet)return;

	char* c_path = SDL_GetBasePath();
	if(c_path==nullptr){

        std::ostringstream os;

        os<<"Cannot get the base path\n";
        os<<SDL_GetError();

        throw PFile::PFileException(os.str());
	}

	#ifndef _WIN32
    fs::path executable_dir = fs::path(c_path).parent_path();
    if(executable_dir.filename().string() == "bin"){
        SetAssetsPath( (executable_dir.parent_path() / "res").string());
    }
    else{
        SetAssetsPath(executable_dir.string());
    }

	#else
	SetAssetsPath(c_path);
	#endif

	SDL_free(c_path);
}


std::string GetAssetsPath(){
    return mAssetsPath.string();
}


void SetEpisode(const std::string& episodeName, void* zip_file){
    mEpisodeName = episodeName;

    if(zip_file!=nullptr){
        std::cout<<"TO DO - support zip files"<<std::endl;
    }
}

/**
 * @brief 
 * Finding files, cAsE insensitive
 */
static bool FindFile(const fs::path& dir, const std::string& cAsE, std::string& res, const std::string& alt_extension){
    if(!fs::exists(dir) || !fs::is_directory(dir))return false;
    std::string name_lowercase = PString::rtrim(PString::lowercase(cAsE));

    std::string name_lowercase_alt = "";
    if(!alt_extension.empty()){
        name_lowercase_alt = fs::path(name_lowercase).replace_extension(alt_extension).string();
    }


    for (const auto & entry : fs::directory_iterator(dir)){
        if(!entry.is_directory()){
            fs::path filename = entry.path().filename();

            std::string s1 = PString::lowercase(filename.string());
            if(name_lowercase == s1 || 
            (!alt_extension.empty() && name_lowercase_alt == s1)){
                
                res = (dir / filename).string();
                return true;
            }
        }
    }

    return false;
}

bool FindAsset_s(std::string& name, const std::string& default_dir, const std::string& alt_extension){
    if(name.empty())return false;

    /**
     * 1. /full_path/pig.spr2
     */
    else if(fs::exists(name) && !fs::is_directory(name))return true;

    std::string filename = fs::path(name).filename().string();
    if(filename.empty()) return false;

    if(!mEpisodeName.empty()){

        /**
         * @brief 
         * episodes/"episode"/pig.spr2
         */

        if(FindFile(mAssetsPath / "episodes" / mEpisodeName, filename, name, alt_extension))return true;

        /**
         * @brief 
         * episodes/"episode"/sprites/pig.spr2
         */

        if(FindFile(mAssetsPath / "episodes" / mEpisodeName / default_dir, filename, name, alt_extension))return true;
    }

    /**
     * @brief 
     * sprites/pig.spr2
     */

    return FindFile(mAssetsPath / default_dir, filename, name, alt_extension);
}


std::optional<PFile::Path> FindAsset(std::string name, const std::string& default_dir, const std::string& alt_extension){
    if(FindAsset_s(name, default_dir, alt_extension)){
        return PFile::Path(name);        
    }

    PLog::Write(PLog::WARN, "PK2", "Can't find %s", name.c_str());

    return {};
}

std::optional<PFile::Path> FindVanillaAsset(std::string name, const std::string& dir){

    fs::path p = mAssetsPath / dir / name;
    if(fs::exists(p)){
        return PFile::Path(p);
    }
    return {};
}

std::optional<PFile::Path> FindEpisodeAsset(std::string name, const std::string& default_dir, const std::string& alt_extension){
    std::string filename = fs::path(name).filename().string();
    if(filename.empty()) return {};

    if(!mEpisodeName.empty()){

        /**
         * @brief 
         * episodes/"episode"/pig.spr2
         */

        if(FindFile(mAssetsPath / "episodes" / mEpisodeName, filename, name, alt_extension)){
            return PFile::Path(name);
        }

        /**
         * @brief 
         * episodes/"episode"/sprites/pig.spr2
         */

        if(FindFile(mAssetsPath / "episodes" / mEpisodeName / default_dir, filename, name, alt_extension)){
            return PFile::Path(name);
        }
    }

    return {};
}


}