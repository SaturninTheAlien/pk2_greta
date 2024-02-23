#include "lua_game.hpp"
#include "game/prototypes_handler.hpp"
#include "game/sprites_handler.hpp"
#include "game/game.hpp"

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
    
    PK2_API["load_sprite_prototype"] = LoadSpritePrototype;
    PK2_API["spawn_sprite"] = sol::overload(AddSprite1, AddSprite2);

    PK2_API["get_player"] = [](){return AI_Functions::player;};

    PK2_API["get_player_invisible"] = [](){ return AI_Functions::player_invisible;};
    PK2_API["get_sprites"] = [](){return Game->spritesHandler.Sprites_List;};

    lua["_pk2_api"] = PK2_API;
}

}