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

    lua["_pk2_api"] = PK2_API;
}

}