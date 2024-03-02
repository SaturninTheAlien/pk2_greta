#include "lua_game.hpp"
#include "game/prototypes_handler.hpp"
#include "game/sprites_handler.hpp"
#include "game/game.hpp"
#include <vector>

namespace PK2lua{


PrototypeClass* LoadSpritePrototype(const std::string& filename){
    return Game->spritesHandler.prototypesHandler.loadPrototype(filename);
}

SpriteClass* AddSprite1(PrototypeClass* prototype, double x, double y, SpriteClass*parent){
    return Game->spritesHandler.addLuaSprite(prototype, x, y, parent);
}

SpriteClass* AddSprite2(PrototypeClass* prototype, double x, double y){
    return Game->spritesHandler.addLuaSprite(prototype, x, y);
}

class LuaEventListener{
public:
    LuaEventListener(int event_type, sol::protected_function func):
        event_type(event_type), func(func) {}

    int event_type;
    sol::protected_function func;
};

std::vector<LuaEventListener> mEventListeners;

void ClearEventListeners(){
    mEventListeners.clear();
}

void TriggerEventListeners(int event_type){
    for(LuaEventListener listener: mEventListeners){
        if(listener.event_type!=event_type) continue;

        sol::protected_function_result res = listener.func();
        if(!res.valid()){
            throw res.get<sol::error>();
        }
    }
}

void AddEventListener(int event_type, sol::object o){
    if(o.is<std::function<void()>>()){
        mEventListeners.push_back(LuaEventListener(event_type, o));
    }
    else{
        throw std::runtime_error("PK2Lua, not proper event listener function!");
    }
}

void ExposeGameAPI(sol::state& lua){

    sol::table PK2_API = lua.create_table();

    /**
     * @brief 
     * Load sprite prototype (.spr2/.spr)
     */

    PK2_API["load_sprite_prototype"] = LoadSpritePrototype;
    /**
     * @brief 
     * Add a new sprite
     */
    PK2_API["add_sprite"] = sol::overload(AddSprite1, AddSprite2);

    /**
     * @brief 
     * Get the player sprite
     */
    PK2_API["get_player"] = [](){return Game->spritesHandler.Player_Sprite;};

    /**
     * @brief 
     * If player is alive and not invisible
     * nil otherwise
     */
    PK2_API["get_player_if_accessible"] = [](){return AI_Functions::player;};

    /**
     * @brief 
     * Get a list of all the sprites
     */
    
    PK2_API["get_sprite_list"] = [](){return Game->spritesHandler.Sprites_List;};


    /**
     * @brief 
     * Lua events API
     */

    PK2_API["events"] = lua.create_table_with(
        "SKULLS_CHANGED", LUA_EVENT_SKULL_BLOCKS_CHANGED,
        "EVENT1", LUA_EVENT_1,
        "EVENT2", LUA_EVENT_2,
        "add_listener", AddEventListener,
        "clean_listeners", ClearEventListeners);

    /**
     * @brief 
     * Show info
     */
    PK2_API["show_info"] = [](const std::string& text){ Game->Show_Info(text);};

    lua["_pk2_api"] = PK2_API;
}

}