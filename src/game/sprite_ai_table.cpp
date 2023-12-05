//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "sprite_ai_table.hpp"
#include "sprite_ai_functions.hpp"
#include "system.hpp"
#include "spriteclass.hpp"
#include "game.hpp"
#include "sprites.hpp"

namespace SpriteAI{

AI_Table AI_Table::INSTANCE;


void AI_Table::InitSpriteAIs(std::vector<AI_Class>& ai_vec, const std::vector<int>& ai_indices)const{
    if(ai_vec.size()>0)ai_vec.clear();

    for(const int& index: ai_indices){

        auto it = this->mTable.find(index);
        if(it!=this->mTable.end()){
            ai_vec.push_back(it->second);
        }
    }
};


void AI_Table::Init_AI(int id,
        int trigger,
        void (*func)(SpriteClass*),
        bool creatures,
        bool player,
        bool bonuses,
        bool backgrounds){

    AI_Class ai;

    ai.id = id;
    ai.trigger = trigger;
    ai.func = func;
    ai.apply_to_creatures = creatures;
    ai.apply_to_player = player;
    ai.apply_to_bonuses = bonuses;
    ai.apply_to_backgrounds = backgrounds;

    this->mTable[id] = ai;
    //sprite_ai_table[id] = ai;    
}


AI_Table::AI_Table(){

    /**
     * @brief
     * AIs triggered on each tick
     */
    
    Init_AI(AI_ROOSTER, AI_TRIGGER_ALIVE, AI_Functions::Rooster);
    Init_AI(AI_LITTLE_CHICKEN, AI_TRIGGER_ALIVE, AI_Functions::Rooster);
    Init_AI(AI_BLUE_FROG, AI_TRIGGER_ALIVE, AI_Functions::BlueFrog);
    Init_AI(AI_RED_FROG, AI_TRIGGER_ALIVE, AI_Functions::RedFrog);

    Init_AI(AI_EGG, AI_TRIGGER_ANYWAY, AI_Functions::Egg);
    Init_AI(AI_EGG2, AI_TRIGGER_ANYWAY, AI_Functions::Egg2);
    Init_AI(AI_PROJECTILE, AI_TRIGGER_ANYWAY, AI_Functions::Projectile);

    Init_AI(AI_JUMPER, AI_TRIGGER_ALIVE, AI_Functions::Jumper);

    Init_AI(AI_BASIC, AI_TRIGGER_ANYWAY, AI_Functions::Basic, true, true, true, true);

    Init_AI(AI_NONSTOP, AI_TRIGGER_ALIVE, AI_Functions::NonStop);
    
    Init_AI(AI_TURNING_HORIZONTALLY, AI_TRIGGER_ALIVE, AI_Functions::Turning_Horizontally);
    Init_AI(AI_TURNING_VERTICALLY, AI_TRIGGER_ALIVE, AI_Functions::Turning_Vertically);

    Init_AI(AI_LOOK_FOR_CLIFFS, AI_TRIGGER_ALIVE, AI_Functions::Look_For_Cliffs);
    Init_AI(AI_RANDOM_CHANGE_DIRECTION_H, AI_TRIGGER_ALIVE, AI_Functions::Random_Change_Dir_H);
    Init_AI(AI_RANDOM_TURNING, AI_TRIGGER_ALIVE, AI_Functions::Random_Turning);

    Init_AI(AI_RANDOM_JUMP, AI_TRIGGER_ALIVE, AI_Functions::Random_Jump);

    Init_AI(AI_FOLLOW_PLAYER, AI_TRIGGER_ALIVE, AI_Functions::Follow_Player);

    Init_AI(AI_FOLLOW_PLAYER_IF_IN_FRONT, AI_TRIGGER_ALIVE, AI_Functions::Follow_Player_If_Seen);

    Init_AI(AI_FOLLOW_PLAYER_VERT_HORI, AI_TRIGGER_ALIVE, AI_Functions::Follow_Player_Vert_Hori);

    Init_AI(AI_FOLLOW_PLAYER_IF_IN_FRONT_VERT_HORI, AI_TRIGGER_ALIVE, AI_Functions::Follow_Player_If_Seen_Vert_Hori);

    Init_AI(AI_RUN_AWAY_FROM_PLAYER, AI_TRIGGER_ALIVE, AI_Functions::Run_Away_From_Player);
    
    Init_AI(AI_SELF_DESTRUCTION, AI_TRIGGER_ALIVE, AI_Functions::SelfDestruction, true, false, true, true);

    Init_AI(AI_ATTACK_1_IF_DAMAGED, AI_TRIGGER_ALIVE, AI_Functions::Attack_1_If_Damaged); //TO DO Redesign
    Init_AI(AI_ATTACK_2_IF_DAMAGED, AI_TRIGGER_ALIVE, AI_Functions::Attack_2_If_Damaged); //TO DO Redesign

    Init_AI(AI_ATTACK_1_NONSTOP, AI_TRIGGER_ALIVE, AI_Functions::Attack_1_Nonstop);
    Init_AI(AI_ATTACK_2_NONSTOP, AI_TRIGGER_ALIVE, AI_Functions::Attack_2_Nonstop);

    Init_AI(AI_ATTACK_1_IF_PLAYER_IN_FRONT, AI_TRIGGER_ALIVE, AI_Functions::Attack_1_if_Player_in_Front); //TO DO Redesign
    Init_AI(AI_ATTACK_2_IF_PLAYER_IN_FRONT, AI_TRIGGER_ALIVE, AI_Functions::Attack_2_if_Player_in_Front); //TO DO Redesign

    Init_AI(AI_ATTACK_1_IF_PLAYER_BELOW, AI_TRIGGER_ALIVE, AI_Functions::Attack_1_if_Player_Bellow); //TO DO Redesign

    Init_AI(AI_ATTACK_1_IF_PLAYER_ABOVE, AI_TRIGGER_ALIVE, AI_Functions::Attack_1_If_Player_Above); //TO DO Redesign
    Init_AI(AI_ATTACK_2_IF_PLAYER_ABOVE, AI_TRIGGER_ALIVE, AI_Functions::Attack_2_If_Player_Above); //TO DO Redesign

    Init_AI(AI_TRANSFORM_IF_PLAYER_BELOW, AI_TRIGGER_ALIVE, AI_Functions::Transform_If_Player_Bellow); //TO DO Redesign
    Init_AI(AI_TRANSFORM_IF_PLAYER_ABOVE, AI_TRIGGER_ALIVE, AI_Functions::Transform_If_Player_Above); //TO DO Redesign
    
    Init_AI(AI_JUMP_IF_PLAYER_ABOVE, AI_TRIGGER_ALIVE, AI_Functions::Jump_If_Player_Above); //TO DO Redesign

    Init_AI(AI_DAMAGED_BY_WATER, AI_TRIGGER_ALIVE, AI_Functions::Damaged_by_Water, true, true);
    Init_AI(AI_KILL_EVERYONE, AI_TRIGGER_ALIVE, AI_Functions::Kill_Everyone);

    Init_AI(AI_FRICTION_EFFECT, AI_TRIGGER_ALIVE, AI_Functions::Friction_Effect);
    Init_AI(AI_HIDING, AI_TRIGGER_ALIVE, AI_Functions::Hiding);

    Init_AI(AI_RETURN_TO_ORIG_X, AI_TRIGGER_ALIVE, AI_Functions::Return_To_Orig_X);
    Init_AI(AI_RETURN_TO_ORIG_Y, AI_TRIGGER_ALIVE, AI_Functions::Return_To_Orig_Y);

    Init_AI(AI_MOVE_X_COS, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_X(cos_table(degree));});
    Init_AI(AI_MOVE_Y_COS, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_Y(cos_table(degree));});

    Init_AI(AI_MOVE_X_SIN, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_X(sin_table(degree));});
    Init_AI(AI_MOVE_Y_SIN, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_Y(sin_table(degree));});

    Init_AI(AI_MOVE_X_COS_FAST, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_X(cos_table(degree*2));});
    Init_AI(AI_MOVE_Y_SIN_FAST, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_Y(sin_table(degree*2));});

    Init_AI(AI_MOVE_X_COS_SLOW, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_X(cos_table(degree/2));});
    Init_AI(AI_MOVE_Y_SIN_SLOW, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_Y(sin_table(degree/2));});

    Init_AI(AI_MOVE_Y_SIN_FREE, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_Y(sin_table(s->action_timer/2));});
    Init_AI(AI_MOVE_X_COS_FREE, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_X(cos_table(s->action_timer/2));});
    Init_AI(AI_MOVE_Y_COS_FREE, AI_TRIGGER_ALIVE, [](SpriteClass*s){s->AI_Move_Y(cos_table(s->action_timer/2));});

    Init_AI(AI_TRANSFORM_WHEN_ENERGY_UNDER_2, AI_TRIGGER_ALIVE, AI_Functions::Transform_When_Energy_Under_2, true, true);

    Init_AI(AI_TRANSFORM_WHEN_ENERGY_OVER_1, AI_TRIGGER_ALIVE, AI_Functions::Transform_When_Energy_Over_1, true, true);

    Init_AI(AI_SELF_TRANSFORMATION, AI_TRIGGER_ALIVE, AI_Functions::Self_Transformation, true, true, true, true);

    Init_AI(AI_TRANSFORM_IF_DAMAGED, AI_TRIGGER_ALIVE, AI_Functions::Transform_If_Damaged, true, true); //TO DO Redesign

    Init_AI(AI_TELEPORT, AI_TRIGGER_ALIVE, AI_Functions::Teleporter);

    Init_AI(AI_CLIMBER, AI_TRIGGER_ALIVE, AI_Functions::Climber);
    Init_AI(AI_CLIMBER2, AI_TRIGGER_ALIVE, AI_Functions::Climber2);

    Init_AI(AI_DIE_IF_PARENT_NULL, AI_TRIGGER_ALIVE, AI_Functions::Die_If_Parent_Nullptr);

    Init_AI(AI_FALL_WHEN_SHAKEN, AI_TRIGGER_ALIVE, AI_Functions::Fall_When_Shaken, true, true, true, true); //TO DO Redesign

    Init_AI(AI_MOVE_DOWN_IF_SWITCH_1_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button1,0,1);});
    Init_AI(AI_MOVE_UP_IF_SWITCH_1_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button1,0,-1);});
    Init_AI(AI_MOVE_LEFT_IF_SWITCH_1_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button1,-1,0);});
    Init_AI(AI_MOVE_RIGHT_IF_SWITCH_1_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button1,1,0);});

    Init_AI(AI_MOVE_DOWN_IF_SWITCH_2_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button2,0,1);});
    Init_AI(AI_MOVE_UP_IF_SWITCH_2_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button2,0,-1);});
    Init_AI(AI_MOVE_LEFT_IF_SWITCH_2_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button2,-1,0);});
    Init_AI(AI_MOVE_RIGHT_IF_SWITCH_2_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button2,1,0);});

    Init_AI(AI_MOVE_DOWN_IF_SWITCH_3_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button3,0,1);});
    Init_AI(AI_MOVE_UP_IF_SWITCH_3_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button3,0,-1);});
    Init_AI(AI_MOVE_LEFT_IF_SWITCH_3_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button3,-1,0);});
    Init_AI(AI_MOVE_RIGHT_IF_SWITCH_3_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){AI_Functions::Move_If_Switch_Pressed(s, Game->button3,1,0);});

    Init_AI(AI_FALL_IF_SWITCH_1_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){if(Game->button1>0)s->initial_weight=1.5;});
    Init_AI(AI_FALL_IF_SWITCH_2_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){if(Game->button2>0)s->initial_weight=1.5;});
    Init_AI(AI_FALL_IF_SWITCH_2_PRESSED, AI_TRIGGER_ALIVE, [](SpriteClass*s){if(Game->button3>0)s->initial_weight=1.5;});

    Init_AI(AI_RANDOM_MOVE_VERT_HORI, AI_TRIGGER_ALIVE, AI_Functions::Random_Move_Vert_Hori);

    Init_AI(AI_TURN_BACK_IF_DAMAGED, AI_TRIGGER_ALIVE, AI_Functions::Turn_Back_If_Damaged);
    Init_AI(AI_DESTRUCTED_NEXT_TO_PLAYER, AI_TRIGGER_ALIVE, AI_Functions::Destructed_Next_To_Player);

    Init_AI(AI_FOLLOW_COMMANDS, AI_TRIGGER_ALIVE, AI_Functions::Follow_Commands);

    /**
     * @brief 
     * Infos
     */

    for(int id=AI_INFOS_BEGIN;id<=AI_INFOS_END;++id){
        AI_Class ai;
        ai.id = id;
        ai.info_id = id - AI_INFOS_BEGIN + 1;
        ai.apply_to_creatures = true;
        ai.apply_to_bonuses = false;
        ai.apply_to_backgrounds = false;
        ai.apply_to_player = false;

        this->mTable[id] = ai;
    }

    /**
     * @brief
     * AIs triggered on death
     * 
     */


    Init_AI(AI_EVIL_ONE, AI_TRIGGER_DEATH, AI_Functions::EvilOne);
    Init_AI(AI_REBORN, AI_TRIGGER_DEATH, AI_Functions::Reborn);
    Init_AI(AI_CHICK, AI_TRIGGER_DEATH, AI_Functions::Chick);
    Init_AI(AI_CHANGE_SKULL_BLOCKS_IF_DEAD, AI_TRIGGER_DEATH, [](SpriteClass*s){
        Game->Change_SkullBlocks();
    });

    /**
     * @brief 
     * AIs triggered on damage
     * 
     */

    Init_AI(AI_CHANGE_SKULL_BLOCKS_IF_DAMAGED, AI_TRIGGER_DAMAGE, [](SpriteClass*s){
        Game->Change_SkullBlocks();
    });
}

}