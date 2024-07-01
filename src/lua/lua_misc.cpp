//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

/**
 * @brief
 * Lua utils by SaturninTheAlien
 */

#include "lua_misc.hpp"
#include "gfx/effect.hpp"
#include "game/game.hpp"
#include "game/levelsector.hpp"

namespace PK2lua{

LevelSector* GetSector(int id){
    if(id>=0 && id<(int)Game->level.sectors.size()){
        return Game->level.sectors[id];
    }
    return nullptr;
}

void ExposeMiscAPI(sol::table& PK2_API){

    /**
     * @brief 
     * Show info
     */
    PK2_API["show_info"] = [](const std::string& text){ Game->Show_Info(text);};

    /**
     * @brief 
     * Spawn effect
     */
    
    PK2_API["effect"] = Effect_By_ID;

    /**
     * @brief 
     * Get level sector
     */

    PK2_API["get_sector"] = GetSector;
}


}