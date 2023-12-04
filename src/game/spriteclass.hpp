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

#include "engine/PJson.hpp"
#include "spriteclass_legacy.hpp"
#include "spriteclass_constants.hpp"
#include "spriteclass_commands.hpp"
#include "sprite_ai_table.hpp"


class PrototypeClass;

void Prototype_ClearAll();
PrototypeClass* Get_Prototype_Debug(std::size_t index);
PrototypeClass* Prototype_Load(const std::string& filename);

//Classes used in game
class SpriteAnimation{
public:
    SpriteAnimation(){};
    SpriteAnimation(const LegacySprAnimation& anim){
        this->loop = anim.loop;
        this->sequence = std::vector<int>(anim.sequence, anim.sequence + anim.frames);
    }
    bool loop = false;
    std::vector<int> sequence;

    friend void to_json(nlohmann::json& j, const SpriteAnimation& a);
    friend void from_json(const nlohmann::json&j, SpriteAnimation& a);
};

class PrototypeClass{
public:

    static const std::map<std::string, int> SoundTypesDict;
    static const std::map<std::string, int> AnimationsDict;
    static const std::map<std::string, u8> ColorsDict;

    std::string version = "2.0";

    std::string filename;
    std::string picture_filename;
    
    int     type = TYPE_NOTHING;

    std::array<std::string, SPRITE_SOUNDS_NUMBER> sound_files = {""};
    std::array<int, SPRITE_SOUNDS_NUMBER> sounds = {-1};

    //int     framet[SPRITE_MAX_FRAMEJA] = {0};
    std::vector<int>frames;
    std::vector<int>frames_mirror;
    //int     framet_peilikuva[SPRITE_MAX_FRAMEJA] = {0};
    int      frames_number            = 0;

    std::array<SpriteAnimation, SPRITE_ANIMATIONS_NUMBER> animations;
    //PK2SPRITE_ANIMAATIO animaatiot[SPRITE_MAX_ANIMAATIOITA] = {};
    //u8      animations_number       = 0;
    int      frame_rate         = 0;
    int     picture_frame_x             = 0;
    int     picture_frame_y             = 0;
    int     picture_frame_width  = 0;
    int     picture_frame_height = 0;

    std::string name;
    int     width    = 0;
    int     height   = 0;
    double  weight    = 0;

    bool    enemy     = false;
    int     energy        = 0;
    int     damage        = 0;
    int      damage_type = DAMAGE_IMPACT;
    int      immunity_type        = DAMAGE_NONE;
    int     score        = 0;

    std::vector<int> AI_v;
    std::vector<SpriteAI::AI_Class> AI_f;

    int first_ai()const{
        return AI_v.empty()? 0 : AI_v[0];
    }

    int      max_jump    = 0;
    double  max_speed   = 3;
    int     charge_time  = 0;
    u8      color         = COLOR_NORMAL;
    bool    is_wall         = false;
    int     how_destroyed = FX_DESTRUCT_ANIMAATIO;
    bool    can_open_locks        = false;
    bool    vibrates      = false;
    int      bonuses_number = 1;
    int     attack1_time = 60;
    int     attack2_time = 60;

    int     parallax_type = 0;

    std::string transformation_sprite;
    std::string bonus_sprite;
    std::string ammo1_sprite;
    std::string ammo2_sprite;


    PrototypeClass* transformation     = nullptr;
    PrototypeClass* bonus      = nullptr;
    PrototypeClass* ammo1     = nullptr;
    PrototypeClass* ammo2     = nullptr;

    bool    check_tiles = true;
    int     sound_frequency      = 22050;
    bool    random_sound_frequency    = true;

    bool    is_wall_up       = true;
    bool    is_wall_down       = true;
    bool    is_wall_right   = true;
    bool    is_wall_left = true;

    u8      effect       = EFFECT_NONE;
    bool    is_transparent       = false;
    int     projectile_charge_time    = 0;
    bool    can_glide    = false;
    //bool    boss         = false; //unused
    bool    bonus_always = false;
    bool    can_swim     = false;

    /**
     * @brief 
     * Field for the purpose of counting.
     * If Episode->ignore_collectable==false, this field will be always false.
     */
    bool    big_apple     = false;   //

    /**
     * @brief 
     * If true a sprite will be active even far from the camera
     */
    bool    always_active = false;   // 

    /**
     * @brief 
     * If the creature sprite is ambient (like white butterflies or chain),
     * it should not consume attacks or interact with other sprites 
     */
    bool    ambient = false;


    bool change_color_to_alpha = true;
    unsigned int color_to_alpha = 255;

    PrototypeClass();
    ~PrototypeClass();

    /**
     * @brief 
     * Load JSON sprite prototype
     */
    void     LoadPrototypeJSON(PFile::Path path);
    /**
     * @brief 
     * Load legacy sprite prototype
     */
    void     LoadPrototypeLegacy(PFile::Path path);
    
    /**
     * @brief 
     * Load sprite assets like texture, sounds, etc.
     */
    void     LoadAssets(PFile::Path path);

    
    void     Draw(int x, int y, int frame)const;
    bool    HasAI(int AI)const;

    std::vector<SpriteCommands::Command*>commands;
private:
    void    SetProto10(PrototypeClass10 &proto);
    void    SetProto11(PrototypeClass11 &proto);
    void    SetProto12(PrototypeClass12 &proto);
    void    SetProto13(PrototypeClass13 &proto);

    void    SetProto20(const nlohmann::json& j);
};
class SpriteClass{
public:

    bool    active       = false;           // if the sprite is acting
    int     player          = 0;               // 0 = isn't player, 1 = is player
    PrototypeClass *prototype   = nullptr;         // the sprite prototype
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
    bool    reuna_oikealla   = false;           // is there a pit on the right side of the sprite?
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

    bool CanDamageOnCollision(const SpriteClass* target)const;

    
    SpriteClass();
    SpriteClass(PrototypeClass *prototype, int player, double x, double y);
    ~SpriteClass();

    int  Draw(int kamera_x, int kamera_y);   // animate and draw the sprite
    int  Animaatio(int anim_i, bool nollaus);  // set sprite animation
    int  Animoi();                             // animate the sprite
    void HandleEffects();                      // create sprite effects
    bool HasAI(int AI)const{
        return prototype->HasAI(AI);
    }; // if the sprite has a AI


    //AI_Functions
    void AI_Rooster();
    void AI_Bonus();
    void AI_Egg();
    void AI_Egg2();
    void AI_Projectile();
    void AI_Jumper();
    void AI_BlueFrog();
    void AI_RedFrog();
    void AI_Basic();
    void AI_Turning_Horizontally();
    void AI_Turning_Vertically();
    void AI_Look_For_Cliffs();
    void AI_Random_Turning();
    void AI_Random_Change_Dir_H();
    void AI_Random_Jump();
    void AI_Random_Move_Vert_Hori();
    void AI_Follow_Player(SpriteClass &player);
    void AI_Follow_Player_If_Seen(SpriteClass &player);
    void AI_Follow_Player_If_Seen_Vert_Hori(SpriteClass &player);
    void AI_Follow_Player_Vert_Hori(SpriteClass &player);
    //int AI_Jahtaa_Pelaajaa(SpriteClass &player); //unused
    void AI_Run_Away_From_Player(SpriteClass &player);
    void AI_Transform_When_Energy_Under_2();
    void AI_Transform_When_Energy_Over_1();
    void AI_Self_Transformation();
    void AI_Transform_If_Damaged();
    void AI_Attack_1_If_Damaged();
    void AI_Attack_2_If_Damaged();
    void AI_Attack_1_Nonstop();
    void AI_Attack_2_Nonstop();
    void AI_Attack_1_if_Player_in_Front(SpriteClass &player);
    void AI_Attack_2_if_Player_in_Front(SpriteClass &player);
    void AI_Attack_1_if_Player_Below(SpriteClass &player);

    void AI_Attack_1_If_Player_Above(SpriteClass& player);
    void AI_Attack_2_If_Player_Above(SpriteClass& player);
    void AI_Transform_If_Player_Above(SpriteClass& player);
    void AI_Transform_If_Player_Below(SpriteClass& player);


    void AI_NonStop();
    void AI_Jump_If_Player_Above(SpriteClass &player);
    void AI_SelfDestruction();
    void AI_Damaged_by_Water();
    void AI_Kill_Everyone();
    void AI_Friction_Effect();
    void AI_Hiding();
    void AI_Return_To_Orig_X();
    void AI_Return_To_Orig_Y();
    void AI_Turn_Back_If_Damaged();
    void AI_Fall_When_Shaken(int tarina);
    void AI_Move_X(double liike);
    void AI_Move_Y(double liike);
    void AI_Tippuu_If_Switch_Pressed(int kytkin);
    void AI_Move_If_Switch_Pressed(int kytkin, int ak, int bk);
    bool AI_Teleport(const std::list<SpriteClass*>& spritet, SpriteClass &player);
    void AI_Climber();
    void AI_Climber2();
    bool AI_Info(SpriteClass &player);
    void AI_Die_If_Parent_Nullptr();

    void AI_Destructed_Next_To_Player(SpriteClass &player);
    void AI_Follow_Commands(SpriteClass *player);

private:
    void Animation_Basic();
    void Animation_Rooster();
    void Animation_Bonus();
    void Animation_Egg();
    void Animation_Projectile();
};
