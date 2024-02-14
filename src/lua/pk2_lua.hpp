//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

/**
 * @brief
 * Lua utils by SaturninTheAlien
 */

#pragma once

#include "3rd_party/sol.hpp"
#include "engine/PFile.hpp"



class EpisodeClass;

namespace PK2lua{

sol::state* CreateGameLuaVM(EpisodeClass* episode);

}
