#include "pk2_lua.hpp"
#include "engine/PLog.hpp"
#include "engine/PFile.hpp"
#include "episode/episodeclass.hpp"
#include "game/prototype.hpp"
#include "game/spriteclass.hpp"
#include "system.hpp"

namespace PK2lua{

void ExposeSpriteClass(sol::state& lua){
    lua.new_usertype<PrototypeClass>(
        "Prototype",
        sol::no_constructor,

        "name", sol::readonly(&PrototypeClass::name),
        "ammo1", sol::readonly(&PrototypeClass::ammo1),
        "ammo2", sol::readonly(&PrototypeClass::ammo2),
        "bonus", sol::readonly(&PrototypeClass::bonus),
        "transformation", sol::readonly(&PrototypeClass::transformation),
        "width", sol::readonly(&PrototypeClass::width),
        "height", sol::readonly(&PrototypeClass::height),
        "weight", sol::readonly(&PrototypeClass::weight),
        "enemy", sol::readonly(&PrototypeClass::enemy));

    lua.new_usertype<SpriteClass>(
        "SpriteClass",
        sol::no_constructor,

        /**
         * @brief 
         * Functions
         */

        "transform", &SpriteClass::Transform,
        "fly_to_target_x", &SpriteClass::FlyToWaypointX,
        "fly_to_target_y", &SpriteClass::FlyToWaypointY,
        "fly_to_target_xy", &SpriteClass::FlyToWaypointXY,

        /**
         * @brief 
         * Writeable fields
         */

        "active", &SpriteClass::active,
        "prototype", &SpriteClass::prototype,
        
        "x", &SpriteClass::x,
        "y", &SpriteClass::y,

        "vx", &SpriteClass::a,
        "vy", &SpriteClass::b,

        "flip_x", &SpriteClass::flip_x,
        "flip_y", &SpriteClass::flip_y,

        "energy", &SpriteClass::energy,
        "weight", &SpriteClass::weight,
        "crouched", &SpriteClass::crouched,

        "damage_timer", &SpriteClass::damage_timer,
        "invisible_timer", &SpriteClass::invisible_timer,
        "super_mode_timer", &SpriteClass::super_mode_timer,
        "attack1_timer", &SpriteClass::attack1_timer,
        "attack2_timer", &SpriteClass::attack2_timer,

        "enemy", &SpriteClass::enemy,
        "ammo1", &SpriteClass::ammo1,
        "ammo2", &SpriteClass::ammo2,

        /**
         * @brief 
         * Read-only fields
         * Shouldn't be changed in scripts because they are handled by physics.cpp
         */

        "orig_x", sol::readonly(&SpriteClass::orig_x),
        "orig_y", sol::readonly(&SpriteClass::orig_y),

        "in_water", sol::readonly(&SpriteClass::in_water),
        "swimming", sol::readonly(&SpriteClass::swimming),

        "hidden", sol::readonly(&SpriteClass::hidden),
        "orig_weight", sol::readonly(&SpriteClass::initial_weight),
        

        "can_move_left", sol::readonly(&SpriteClass::can_move_left),
        "can_move_right", sol::readonly(&SpriteClass::can_move_right),
        "can_move_up", sol::readonly(&SpriteClass::can_move_up),
        "can_move_down", sol::readonly(&SpriteClass::can_move_down)              
        
        );
}


sol::state* CreateEpisodeLuaVM(EpisodeClass* episode){

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

    ExposeSpriteClass(*lua);

    lua->safe_script(main_lua);
    return lua;
}

}