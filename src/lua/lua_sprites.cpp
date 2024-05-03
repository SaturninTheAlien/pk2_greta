//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

/**
 * @brief
 * Lua utils by SaturninTheAlien
 */

#include "lua_sprites.hpp"
#include "game/prototype.hpp"
#include "game/spriteclass.hpp"
namespace PK2lua{


void ExposePrototypeClass(sol::state& lua){

    /**
     * @brief 
     * Prototypes should be read-only not to change them during the gameplay.
     */

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
        "enemy", sol::readonly(&PrototypeClass::enemy),
        "hostile_to_everyone", sol::readonly(&PrototypeClass::hostile_to_everyone),
        "indestructible", sol::readonly(&PrototypeClass::indestructible),

        "color", sol::readonly(&PrototypeClass::color),
        "destruction_effect", sol::readonly(&PrototypeClass::destruction_effect),
        "energy", sol::readonly(&PrototypeClass::energy),
        "max_speed", sol::readonly(&PrototypeClass::max_speed));
}

void ExposeSpriteClass(sol::state& lua){
    lua.new_usertype<SpriteClass>(
        "SpriteClass",
        sol::no_constructor,

        /**
         * @brief 
         * Functions
         */

        "transform", &SpriteClass::Transform,
        "transform_to", &SpriteClass::TransformTo,

        "die", &SpriteClass::Die,
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

        "a", &SpriteClass::a, // x velocity
        "b", &SpriteClass::b, // y velocity

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
         * Shouldn't be changed in scripts because they are handled by physics.cpp and so on
         */

        //"player", sol::readonly(&SpriteClass::player),
        "player", &SpriteClass::player,

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

}