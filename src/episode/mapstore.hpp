//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/platform.hpp"

#include <vector>
#include <string>


struct episode_entry {

    std::string name;
    std::string zipfile;
    bool is_zip;

};

extern std::vector<episode_entry> episodes;

void Search_Episodes();

#ifdef __ANDROID__
void Android_InstallZipEpisode();

#endif