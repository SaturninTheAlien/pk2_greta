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

namespace fs = std::filesystem;

std::vector<episode_entry> episodes;

void Search_Episodes() {

	std::vector<std::string> list = PFilesystem::ScanDirectory_s(PFilesystem::EPISODES_DIR, "/");

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


