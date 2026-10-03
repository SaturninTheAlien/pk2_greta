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
#include <filesystem>
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


void CreateDirectoryP(const std::filesystem::path& path);

void SetAssetsPath(const std::string& name);
void SetDataPath(const std::string& name);
void SetPrefDataPath();

void SetDefaultPaths();

/*std::string GetAssetsPath();
std::string GetDataPath();*/

const std::filesystem::path& GetAssetsPathP();
const std::filesystem::path& GetDataPathP();

//PFile::File GetDataFileW(const std::string& filename);

const std::filesystem::path& GetEpisodeDirectoryP();

std::filesystem::path GetScreenshotNameP();

void SetEpisode(const std::string& episodeName, PZip::PZip* zip_file=nullptr);
//bool FindAsset_s(std::string& name, const std::string& default_dir, const std::string& alt_extension);

std::optional<PFile::File> FindAsset(const std::string& name, const std::string& default_dir, const std::string& alt_extension="");
std::optional<PFile::File> FindVanillaAsset(const std::string& name, const std::string& default_dir, const std::string& alt_extension="");
std::optional<PFile::File> FindEpisodeAsset(const std::string& name, const std::string& default_dir, const std::string& alt_extension="");

std::vector<std::string> ScanDirectoryP(const std::filesystem::path& path_in, const std::string& filter="");
std::vector<std::string> ScanOriginalAssetsDirectory(const std::string& name, const std::string& filter="");


}