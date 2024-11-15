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
#include <filesystem>
#include <sstream>


#ifdef __ANDROID__
#include <jni.h>
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
static fs::path mDataPath;

static std::string mEpisodeName;
static PZip::PZip* mEpisodeZip;


static bool mAssetsPathSet = false;
static bool mDataPathSet = false;


void CreateDirectory(const std::string& path_in){
    fs::path p(path_in);

    /*if(!p.is_absolute()){
        p = mAssetsPath / p;
    }*/

	if(!std::filesystem::exists(p) || !std::filesystem::is_directory(p)){
		std::filesystem::create_directory(p);
	}
}




bool SetAssetsPath(const std::string& name){
    fs::path p = name;

    fs::path p1 = p / "gfx" / "pk2stuff.bmp";

    if(fs::exists(p1)){

        mAssetsPath = p;
        mAssetsPathSet = true;
        return true;
    }

    fs::path p2 = p / "res" / "gfx" / "pk2stuff.bmp";
    if(fs::exists(p2)){
        mAssetsPath = p / "res";
        mAssetsPathSet = true;
        return true;
    }

    return false;
}

bool SetDataPath(const std::string& name){
    mDataPath = name;
    mDataPathSet = true;

    /**
     * @brief 
     * Create the directories if they don't exist.
     */

    CreateDirectory(mDataPath.string());
    CreateDirectory( (mDataPath / "scores").string());
    CreateDirectory( (mDataPath / "mapstore").string());

    //TODO
    //return false if the directory is not writeable

    return true;
}

void SetDefaultAssetsPath() {
	if(mAssetsPathSet)return;

#ifdef __ANDROID__
    mDataPath = SDL_AndroidGetInternalStoragePath();
    mDataPathSet = true;
#else
	char* c_path = SDL_GetBasePath();
	if(c_path==nullptr){

        std::ostringstream os;

        os<<"Cannot get the base path\n";
        os<<SDL_GetError();

        throw PFile::PFileException(os.str());
	}

    bool success = false;

	#ifndef _WIN32
    fs::path executable_dir = fs::path(c_path).parent_path();
    if(executable_dir.filename().string() == "bin"){
        success = SetAssetsPath( (executable_dir.parent_path() / "res").string());
    }
    else{
        success = SetAssetsPath(executable_dir.string());
    }

	#else
	success = SetAssetsPath(c_path);
	#endif

	SDL_free(c_path);

    if(!success){
        throw PFile::PFileException("Cannot set the default assets path!");
    }

#endif
}

void SetDefaultDataPath(){
    if(mDataPathSet)return;

    bool success = false;


#ifdef __ANDROID__
    SetDataPath(SDL_AndroidGetInternalStoragePath());
#else
    #ifdef PK2_PORTABLE
    success = SetDataPath((mAssetsPath / "data").string());
    #else
        char* data_path_p = SDL_GetPrefPath("Piste Gamez", "Pekka Kana 2");
        if(data_path_p==nullptr){
            std::ostringstream os;
            os<<"SDL_GetPrefPath failed: "<<SDL_GetError();
            throw PFile::PFileException(os.str());
        }

        success = SetDataPath(data_path_p);
        SDL_free(data_path_p);
    #endif
#endif

    if(!success){
        throw PFile::PFileException("Cannot set the default data path!");
    }

    return;
}

std::string GetAssetsPath(){
    return mAssetsPath.string();
}

std::string GetDataPath(){
    return mDataPath.string();
}

PFile::Path GetDataFileW(const std::string& filename){
    return (mDataPath / filename).string();
}

std::string GetEpisodeDirectory(){
    if(!mEpisodeName.empty()){
        return (mAssetsPath / EPISODES_DIR / mEpisodeName).string();
    }

    return mAssetsPath.string();
}


void SetEpisode(const std::string& episodeName, PZip::PZip* zip_file){
    mEpisodeName = episodeName;
    mEpisodeZip = zip_file;
}

/**
 * @brief 
 * Finding files, cAsE insensitive
 */
static std::optional<std::string> FindFile(const fs::path& dir, const std::string& cAsE,  const std::string& alt_extension){
    if(!fs::exists(dir) || !fs::is_directory(dir))return {};
    std::string name_lowercase = PString::rtrim(PString::lowercase(cAsE));

    std::string name_lowercase_alt = "";
    if(!alt_extension.empty()){
        name_lowercase_alt = fs::path(name_lowercase).replace_extension(alt_extension).string();
    }

    std::optional<std::string> alt_res = {};


    for (const auto & entry : fs::directory_iterator(dir)){
        if(!entry.is_directory()){
            fs::path filename = entry.path().filename();

            std::string s1 = PString::lowercase(filename.string());
            
            if(name_lowercase == s1){

                return (dir / filename).string();
            }
            else if(!alt_extension.empty() && name_lowercase_alt == s1){
                alt_res = (dir / filename).string();

            }
        }
    }

    return alt_res;
}






std::optional<PFile::Path> FindVanillaAsset(const std::string& name, const std::string& default_dir, const std::string& alt_extension){
    std::string filename = fs::path(name).filename().string();

#ifdef __ANDROID__
    /**
     * apk://assets/sprites/pig.spr2
     */

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();

    jclass clazz(env->GetObjectClass(activity));
    jmethodID method_id = env->GetMethodID(clazz, "findPK2Asset", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    if(method_id==0){
        throw std::runtime_error("JNI: Method \"findPK2Asset\" not found!");
    }

    jstring param1 = env->NewStringUTF(name.c_str());
    jstring param2 = env->NewStringUTF(default_dir.c_str());
    jstring param3 = alt_extension.empty() ? nullptr : env->NewStringUTF(alt_extension.c_str());

    jstring res = (jstring)env->CallObjectMethod(activity, method_id, param1, param2, param3);
    env->DeleteLocalRef(param1);
    env->DeleteLocalRef(param2);  

    if(param3!=nullptr){
        env->DeleteLocalRef(param3);
    }

    if(res!=nullptr){
        const char* utf = env->GetStringUTFChars(res, nullptr);

        PFile::Path p((fs::path(default_dir) / utf).string());

        env->ReleaseStringUTFChars(res, utf);
        env->DeleteLocalRef(res);

        p.insideAndroidAPK = true;

        return p;
    }

#else
    /**
     * @brief 
     * sprites/pig.spr2
     */
    std::optional<std::string> op = FindFile(mAssetsPath / default_dir, filename, alt_extension);
    if(op.has_value()){
        return PFile::Path(*op);
    }

#endif

    return {};
}



std::optional<PFile::Path> FindEpisodeAsset(const std::string& name, const std::string& default_dir, const std::string& alt_extension){
    std::string filename = fs::path(name).filename().string();
    if(filename.empty()) return {};

    if(mEpisodeZip!=nullptr){

        std::optional<PZip::PZipEntry> entry;
        /**
         * @brief 
         * zip:/episodes/"episode"/pig.spr2
         */

        entry = mEpisodeZip->getEntry( (fs::path("episodes") / mEpisodeName / filename).string(), alt_extension);
        if(entry.has_value())return PFile::Path(mEpisodeZip, *entry);
        
        /**
         * @brief 
         * zip:/sprites/pig.spr2
         */
        if(!default_dir.empty()){
            entry = mEpisodeZip->getEntry((fs::path(default_dir)/filename).string(), alt_extension);
            if(entry.has_value())return PFile::Path(mEpisodeZip, *entry);
        }

        
    }
    
    else if(!mEpisodeName.empty()){

        #ifndef __ANDROID__

        /**
         * @brief 
         * episodes/"episode"/pig.spr2
         */
        std::optional<std::string> op = FindFile(mAssetsPath / "episodes" / mEpisodeName, filename, alt_extension);
        if(op.has_value()){
            return PFile::Path(*op);
        }

        /**
         * @brief 
         * episodes/"episode"/sprites/pig.spr2
         */
        if(!default_dir.empty()){
            op = FindFile(mAssetsPath / "episodes" / mEpisodeName / default_dir, filename, alt_extension);
            if(op.has_value()){
                return PFile::Path(*op);
            }
        }

        #else
        /**
         * @brief 
         * apk://episodes/"episode"/pig.spr2
         */
        std::optional<PFile::Path> op = FindVanillaAsset(
            filename, (fs::path("episodes") / mEpisodeName).string(), alt_extension );
        if(op.has_value()){
            return op;
        }
        
        #endif
    }

    return {};
}

std::optional<PFile::Path> FindAsset(const std::string& name, const std::string& default_dir, const std::string& alt_extension){
    if(name.empty())return {};

    /**
     * 1. /full_path/pig.spr2
     */    
    fs::path p(name);
    if(p.is_absolute() && fs::exists(p) && !fs::is_directory(p))return PFile::Path(name);

    std::optional<PFile::Path> op = FindEpisodeAsset(name, default_dir, alt_extension);
    if(op.has_value())return op;

    op = FindVanillaAsset(name, default_dir, alt_extension);
    
    if(!op.has_value()){
        PLog::Write(PLog::WARN, "PFilesystem", "File \"%s\" not found!", name.c_str());
    }

    return op;
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


std::vector<std::string> ScanOriginalAssetsDirectory(const std::string& name, const std::string& filter){

#ifndef __ANDROID__
    return ScanDirectory_s(name, filter);
#else
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();

    jclass clazz(env->GetObjectClass(activity));
    jmethodID method_id = env->GetMethodID(clazz, "listPK2Assets", "(Ljava/lang/String;)[Ljava/lang/String;");
    if(method_id==0){
        throw std::runtime_error("JNI: Method listPK2Assets not found!");
    }

    jstring param = env->NewStringUTF(name.c_str());
    jobjectArray strings_array = (jobjectArray)env->CallObjectMethod(activity, method_id, param);
    env->DeleteLocalRef(param);
    

    if(strings_array==nullptr){
        throw std::runtime_error(std::string("Cannot scan the APK directory: ")+name);
    }


    int len = env->GetArrayLength(strings_array);
    std::vector<std::string> result;

    for (int i = 0; i < len; i++) {

		jstring js = (jstring)env->GetObjectArrayElement(strings_array, i);
		const char* utf = env->GetStringUTFChars(js, nullptr);

        std::string s = utf;

        env->ReleaseStringUTFChars(js, utf);
        env->DeleteLocalRef(js);


        if(filter.empty()){
            result.emplace_back(s);
        }
        else if(filter=="/"){
            if(s.find(".")==std::string::npos){
                result.emplace_back(s);
            }
        }
        else{
            std::string extension = PString::lowercase(fs::path(s).extension().string());
            if(extension==filter){
                result.emplace_back(s);
            }
        }
	}

    env->DeleteLocalRef(strings_array);
	
	return result;
#endif
}

}