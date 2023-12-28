//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include "spriteclass_constants.hpp"


class SpriteClass;

enum AI_TRIGGER{  
    AI_TRIGGER_NONE,
    AI_TRIGGER_ANYWAY, //on each game tick 
    AI_TRIGGER_ALIVE, // on each game tick if sprite->energy>0


    AI_TRIGGER_DEATH,  //if the sprite dies
    AI_TRIGGER_DAMAGE, //if the sprite is damaged
    AI_TRIGGER_GAME_START, // when the game starts

    AI_TRIGGER_SPAWN,  //on spawn

    AI_TRIGGER_SKULLS_CHANGED, //if skull blocks are changed
    AI_TRIGGER_EVENT1, //if event1
    AI_TRIGGER_EVENT2, //if event2

    AI_TRIGGER_BONUS_COLLECTED, //if bonus sprite is collected

    //considering them
    AI_TRIGGER_PLAYER_ABOVE,
    AI_TRIGGER_PLAYER_BELOW,
    AI_TRIGGER_PLAYER_IN_FRONT,
};

namespace AI_Functions{
    extern SpriteClass* player;
    extern SpriteClass* player_invisible;
}

namespace SpriteAI{


class AI_Class{
public:
    AI_Class() = default;
    int id = AI_NONE;
    int trigger = AI_TRIGGER_NONE;
    std::function<void(SpriteClass* sprite)> func;

    bool apply_to_player = false;
    bool apply_to_creatures = false;
    bool apply_to_bonuses = false;
    bool apply_to_backgrounds = false;

    int info_id = -1;
};

/*
AI Table, singleton
*/
class AI_Table{
public:
    static AI_Table INSTANCE;

    void InitSpriteAIs(std::vector<AI_Class>& ai_vec, const std::vector<int>& ai_indices)const;

    AI_Table(const AI_Table& src) = delete;
    AI_Table& operator=(const AI_Table& src) = delete;
private:
    AI_Table();
    void Init_AI(int id,
        int trigger,
        void (*func)(SpriteClass*),
        bool creatures=true,
        bool player=false,
        bool bonuses=false,
        bool backgrounds=false);


    std::map<int, AI_Class> mTable;
};
//extern std::map<int, AI_Class> sprite_ai_table;

}