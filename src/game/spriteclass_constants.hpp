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

    AI_TRANSFORM_IF_KEYLOCKS_OPENED, //94,
    AI_DIE_IF_KEYLOCKS_OPENED, //95

    AI_EMIT_EVENT1_IF_BONUS_COLLECTED, //96
    AI_EMIT_EVENT2_IF_BONUS_COLLECTED, //97
    AI_CHANGE_SKULLS_IF_BONUS_COLLECTED, //98


    // AI_99 - AI_100

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

    AI_BONUS_SUPERMODE, // 125
    AI_BONUS_CANNOT_BE_PUSHED, // 126

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
    AI_DESPAWN_IF_OFFSCREEN, //146,
    AI_HEAL_IF_OFFSCREEN, //147
    AI_SELF_TRANSFORMATION_RANDOM_PROTOTYPE, //148
    AI_JUMP_IF_PLAYER_IN_FRONT, //149

    AI_MAX_SPEED_PLAYER = 150,
    AI_MAX_SPEED_PLAYER_ON_SUPER, //151
    AI_SWIMMING, //152
    AI_MAX_SPEED_SWIMMING, //153

    AI_LIMITED_PLAYER_CONTROL, //154
    AI_VERY_LIMITED_PLAYER_CONTROL, //155

    AI_NO_CONTACT_ATTACK_ANIMATION, //156

    AI_PROJECTILE_AIM_RANDOMLY, //157
    AI_PROJECTILE_AIM_AT_PLAYER, //158

    AI_FOLLOW_PLAYER_DIAGONALLY, //159
    AI_ATTACK_1_IF_PLAYER_NEARBY, //160
    AI_ATTACK_2_IF_PLAYER_NEARBY, //161

    AI_TRANSFORM_IF_TOUCHES_FLOOR, //162
    AI_TRANSFORM_IF_TOUCHES_WALL, //163
    AI_TRANSFORM_IF_PLAYER_IN_FRONT, //164

    

    AI_EGG2 = 170, 

    AI_NONSTOP_VERTICAL, //171

    AI_START_DOWN, //172
    AI_START_UP, //173
    AI_START_LEFT, //174
    AI_START_RIGHT, //175

    AI_LOOK_AT_PLAYER, //176
    AI_RUN_AWAY_FROM_SUPERMODE_PLAYER, //177
      

    // AI_154 - AI_200

    /**
     * @brief 
     * Obsolete, legacy info AIs,
     * Automatically translated to AI_INFO_IF_TOUCHES_PLAYER (302) while sprites loading
     */
    AI_LEGACY_INFOS_BEGIN = 201,
    AI_LEGACY_INFOS_END = 301,

    /**
     * @brief 
     * New info AIs
     */
    AI_INFO_IF_TOUCHES_PLAYER, // 302,
    AI_INFO_IF_DAMAGED, // 303
    AI_INFO_IF_DEAD, // 304
    AI_INFO_IF_BONUS_COLLECTED, // 305


    AI_CHICK = 400,
    AI_CHICKBOX, //401
    AI_DESTRUCTED_NEXT_TO_PLAYER, //402
    AI_TRANSFORM_IF_DEAD, //403
    AI_WIENER_PROCESS, //404

};

enum { //Damage

    DAMAGE_SELF_DESTRUCTION = -2,
    DAMAGE_WEAK_SELF_DESTRUCTION = -1,

    DAMAGE_NONE = 0, //0
    DAMAGE_IMPACT, //1
    DAMAGE_DROP, //2
    DAMAGE_NOISE, //3
    DAMAGE_FIRE, //4
    DAMAGE_WATER, // 5
    DAMAGE_SNOW, //6
    DAMAGE_BONUS, //7
    DAMAGE_ELECTRIC, //8
    DAMAGE_MAGIC, //9
    DAMAGE_COMPRESSION, //10
    DAMAGE_SMELL, //11
    DAMAGE_SUPERMODE, //12
    DAMAGE_STITCH, //13
    DAMAGE_INSTANT, //14
};

enum {

    TYPE_NOTHING, //0
    TYPE_GAME_CHARACTER, //1
    TYPE_BONUS, //2
    TYPE_PROJECTILE, //3
    TYPE_TELEPORT, //4
    TYPE_BACKGROUND, //5
    TYPE_FOREGROUND, //6

    TYPE_CHECKPOINT, //7 Reserved, not implemented
    TYPE_EXIT, //8 Reserved, not implemented

    TYPE_BLACK_HOLE, //9

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

    FX_DESTRUCT_NO_EFFECT, //0
    FX_DESTRUCT_FEATHERS, //1
    FX_DESTRUCT_STARS_GRAY, //2
    FX_DESTRUCT_STARS_BLUE, //3
    FX_DESTRUCT_STARS_RED, //4
    FX_DESTRUCT_STARS_GREEN, //5
    FX_DESTRUCT_STARS_ORANGE, //6
    FX_DESTRUCT_STARS_VIOLET, //7
    FX_DESTRUCT_STARS_TURQUOISE, //8
    FX_DESTRUCT_EXPLOSION_GRAY, //9
    FX_DESTRUCT_EXPLOSION_BLUE, //10
    FX_DESTRUCT_EXPLOSION_RED, //11
    FX_DESTRUCT_EXPLOSION_GREEN, //12
    FX_DESTRUCT_EXPLOSION_ORANGE, //13
    FX_DESTRUCT_EXPLOSION_VIOLET, //14
    FX_DESTRUCT_EXPLOSION_TURQUOISE, //15
    FX_DESTRUCT_SMOKE_GRAY, //16
    FX_DESTRUCT_SMOKE_BLUE, //17
    FX_DESTRUCT_SMOKE_RED, //18
    FX_DESTRUCT_SMOKE_GREEN, //19
    FX_DESTRUCT_SMOKE_ORANGE, //20
    FX_DESTRUCT_SMOKE_VIOLET, //21
    FX_DESTRUCT_SMOKE_TURQUOISE, //22
    FX_DESTRUCT_SMOKECLOUDS, //23

    /**
     * @brief 
     * New GE destruction effects 
     */
    FX_DESTRUCT_SPLASH_GRAY, //24
    FX_DESTRUCT_SPLASH_BLUE, //25
    FX_DESTRUCT_SPLASH_RED, //26,
    FX_DESTRUCT_SPLASH_GREEN, //27,
    FX_DESTRUCT_SPLASH_ORANGE, //28,
    FX_DESTRUCT_SPLASH_VIOLET, //29
    FX_DESTRUCT_SPLASH_TURQUOISE, //30

    FX_DESTRUCT_SMOKELESS_EXPLOSION_GRAY, //31
    FX_DESTRUCT_SMOKELESS_EXPLOSION_BLUE, //32
    FX_DESTRUCT_SMOKELESS_EXPLOSION_RED, //33
    FX_DESTRUCT_SMOKELESS_EXPLOSION_GREEN, //34
    FX_DESTRUCT_SMOKELESS_EXPLOSION_ORANGE, //35
    FX_DESTRUCT_SMOKELESS_EXPLOSION_VIOLET, //36
    FX_DESTRUCT_SMOKELESS_EXPLOSION_TURQUOISE, //37
    
    /**
     * @brief 
     * Animated
     */
    FX_DESTRUCT_ANIMATED = 100

};

enum {

    BONUSITEM_NOTHING,
    BONUSITEM_KEY,
    BONUSITEM_SCORE

};