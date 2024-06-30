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

namespace PK2lua{

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
}


}