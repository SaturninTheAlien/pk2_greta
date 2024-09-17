//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
/**
 * @brief 
 * New filesystem utils by SaturninTheAlien to replace obsolete ones from PFile.cpp
 */
#pragma once

#include <string>
#include <vector>
#include <optional>
#include "PFile.hpp"

namespace PFilesystem{

extern const std::string EPISODES_DIR;

extern const std::string GFX_DIR;
extern const std::string TILES_DIR;
extern const std::string SCENERY_DIR;

extern const std::string LANGUAGE_DIR;
extern const std::string FONTS_DIR;

extern const std::string SFX_DIR;
extern const std::string SPRITES_DIR;
extern const std::string MUSIC_DIR;

extern const std::string LUA_DIR;
extern const std::string LIFE_DIR;


void CreateDirectory(const std::string& path);

bool SetAssetsPath(const std::string& name);
void SetDefaultAssetsPath();

std::string GetAssetsPath();

void SetEpisode(const std::string& episodeName, void* zip_file=nullptr);
bool FindAsset_s(std::string& name, const std::string& default_dir, const std::string& alt_extension);

std::optional<PFile::Path> FindAsset(std::string name, const std::string& default_dir, const std::string& alt_extension="");

std::optional<PFile::Path> FindVanillaAsset(std::string name, const std::string& default_dir);
}