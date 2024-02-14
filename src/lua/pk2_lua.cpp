//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

#include "pk2_lua.hpp"

#include "lua_sprites.hpp"

#include "engine/PLog.hpp"
#include "engine/PFile.hpp"

#include "episode/episodeclass.hpp"
#include "system.hpp"

namespace PK2lua{


sol::state* CreateGameLuaVM(EpisodeClass* episode){

    PFile::Path main_lua_f = episode->Get_Dir("main.lua");
    if(!FindAsset(&main_lua_f, "lua" PE_SEP)){
        PLog::Write(PLog::INFO, "PK2lua", "No Lua scripting in this episode");
        return nullptr;
    }
    PLog::Write(PLog::INFO, "PK2lua", "Loading main.lua script");
    std::string main_lua = main_lua_f.GetContentAsString();
    
    PLog::Write(PLog::INFO, "PK2lua", "Creating lua VM");
    sol::state* lua = new sol::state();
    lua->open_libraries(sol::lib::base,
            sol::lib::string,
            sol::lib::table,
            sol::lib::math,
            sol::lib::coroutine,
            sol::lib::package);

    PLog::Write(PLog::INFO, "PK2lua", "Running main.lua");

    ExposePrototypeClass(*lua);
    ExposeSpriteClass(*lua);

    lua->safe_script(main_lua);
    return lua;
}

}