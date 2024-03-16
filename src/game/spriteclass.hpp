//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/PFile.hpp"
#include "engine/types.hpp"

#include <list>
#include <vector>
#include <string>
#include <map>
#include <array>

#include "prototype.hpp"

class SpriteClass{
public:

    bool    active       = false;           // if the sprite is acting
    bool    player          = false;          // if the sprite is the player
    PrototypeClass *prototype   = nullptr;      // the sprite prototype
    bool    removed          = true;            // the sprite was removed
    double  orig_x           = 0;               // original x location
    double  orig_y           = 0;               // original y location
    double  x                = 0;               // sprite x location
    double  y                = 0;               // sprite y location
    double  a                = 0;               // horizontal speed
    double  b                = 0;               // vertical speed
    bool    flip_x           = false;           // if it is flipped horizontally
    bool    flip_y           = false;           // if it is flipped vertically
    int     jump_timer       = 0;               // jump times: = 0 not jumping; > 0 jumping; < 0 falling
    bool    can_move_up             = true;            // can sprite move up now?
    bool    can_move_down             = true;            // can sprite move down now?
    bool    can_move_right         = true;            // can sprite move right now?
    bool    can_move_left       = true;            // can sprite move left now?
    bool    edge_on_the_left = false;           // is there a pit on the left side of the sprite?
    bool    edge_on_the_right   = false;           // is there a pit on the right side of the sprite?
    int     energy          = 0;               // the sprite energy
    SpriteClass *parent_sprite   = nullptr;         // the sprite's parent
    double  weight           = 0;               // sprite weight
    double  weight_button      = 0;               // sprite weight + weight above him (why it doesn't work?)
    bool    crouched         = false;           // if the sprite is crouched
    int     damage_timer     = 0;               // damage timer
    int     invisible_timer  = 0;               // invisibility timer
    int     super_mode_timer = 0;               // super mode timer
    int     charging_timer   = 0;               // charging time for the attacks
    int     attack1_timer    = 0;               // timer after attack 1
    int     attack2_timer    = 0;               // timer after attack 2
    bool    in_water          = false;           // if the sprite is inside water

    bool    swimming        = false;            // if the sprite is actively swimming in the water
    bool    max_speed_available = false;        // if the speed limit is disabled


    bool    hidden         = false;           // if the sprite is hidden
    double  initial_weight   = 0;               // sprite's original weight - the same as that of the prototype
    int     damage_taken    = 0;               // damage taken
    u8      damage_taken_type = DAMAGE_NONE; // damage taken type (e.g. snow).
    bool    enemy       = false;           // if it is a enemy
    PrototypeClass* ammo1   = nullptr;         // projectile 1
    PrototypeClass* ammo2   = nullptr;         // projectile 2

    int     seen_player_x    = -1;              // where the player was last seen x
    int     seen_player_y    = -1;              // where the player was last seen y

    int     action_timer     = 0;               // timer for some AI actions. vary from 1 to 32000
 
    int      animation_index  = ANIMATION_IDLE;  // animation index
    unsigned int current_sequence = 0;               // current sequence
    int      frame_timer      = 0;               // frame times
    int     mutation_timer   = 0;               // the mutation timer
    int     respawn_timer = 0;

    std::size_t current_command = 0;
    int     command_timer = -1;
    bool    self_destruction = false;           // true if the sprite commits a suicide
    
    /**
     * @brief 
     * To remove the initial splash effect
     */
    bool    initial_update = false; 

    bool CanDamageOnCollision(const SpriteClass* target)const;

    
    SpriteClass();
    SpriteClass(PrototypeClass *prototype, bool player, double x, double y, SpriteClass*parent=nullptr);
    ~SpriteClass();

    int  Draw(int camera_x, int camera_y);   // animate and draw the sprite
    int  Animaatio(int anim_i, bool nollaus);  // set sprite animation
    int  Animoi();                             // animate the sprite
    void HandleEffects();                      // create sprite effects
    bool HasAI(int AI)const{
        return prototype->HasAI(AI);
    };
    
    void AI_Move_X(double dx);
    void AI_Move_Y(double dy);

    bool FlyToWaypointX(double target_x);
    bool FlyToWaypointY(double target_y);
    bool FlyToWaypointXY(double target_x, double target_y);

    bool Transform(){
        return this->TransformTo(this->prototype->transformation);
    }

    bool TransformTo(PrototypeClass* transformation);

    void Die();

    void StartThunder();

private:
    void Animation_Basic();
    void Animation_Rooster();
    void Animation_Bonus();
    void Animation_Egg();
    void Animation_Projectile();
};
