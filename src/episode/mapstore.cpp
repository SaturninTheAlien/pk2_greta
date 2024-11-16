//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "mapstore.hpp"
#include "save_legacy.hpp"

#include "system.hpp"

#include "engine/PUtils.hpp"
#include "engine/PFile.hpp"
#include "engine/PLog.hpp"
#include "engine/PZip.hpp"
#include "engine/PFilesystem.hpp"


#include <cstring>
#include <algorithm>
#include <filesystem>

#ifdef __ANDROID__
#include <jni.h>
#include <optional>
#include <SDL.h>
#endif

namespace fs = std::filesystem;

std::vector<episode_entry> episodes;

void Search_Episodes() {
	episodes.clear();

	std::vector<std::string> list = PFilesystem::ScanOriginalAssetsDirectory(PFilesystem::EPISODES_DIR, "/");

	for (std::string ep : list) {
		episode_entry e;
		e.name = ep;
		e.is_zip = false;
        episodes.push_back(e);
	}

	#ifdef PK2_USE_ZIP

	std::string mapstore_path=(fs::path(PFilesystem::GetDataPath())/"mapstore").string();

	list = PFilesystem::ScanDirectory_s(mapstore_path, ".zip");
	for (std::string zip : list) {
		try{

			PZip::PZip zp((fs::path(mapstore_path)/zip).string());
			std::vector<std::string> zip_list = zp.findSubdirectories("episodes");

			for (std::string ep : zip_list) {
				episode_entry e;
				e.name = ep;
				e.is_zip = true;
				e.zipfile = zip;
				episodes.push_back(e);
			}
		}
		catch(const std::exception& e){
			PLog::Write(PLog::ERR, "PK2", e.what());
		}
	}
	#endif

	if (episodes.size() > 1)
		std::stable_sort(episodes.begin(), episodes.end(),
		[](const episode_entry& a, const episode_entry& b) {
			return PUtils::Alphabetical_Compare(a.name.c_str(), b.name.c_str()) == 1;
		});
	
	PLog::Write(PLog::DEBUG, "PK2", "Found %i episodes", (int)episodes.size());

	Load_SaveFile();
}

#ifdef __ANDROID__


static std::optional<std::string> installZip(const std::string& targetPath){

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();

    jclass clazz(env->GetObjectClass(activity));
    jmethodID method_id = env->GetMethodID(clazz, "installZip", "(Ljava/lang/String;)Ljava/lang/String;");
    if(method_id==0){
        throw std::runtime_error("JNI: Method \"installZip\" not found!");
    }

    jstring param = targetPath.empty()? nullptr : env->NewStringUTF(targetPath.c_str());
    jstring res = (jstring)env->CallObjectMethod(activity, method_id, param);
    if(param!=nullptr){
        env->DeleteLocalRef(param);
    }

    std::optional<std::string> result;
    if(res!=nullptr){
        const char* utf = env->GetStringUTFChars(res, nullptr);
        result = std::string(utf);
        env->ReleaseStringUTFChars(res, utf);
        env->DeleteLocalRef(res);
    }


    return result;
}

void Android_InstallZipEpisode(){
	std::optional<std::string> res = installZip(
		(fs::path(PFilesystem::GetDataPath())/"mapstore").string());

	if(res.has_value()){
		PLog::Write(PLog::INFO, "PK2", "Installed a new zip episode: %s", res->c_str());

		try{
			PZip::PZip zp(*res);
			std::vector<std::string> zip_list = zp.findSubdirectories("episodes");

			for (std::string ep : zip_list) {
				episode_entry e;
				e.name = ep;
				e.is_zip = true;
				e.zipfile = fs::path(*res).filename().string();
				episodes.push_back(e);
			}
		}
		catch(const std::exception& e){
			PLog::Write(PLog::ERR, "PK2", e.what());
		}

	}
	else{
		PLog::Write(PLog::WARN, "PK2", "Episode not installed!");
	}
}

#endif


