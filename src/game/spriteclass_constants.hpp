//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

/**
 * @brief 
 * Sprite constants.
 */

#pragma once

#define PK2SPRITE_CURRENT_VERSION "2.0"
#define MAX_PROTOTYYPPEJA  256

#define SPRITE_SOUNDS_NUMBER     7
#define SPRITE_SOUNDS_NUMBER_LEGACY     5

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
    AI_NONE, //0
    AI_ROOSTER, //1
    AI_EGG,  //2
    AI_LITTLE_CHICKEN,  //3
    AI_BONUS, //4
    AI_JUMPER, //5
    AI_BASIC, //6
    AI_TURNING_HORIZONTALLY, //7
    AI_LOOK_FOR_CLIFFS, //8
    AI_RANDOM_CHANGE_DIRECTION_H, //9
    AI_RANDOM_JUMP, //10
    AI_FOLLOW_PLAYER, //11
    AI_RANDOM_START_DIRECTION, //12
    AI_FOLLOW_PLAYER_IF_IN_FRONT, //13
    AI_TRANSFORM_WHEN_ENERGY_UNDER_2, //14
    AI_TRANSFORM_WHEN_ENERGY_OVER_1, //15
    AI_START_DIRECTIONS_TOWARDS_PLAYER, //16
    AI_PROJECTILE, //17
    AI_NONSTOP, //18
    AI_ATTACK_1_IF_DAMAGED, //19
    AI_SELF_DESTRUCTION, //20
    AI_ATTACK_1_IF_PLAYER_IN_FRONT, //21
    AI_ATTACK_1_IF_PLAYER_BELOW, //22
    AI_DAMAGED_BY_WATER, //23
    AI_ATTACK_2_IF_PLAYER_IN_FRONT, //24
    AI_KILL_EVERYONE, //25
    AI_FRICTION_EFFECT, //26
    AI_HIDING, //27     // What's the purpose of this AI? (wolf, red wolf)
    AI_RETURN_TO_ORIG_X, //28
    AI_RETURN_TO_ORIG_Y, //29
    AI_TELEPORT, //30

    /**
     * @brief 
     * "Greta Engine", new AIs: 31 - 34
     * 
     */

    AI_ATTACK_1_IF_PLAYER_ABOVE, //31
    AI_ATTACK_2_IF_PLAYER_ABOVE, //32
    AI_TRANSFORM_IF_PLAYER_BELOW, //33
    AI_TRANSFORM_IF_PLAYER_ABOVE, //34


    AI_THROWABLE_WEAPON = 35,
    AI_FALL_WHEN_SHAKEN, //36
    AI_CHANGE_SKULL_BLOCKS_IF_DEAD, //37
    AI_CHANGE_SKULL_BLOCKS_IF_DAMAGED, //38
    AI_DIE_WITH_MOTHER_SPPRITE, //39

    /**
     * @brief 
     * "Greta Engine" commands/waypoints AI
     */
    AI_FOLLOW_COMMANDS = 40,

    AI_MOVE_X_COS = 41,
    AI_MOVE_Y_COS, //42
    AI_MOVE_X_SIN, //43
    AI_MOVE_Y_SIN, //44
    AI_MOVE_X_COS_FAST, //45
    AI_MOVE_Y_SIN_FAST, //46
    AI_MOVE_X_COS_SLOW, //47
    AI_MOVE_Y_SIN_SLOW, //48
    AI_MOVE_Y_SIN_FREE, //49

    AI_RANDOM_TURNING, //50
    AI_JUMP_IF_PLAYER_ABOVE, //51
    AI_SELF_TRANSFORMATION, //52

    AI_FALL_IF_SWITCH_1_PRESSED, //53
    AI_FALL_IF_SWITCH_2_PRESSED, //54
    AI_FALL_IF_SWITCH_3_PRESSED, //55

    AI_MOVE_DOWN_IF_SWITCH_1_PRESSED, //56
    AI_MOVE_UP_IF_SWITCH_1_PRESSED, //57
    AI_MOVE_LEFT_IF_SWITCH_1_PRESSED, //58
    AI_MOVE_RIGHT_IF_SWITCH_1_PRESSED, //59
    AI_MOVE_DOWN_IF_SWITCH_2_PRESSED, //60
    AI_MOVE_UP_IF_SWITCH_2_PRESSED, //61
    AI_MOVE_LEFT_IF_SWITCH_2_PRESSED, //62
    AI_MOVE_RIGHT_IF_SWITCH_2_PRESSED, //63
    AI_MOVE_DOWN_IF_SWITCH_3_PRESSED, //64
    AI_MOVE_UP_IF_SWITCH_3_PRESSED, //65
    AI_MOVE_LEFT_IF_SWITCH_3_PRESSED, //66
    AI_MOVE_RIGHT_IF_SWITCH_3_PRESSED, //67

    AI_MOVE_X_COS_FREE, //68,
    AI_MOVE_Y_COS_FREE, //69

    AI_TURNING_VERTICALLY = 70,
    AI_RANDOM_START_DIRECTION_VERT, //71
    AI_START_DIRECTIONS_TOWARDS_PLAYER_VERT, //72
    AI_CLIMBER, //73
    AI_CLIMBER2, //74
    AI_RUN_AWAY_FROM_PLAYER, //75
    AI_REBORN, //76

    AI_ARROW_LEFT, //77
    AI_ARROW_RIGHT, //78
    AI_ARROW_UP, //79
    AI_ARROW_DOWN, //80
    AI_ARROW_BARRIER, //81

    /**
     * @brief 
     * "Greta Engine" new AIs
     */
    AI_TRANSFORM_IF_SKULL_BLOCKS_CHANGED, //82
    AI_DIE_IF_SKULL_BLOCKS_CHANGED, //83
    AI_RETURN_TO_ORIG_X_CONSTANT, //84
    AI_RETURN_TO_ORIG_Y_CONSTANT, //85

    /**
     * @brief 
     * event AIs
     */

    AI_EMIT_EVENT1_IF_DEAD, // 86,
    AI_EMIT_EVENT1_IF_DAMAGED, // 87,
    AI_TRANSFORM_IF_EVENT1, // 88
    AI_DIE_IF_EVENT1, // 89 

    AI_EMIT_EVENT2_IF_DEAD, // 90,
    AI_EMIT_EVENT2_IF_DAMAGED, // 91,   
    AI_TRANSFORM_IF_EVENT2, // 92
    AI_DIE_IF_EVENT2, // 93


    // AI_82 - AI_100

    AI_BACKGROUND_MOON = 101,
    AI_BACKGROUND_MOVE_LEFT, // 102  // unused
    AI_BACKGROUND_MOVE_RIGHT, // 103 // unused
    AI_BACKGROUND_HORIZONTAL_PARALLAX, // 104,

    AI_BACKGROUND_BRING_TO_FRONT, // 105,
    AI_BACKGROUND_SEND_TO_BACK, //106

    // AI_104 - AI_119

    AI_BONUS_CLOCK = 120,
    AI_BONUS_INVISIBILITY, // 121
    AI_BONUS_SUPERHYPPY, // 122     // unused
    AI_BONUS_SUPERTULITUS, // 123   // unused
    AI_BONUS_SUPERVAUHTI, // 124    // unused    

    // New AI
    AI_BONUS_SUPERMODE, // 125

    // AI_126 - AI_128

    AI_TRANSFORM_IF_DAMAGED = 129,
    AI_FOLLOW_PLAYER_VERT_HORI, //130
    AI_FOLLOW_PLAYER_IF_IN_FRONT_VERT_HORI, //131
    AI_RANDOM_MOVE_VERT_HORI, //132
    AI_BLUE_FROG, //133
    AI_RED_FROG, //134
    AI_FROG_3_UNUSED, //135   //unused
    AI_ATTACK_2_IF_DAMAGED, //136
    AI_ATTACK_1_NONSTOP, //137
    AI_ATTACK_2_NONSTOP, //138
    AI_TURN_BACK_IF_DAMAGED, //139
    AI_EVIL_ONE, //140
    AI_INFINITE_ENERGY, //141
    AI_THROWABLE_WEAPON2, //142,
    AI_STATIC_PROJECTILE, //143
    AI_DIE_IF_TOUCHES_WALL, //144
    AI_NPC_COLLECT_BONUSES, //145

    // AI_145 - AI_149

    // New AI
    AI_MAX_SPEED_PLAYER = 150,
    AI_MAX_SPEED_PLAYER_ON_SUPER, //151
    AI_SWIMMING, //152
    AI_MAX_SPEED_SWIMMING, //153

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
    TYPE_BACKGROUND,
    TYPE_FOREGROUND

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

    FX_DESTRUCT_INDESTRUCTIBLE,
    FX_DESTRUCT_FEATHERS,
    FX_DESTRUCT_STARS_GRAY,
    FX_DESTRUCT_STARS_BLUE,
    FX_DESTRUCT_STARS_RED,
    FX_DESTRUCT_STARS_GREEN,
    FX_DESTRUCT_STARS_ORANGE,
    FX_DESTRUCT_STARS_VIOLET,
    FX_DESTRUCT_STARS_TURQUOISE,
    FX_DESTRUCT_EXPLOSION_GRAY,
    FX_DESTRUCT_EXPLOSION_BLUE,
    FX_DESTRUCT_EXPLOSION_RED,
    FX_DESTRUCT_EXPLOSION_GREEN,
    FX_DESTRUCT_EXPLOSION_ORANGE,
    FX_DESTRUCT_EXPLOSION_VIOLET,
    FX_DESTRUCT_EXPLOSION_TURQUOISE,
    FX_DESTRUCT_SMOKE_GRAY,
    FX_DESTRUCT_SMOKE_BLUE,
    FX_DESTRUCT_SMOKE_RED,
    FX_DESTRUCT_SMOKE_GREEN,
    FX_DESTRUCT_SMOKE_ORANGE,
    FX_DESTRUCT_SMOKE_VIOLET,
    FX_DESTRUCT_SMOKE_TURQUOISE,
    FX_DESTRUCT_SMOKECLOUDS,
    FX_DESTRUCT_ANIMATED = 100

};

enum {

    BONUSITEM_NOTHING,
    BONUSITEM_KEY,
    BONUSITEM_SCORE

};