#pragma once

#define SOL_ALL_SAFETIES_ON 1
#define SOL_SAFE_FUNCTION 1
#include "3rd_party/sol.hpp"

class EpisodeClass;

namespace PK2lua{

sol::state* CreateEpisodeLuaVM(EpisodeClass* episode);

}
