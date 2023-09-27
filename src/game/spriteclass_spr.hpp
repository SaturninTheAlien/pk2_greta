#pragma once

#include "engine/types.hpp"

#define PK2SPRITE_CURRENT_VERSION "1.3"

#define MAX_PROTOTYYPPEJA  256

//#define SPRITE_MAX_FRAMEJA      50
//#define SPRITE_MAX_ANIMAATIOITA 20
//#define SPRITE_MAX_AI           10

//#define ANIMATION_SEQUENCE_SIZE 10

#define SPRITE_SOUNDS_NUMBER     7
#define SPRITE_ANIMATIONS_NUMBER       11

#define DAMAGE_TIME             50

//Spite file values

enum {

    ANIMATION_IDLE, //0
    ANIMATION_WALKING, //1
    ANIMATION_JUMP_UP, //2
    ANIMATION_JUMP_DOWN, //3
    ANIMATION_SQUAT, //4
    ANIMATION_DAMAGE, //5
    ANIMATION_DEATH, //6
    ANIMATION_ATTACK1, //7
    ANIMATION_ATTACK2, //8

    /**
     * @brief 
     * Reserved for future AIs, scripting, etc
     */
    ANIMATION_SPECIAL1, //9
    ANIMATION_SPECIAL2 //10
};

enum {

    SOUND_DAMAGE,
    SOUND_DESTRUCTION,
    SOUND_ATTACK1,
    SOUND_ATTACK2,
    SOUND_RANDOM,
    SOUND_SPECIAL1,
    SOUND_SPECIAL2

};

enum {

    EFFECT_NONE,
    EFFECT_STARS,
    EFFECT_SMOKE,
    EFFECT_THUNDER,

};

enum {
/*

RANDOM_HORIZONTAL_CHANGE_OF_DIRECTION
RANDOM_JUMPING
FOLLOW_THE_PLAYER
RANDOM_HORIZONTAL_CHANGE_OF_DIRECTION
FOLLOW_THE_PLAYER_IF_PLAYER_IS_IN_FRONT
TRANSFORMATION_IF_ENERGY_BELOW_2_(P)
TRANSFORMATION_IF_ENERGY_ABOVE_1_(P)
START_DIRECTIONS_TOWARDS_PLAYER
AMMONITION
NON_STOP
ATTACK_1_IF_DAMAGED
SELF_DESTRUCTION
ATTACK_1_IF_PLAYER_IS_IN_FRONT
ATTACK_1_IF_PLAYER_IS_BELOW
DAMAGED_BY_WATER_(P)
ATTACK_2_IF_PLAYER_IS_IN_FRONT
KILL_'EM_ALL
AFFECTED_BY_FRICTION
HIDE
RETURN_TO_START_POSITION_X
RETURN_TO_START_POSITION_Y
TELEPORT
THROWABLE_WEAPON
FALLS_WHEN_SHAKEN_(B)
CHANGE_TRAP_STONES_IF_KO'ED
CHANGE_TRAP_STONES_IF_DAMAGED
SELF_DESTRUCTS_WITH_MOTHER_SPRITE
MOVES_X_COS
MOVES_Y_COS
MOVES_X_SIN
MOVES_Y_SIN
MOVES_X_COS_FAST
MOVES_Y_SIN_FAST
MOVES_X_COS_SLOW
MOVES_Y_SIN_SLOW
MOVES_Y_SIN_FREE
RANDOM_TURNING
JUMP_IF_PLAYER_IS_ABOVE
TRANSFORMATION_TIMER_(B)
FALLS_IF_SWITCH_1_IS_PRESSED_(B)
FALLS_IF_SWITCH_2_IS_PRESSED_(B)
FALLS_IF_SWITCH_3_IS_PRESSED_(B)
MOVES_DOWN_IF_SWITCH_1_IS_PRESSED
MOVES_UP_IF_SWITCH_1_IS_PRESSED
MOVES_RIGHT_IF_SWITCH_1_IS_PRESSED
MOVES_LEFT_IF_SWITCH_1_IS_PRESSED
MOVES_DOWN_IF_SWITCH_2_IS_PRESSED
MOVES_UP_IF_SWITCH_2_IS_PRESSED
MOVES_RIGHT_IF_SWITCH_2_IS_PRESSED
MOVES_LEFT_IF_SWITCH_2_IS_PRESSED
MOVES_DOWN_IF_SWITCH_3_IS_PRESSED
MOVES_UP_IF_SWITCH_3_IS_PRESSED
MOVES_RIGHT_IF_SWITCH_3_IS_PRESSED
MOVES_LEFT_IF_SWITCH_3_IS_PRESSED
TURNS_VERTICALLY_FROM_OBSTACLE
RANDOM_VERTICAL_STARTING_DIRECTION
STARTING_DIRECTION_TOWARDS_PLAYER
CLIMBER
CLIMBER_TYPE_2
RUNS_AWAY_FROM_PLAYER_IF_SEES_PLAYER
REBORN_(B)
ARROW_LEFT
ARROW_RIGHT
ARROW_UP
ARROW_DOWN
MOVE_TO_ARROWS_DIRECTION
BACKGROUND_SPRITE_MOON
BACKGROUND_SPRITE_MOVES_TO_LEFT
BACKGROUND_SPRITE_MOVES_TO_RIGHT
ADD_TIME_TO_CLOCK
MAKE_PLAYER_INVISIBLE
FOLLOW_THE_PLAYER_VERTIC._AND_HORIZ.
FOLLOW_THE_PLAYER_VERTIC._AND_HORIZ.,_IF_PLAYER_IS_IN_FRONT
RANDOM_MOVE_VERTIC._AND_HORIZ.*/

    AI_NONE,
    AI_KANA,
    AI_EGG,
    AI_LITTLE_CHICKEN,
    AI_BONUS,
    AI_JUMPER, //?
    AI_BASIC,
    AI_TURNING_HORIZONTALLY,
    AI_LOOK_FOR_CLIFFS,
    AI_RANDOM_CHANGE_DIRECTION_H,
    AI_RANDOM_JUMP,
    AI_FOLLOW_PLAYER,
    AI_RANDOM_START_DIRECTION,
    AI_FOLLOW_PLAYER_IF_IN_FRONT,
    AI_CHANGE_WHEN_ENERGY_UNDER_2,
    AI_CHANGE_WHEN_ENERGY_OVER_1,
    AI_START_DIRECTIONS_TOWARDS_PLAYER,
    AI_AMMUS,
    AI_NONSTOP,
    AI_ATTACK_1_JOS_OSUTTU,
    AI_POMMI,
    AI_ATTACK_1_IF_PLAYER_IN_FRONT,
    AI_ATTACK_1_IF_PLAYER_BELLOW,
    AI_DAMAGED_BY_WATER,
    AI_ATTACK_2_IF_PLAYER_IN_FRONT,
    AI_KILL_EVERYONE,
    AI_KITKA_VAIKUTTAA,
    AI_PIILOUTUU,
    AI_PALAA_ALKUUN_X,
    AI_PALAA_ALKUUN_Y,
    AI_TELEPORT,

    // AI_31 - AI_34

    AI_HEITTOASE = 35,
    AI_TIPPUU_TARINASTA,
    AI_VAIHDA_KALLOT_JOS_TYRMATTY,
    AI_VAIHDA_KALLOT_JOS_OSUTTU,
    AI_TUHOUTUU_JOS_EMO_TUHOUTUU,

    // AI_40

    AI_LIIKKUU_X_COS = 41,
    AI_LIIKKUU_Y_COS,
    AI_LIIKKUU_X_SIN,
    AI_LIIKKUU_Y_SIN,
    AI_LIIKKUU_X_COS_NOPEA,
    AI_LIIKKUU_Y_SIN_NOPEA,
    AI_LIIKKUU_X_COS_HIDAS,
    AI_LIIKKUU_Y_SIN_HIDAS,
    AI_LIIKKUU_Y_SIN_VAPAA,

    AI_RANDOM_KAANTYMINEN,
    AI_HYPPY_JOS_PELAAJA_YLAPUOLELLA,
    AI_MUUTOS_AJASTIN,

    AI_TIPPUU_JOS_KYTKIN1_PAINETTU,
    AI_TIPPUU_JOS_KYTKIN2_PAINETTU,
    AI_TIPPUU_JOS_KYTKIN3_PAINETTU,

    AI_LIIKKUU_DOWN_JOS_KYTKIN1_PAINETTU,
    AI_LIIKKUU_UP_JOS_KYTKIN1_PAINETTU,
    AI_LIIKKUU_LEFT_JOS_KYTKIN1_PAINETTU,
    AI_LIIKKUU_RIGHT_JOS_KYTKIN1_PAINETTU,
    AI_LIIKKUU_DOWN_JOS_KYTKIN2_PAINETTU,
    AI_LIIKKUU_UP_JOS_KYTKIN2_PAINETTU,
    AI_LIIKKUU_LEFT_JOS_KYTKIN2_PAINETTU,
    AI_LIIKKUU_RIGHT_JOS_KYTKIN2_PAINETTU,
    AI_LIIKKUU_DOWN_JOS_KYTKIN3_PAINETTU,
    AI_LIIKKUU_UP_JOS_KYTKIN3_PAINETTU,
    AI_LIIKKUU_LEFT_JOS_KYTKIN3_PAINETTU,
    AI_LIIKKUU_RIGHT_JOS_KYTKIN3_PAINETTU,

    // AI_68, AI_69

    AI_KAANTYY_ESTEESTA_VERT = 70,
    AI_RANDOM_ALOITUSSUUNTA_VERT,
    AI_START_DIRECTIONS_TOWARDS_PLAYER_VERT,
    AI_KIIPEILIJA,
    AI_KIIPEILIJA2,
    AI_PAKENEE_PELAAJAA_JOS_NAKEE,
    AI_UUSI_JOS_TUHOUTUU,

    AI_NUOLI_LEFT,
    AI_NUOLI_RIGHT,
    AI_NUOLI_UP,
    AI_NUOLI_DOWN,
    AI_NUOLET_VAIKUTTAVAT,

    // AI_82 - AI_100

    AI_TAUSTA_KUU = 101,
    AI_TAUSTA_LIIKKUU_LEFT,  // unused
    AI_TAUSTA_LIIKKUU_RIGHT, // unused

    // AI_104 - AI_119

    AI_BONUS_AIKA = 120,
    AI_BONUS_NAKYMATTOMYYS,
    AI_BONUS_SUPERHYPPY,
    AI_BONUS_SUPERTULITUS,
    AI_BONUS_SUPERVAUHTI,

    // New AI
    AI_BONUS_SUPERMODE,

    // AI_126 - AI_128

    AI_MUUTOS_JOS_OSUTTU = 129,
    AI_FOLLOW_PLAYER_VERT_HORI,
    AI_FOLLOW_PLAYER_IF_IN_FRONT_VERT_HORI,
    AI_RANDOM_LIIKAHDUS_VERT_HORI,
    AI_SAMMAKKO1,
    AI_SAMMAKKO2,
    AI_SAMMAKKO3,
    AI_ATTACK_2_JOS_OSUTTU,
    AI_ATTACK_1_NONSTOP,
    AI_ATTACK_2_NONSTOP,
    AI_KAANTYY_JOS_OSUTTU,
    AI_EVIL_ONE,

    // AI_141 - AI_149

    // New AI
    AI_MAX_SPEED_PLAYER = 150,
    AI_MAX_SPEED_PLAYER_ON_SUPER,
    AI_SWIMMING,
    AI_MAX_SPEED_SWIMMING,

    AI_EGG2 = 170,

    // AI_154 - AI_200

    // New AI
    AI_INFOS_BEGIN = 201,
    AI_INFOS_END = 301,

    AI_CHICK = 400,
    AI_CHICKBOX,
    AI_DESTRUCTED_NEXT_TO_PLAYER,

};

enum { //Damage

    DAMAGE_NONE,
    DAMAGE_IMPACT,
    DAMAGE_DROP,
    DAMAGE_NOISE,
    DAMAGE_FIRE,
    DAMAGE_WATER,
    DAMAGE_SNOW,
    DAMAGE_BONUS,
    DAMAGE_ELECTRIC,
    DAMAGE_ITSARI,
    DAMAGE_COMPRESSION,
    DAMAGE_SMELL,
    DAMAGE_ALL,
    DAMAGE_STITCH

};

enum {

    TYPE_NOTHING,
    TYPE_GAME_CHARACTER,
    TYPE_BONUS,
    TYPE_PROJECTILE,
    TYPE_TELEPORT,
    TYPE_BACKGROUND

};

enum {

    COLOR_GRAY      = 0,
    COLOR_BLUE      = 32,
    COLOR_RED       = 64,
    COLOR_GREEN     = 96,
    COLOR_ORANGE    = 128,
    COLOR_VIOLET    = 160,
    COLOR_TURQUOISE = 192,
    COLOR_NORMAL    = 255

};

enum { //Destruction Effect

    FX_DESTRUCT_EI_TUHOUDU,
    FX_DESTRUCT_HOYHENET,
    FX_DESTRUCT_TAHDET_HARMAA,
    FX_DESTRUCT_TAHDET_SININEN,
    FX_DESTRUCT_TAHDET_PUNAINEN,
    FX_DESTRUCT_TAHDET_VIHREA,
    FX_DESTRUCT_TAHDET_ORANSSI,
    FX_DESTRUCT_TAHDET_VIOLETTI,
    FX_DESTRUCT_TAHDET_TURKOOSI,
    FX_DESTRUCT_RAJAHDYS_HARMAA,
    FX_DESTRUCT_RAJAHDYS_SININEN,
    FX_DESTRUCT_RAJAHDYS_PUNAINEN,
    FX_DESTRUCT_RAJAHDYS_VIHREA,
    FX_DESTRUCT_RAJAHDYS_ORANSSI,
    FX_DESTRUCT_RAJAHDYS_VIOLETTI,
    FX_DESTRUCT_RAJAHDYS_TURKOOSI,
    FX_DESTRUCT_SAVU_HARMAA,
    FX_DESTRUCT_SAVU_SININEN,
    FX_DESTRUCT_SAVU_PUNAINEN,
    FX_DESTRUCT_SAVU_VIHREA,
    FX_DESTRUCT_SAVU_ORANSSI,
    FX_DESTRUCT_SAVU_VIOLETTI,
    FX_DESTRUCT_SAVU_TURKOOSI,
    FX_DESTRUCT_SAVUPILVET,
    FX_DESTRUCT_ANIMAATIO = 100

};

enum {

    BONUSITEM_NOTHING,
    BONUSITEM_KEY,
    BONUSITEM_SCORE

};

struct LegacySprAnimation {

    u8    sequence[10] = {0}; // sequence
    u8    frames = 0;                              // frames
    bool  loop = false;                           // loop

};

//.spr file structures
struct PrototypeClass10{

    u32     sprite_type;

    char    picture[13];

    char    sound_files[7][13];
    u32     sounds[7];

    u8      frames_number;
    LegacySprAnimation animaatiot[20];
    u8      animations_number;
    u8      frame_rate;
    u32     picture_frame_x;
    u32     picture_frame_y;
    u32     picture_frame_width;
    u32     picture_frame_height;
    u32     kuva_frame_vali;

    char    name[30];
    u32     width;
    u32     height;
    double  weight;
    bool    enemy;
    u32     energy;
    u32     damage;
    u32     score;
    u32     AI[5];
    u32     immunity_type;
    u8      max_jump;
    u8      max_speed;
    u32     charge_time;
    u8      color;
    bool    is_wall;
    u32     how_destroyed;

    char    transformation_sprite[13];
    char    bonus_sprite[13];
    char    ammo1_sprite[13];
    char    ammo2_sprite[13];
    bool    can_open_locks;
};
struct PrototypeClass11{

    u32     sprite_type;

    char    picture[13];

    char    sound_files[7][13];
    u32     sounds[7];

    u8      frames_number;
    LegacySprAnimation animaatiot[20];
    u8      animations_number;
    u8      frame_rate;
    u32     picture_frame_x;
    u32     picture_frame_y;
    u32     picture_frame_width;
    u32     picture_frame_height;
    u32     kuva_frame_vali;

    char    name[30];
    u32     width;
    u32     height;
    double  weight;
    bool    enemy;
    u32     energy;
    u32     damage;
    u8      damage_type;
    u8      immunity_type;
    u32     score;
    u32     AI[5];
    u8      max_jump;
    u8      max_speed;
    u32     charge_time;
    u8      color;
    bool    is_wall;
    u32     how_destroyed;
    bool    can_open_locks;
    bool    vibrates;
    u8      bonuses_number;
    u32     attack1_time;
    u32     attack2_time;
    u32     parallax_type;

    char    transformation_sprite[13];
    char    bonus_sprite[13];
    char    ammo1_sprite[13];
    char    ammo2_sprite[13];

};
struct PrototypeClass12{

    u32     sprite_type;

    char    picture[13];

    char    sound_files[7][13];
    u32     sounds[7];

    u8      frames_number;
    LegacySprAnimation animaatiot[20];
    u8      animations_number;
    u8      frame_rate;
    u32     picture_frame_x;
    u32     picture_frame_y;
    u32     picture_frame_width;
    u32     picture_frame_height;
    u32     kuva_frame_vali;

    char    name[30];
    u32     width;
    u32     height;
    double  weight;
    bool    enemy;
    u32     energy;
    u32     damage;
    u8      damage_type;
    u8      immunity_type;
    u32     score;
    u32     AI[5];
    u8      max_jump;
    u8      max_speed;
    u32     charge_time;
    u8      color;
    bool    is_wall;
    u32     how_destroyed;
    bool    can_open_locks;
    bool    vibrates;
    u8      bonuses_number;
    u32     attack1_time;
    u32     attack2_time;
    u32     parallax_type;

    char    transformation_sprite[13];
    char    bonus_sprite[13];
    char    ammo1_sprite[13];
    char    ammo2_sprite[13];

    bool    makes_sounds;
    u32     sound_frequency;
    bool    random_sound_frequency;

    bool    is_wall_up;
    bool    is_wall_down;
    bool    is_wall_right;
    bool    is_wall_left;

};

struct PrototypeClass13{

    u32     sprite_type;                             // sprite type
    char    picture[100];                  // bmp path

    /**
     * @brief 
     * Sound files.
     * The index determines the sound type:
     * 0 -> damage
     * 1 -> destruction
     * 2 -> attack1
     * 3 -> attack2
     * 4 -> random
     * 5 -> special1
     * 6 -> special2
     */
    char    sound_files[7][100];
    /**
     * @brief 
     * This field is unused, seems to be -1 in all legacy sprites.
     * Although it's copied to SpritePrototype, but later is replaced while the sound loading.
     */
    u32     sounds[7];                           

    u8      frames_number;                            // number of frames
    LegacySprAnimation animaatiot[20];         // animation sequences
    u8      animations_number;                       // number of animations
    u8      frame_rate;                         // frame rate
    u32     picture_frame_x;                             // x position of first frame
    u32     picture_frame_y;                             // y position of first frame
    u32     picture_frame_width;                  // frame width
    u32     picture_frame_height;                 // frame height
    u32     picture_frame_space;                    // space between frames //unused


    char    name[30];                           // name
    u32     width;                             // width
    u32     height;                            // height
    double  weight;                             // weight (for jump and switches)
    bool    enemy;                         // if sprite is a enemy
    u32     energy;                            //?sprite energy
    u32     damage;                            //?damage if it got hit
    u8      damage_type;                     // damage type
    u8      immunity_type;                            // immunity type
    u32     score;                            // how much score
    u32     AI[10];                             // AI type (max 10)
    u8      max_jump;                          // max jump time
    double  max_speed;                         // max speed
    u32     charge_time;                        // wait post shoot
    u8      color;                               // color
    bool    is_wall;                               // is a wall
    u32     how_destroyed;                       // how sprite is destroyed
    bool    can_open_locks;                              // can sprite open locks
    bool    vibrates;                            // vibrate sprite randomly
    u8      bonuses_number;                       // number of bonuses
    u32     attack1_time;                       // attack 1 duration (frames)
    u32     attack2_time;                       // attack 2 duration (frames)
    u32     parallax_type;                    // parallax type (just to TYPE_BACKGROUND)


    char    transformation_sprite[100];                 // another sprite that this sprite may change
    char    bonus_sprite[100];                  // bonus that this sprite gives
    char    ammo1_sprite[100];                 // ammo 1 sprite
    char    ammo2_sprite[100];                 // ammo 2 sprite


    bool    makes_sounds;                      //?make sounds?
    u32     sound_frequency;                           // sound frequency (def. 22050)
    bool    random_sound_frequency;                         // use random frequency?


    bool    is_wall_up;                          // if is wall at up
    bool    is_wall_down;                          // if is wall at down
    bool    is_wall_right;                      // if is wall at right
    bool    is_wall_left;                    // if is wall at left


    u8      effect;                             // sprite effect
    bool    is_transparent;                             // if it is transparent //unused
    u32     projectile_charge_time;                          //?charge_time inflicted by projectile sprites on the shooter
    bool    can_glide;                          // can drip quietly down?
    bool    boss;                               // if it is a boss //unused
    bool    bonus_always;                       // if not there is 1/4 chance of droping bonus
    bool    can_swim;                           // walk fast under water

};