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
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <sstream>


#ifdef __ANDROID__
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

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

std::string GetEpisodeDirectory(){
    if(!mEpisodeName.empty()){
        return (mAssetsPath / EPISODES_DIR / mEpisodeName).string();
    }

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

        if(!default_dir.empty() &&
        FindFile(mAssetsPath / "episodes" / mEpisodeName / default_dir, filename, name, alt_extension)){
            return PFile::Path(name);
        }
    }

    return {};
}

std::vector<std::string> ScanDirectory_s(const std::string& name, const std::string& filter){
    fs::path dir(name);
    if(!dir.is_absolute()){
        dir = mAssetsPath / dir;
    }
    std::vector<std::string> result;
    if(!fs::exists(dir) || !fs::is_directory(dir)){
        PLog::Write(PLog::WARN, "PFile", "Directory \"%s\" not found, cannot scan it", name.c_str());
        return result;
    }


    for (const auto & entry : fs::directory_iterator(dir)){

        if(filter.empty()){
            result.push_back(entry.path().filename().string());
        }
        else if(filter=="/"){
            if(entry.is_directory()){
                result.push_back(entry.path().filename().string());
            }
        }
        else{
            auto filename = entry.path().filename();
            std::string extension = PString::lowercase(filename.extension().string());
            if(extension==filter){
                result.push_back(filename.string());
            }
        }
    }

    std::sort(result.begin(), result.end());
    
    return result;
}


#ifdef __ANDROID__
/**
 * Is it necessary?
 * Why can't apk be handled as a regular zip.
 * Perhaps we don't need Java to do it.
 */
std::vector<std::string> scan_apk(const char* dir, const char* type) {

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));
	jmethodID method_id = env->GetMethodID(clazz, "listDir", "(Ljava/lang/String;)[Ljava/lang/String;");

	jstring param = env->NewStringUTF(dir);
	jobjectArray array = (jobjectArray)env->CallObjectMethod(activity, method_id, param);

	jsize size = env->GetArrayLength(array);
	
	std::vector<std::string> result;

	for (int i = 0; i < size; i++) {

		jstring filename = (jstring)env->GetObjectArrayElement(array, i);
		jboolean isCopy;
		const char* file = env->GetStringUTFChars(filename, &isCopy);

		if( file[0] != '.' ) {

			if(type[0] == '/' && strstr(file, ".") == NULL) { //provisory way - consider file without '.' a directory

				result.push_back(file);
				continue;

			} else if(type[0] == '\0') {
			
				result.push_back(file);
				continue;
			
			} else {

				const char* ext = Get_Extension(file);
				if(strcmp(ext, type) == 0) {

					result.push_back(file);
					continue;

				}
			}
		}

		if (isCopy == JNI_TRUE) {
    		env->ReleaseStringUTFChars(filename, file);
		}
	}


	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

    PLog::Write(PLog::DEBUG, "PFile", "Scanned APK on \"%s\" for \"%s\". Found %i matches", dir, type, (int)result.size());

	return result;

}

#endif

}