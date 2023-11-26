//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
/**
 * @brief 
 * Experimental commands/waypoints AI by SaturninTheAlien
 */

#include <sstream>
#include "spriteclass_commands.hpp"
#include "spriteclass.hpp"
#include "exceptions.hpp"
#include "sprites.hpp"
#include "sfx.hpp"

namespace SpriteCommands{


class WaypointX:public Command{
public:
    WaypointX(double target_x):target_x(target_x){};
    ~WaypointX()=default;
    bool execute(SpriteClass*sprite, SpriteClass*);
private:
    double target_x;
};

bool WaypointX::execute(SpriteClass*sprite, SpriteClass*){

    double max_speed = sprite->prototype->max_speed / 3.5;
    double dx = sprite->x - this->target_x;
    
    
    if(dx*dx <= max_speed*max_speed){
        //Waypoint reached, align
        sprite->a = 0;
        sprite->x = this->target_x;
        return true;
    }
    else if(sprite->x > this->target_x){
        sprite->flip_x = true;
        sprite->a = -max_speed;
    }
    else{
        sprite->flip_x = false;
        sprite->a = max_speed;
    }

    return false;
}

class WaypointY:public Command{
public:
    WaypointY(double target_y):target_y(target_y){};
    ~WaypointY()=default;
    bool execute(SpriteClass*sprite, SpriteClass*);
private:
    double target_y;
};

bool WaypointY::execute(SpriteClass*sprite, SpriteClass*){
    double max_speed = sprite->prototype->max_speed / 3.5;
    double dy = sprite->y - this->target_y;

    if(dy*dy <= max_speed*max_speed){
        //Waypoint reached, align
        sprite->b = 0;
        sprite->y = this->target_y;
        return true;
    }
    else if(sprite->y > this->target_y){
        sprite->b = -max_speed;
    }
    else{
        sprite->b = max_speed;
    }


    return false;
}

class Waypoint: public Command{
public:
    Waypoint(double target_x, double target_y):target_x(target_x), target_y(target_y){};
    ~Waypoint()=default;
    bool execute(SpriteClass*sprite, SpriteClass*);
private:
    double target_x, target_y;
};


bool waypoint_xy_helper(SpriteClass*sprite, double target_x, double target_y){
    double velocity = sprite->prototype->max_speed / 3.5;
    double dx = sprite->x - target_x;
    double dy = sprite->y - target_y;

    double eps2 = dx*dx + dy*dy; 

    if(eps2 <= velocity*velocity){
        //Waypoint reached, align
        sprite->a = 0;
        sprite->b = 0;
        sprite->x = target_x;
        sprite->y = target_y;
        return true;
    }
    else{
        double z = sqrt(eps2);
        sprite->a = -velocity * dx / z;
        sprite->b = -velocity * dy / z;
        sprite->flip_x = dx>0;
    }

    return false;
}


bool Waypoint::execute(SpriteClass*sprite, SpriteClass*){
    return waypoint_xy_helper(sprite, this->target_x, this->target_y);
}


class WaypointSeenPlayer: public Command{
public:
    bool execute(SpriteClass*sprite, SpriteClass*);
};

bool WaypointSeenPlayer::execute(SpriteClass*sprite, SpriteClass*player){
    //if(player==nullptr)return true;

    bool success = true;
    if(player!=nullptr){
        if(sprite->seen_player_x==-1){
            sprite->seen_player_x = player->x;
            sprite->seen_player_y = player->y;
        }

        success = waypoint_xy_helper(sprite, sprite->seen_player_x, sprite->seen_player_y);
    }

    if(success){
        sprite->seen_player_x = -1;
        sprite->seen_player_y = -1;
    }

    return success;
}


class WaypointOrigXY: public Command{
public:
    bool execute(SpriteClass*sprite, SpriteClass*);
};


bool WaypointOrigXY::execute(SpriteClass*sprite, SpriteClass*){
    return waypoint_xy_helper(sprite, sprite->orig_x, sprite->orig_y);
}


class TransformationCommand: public Command{
public:
    bool execute(SpriteClass*sprite, SpriteClass*);
};


bool TransformationCommand::execute(SpriteClass*sprite, SpriteClass*){
    PrototypeClass* transformation = sprite->prototype->transformation;   

    if(transformation!=nullptr){
        sprite->prototype = transformation;
        sprite->initial_weight = transformation->weight;
        sprite->animation_index = 0;
        
        sprite->ammo1 = transformation->ammo1;
        sprite->ammo2 = transformation->ammo2;
        
        sprite->enemy = transformation->enemy;
    }
    //skip command if transformation is nullptr
    return true;
};

class SelfDestructionCommand: public Command{
public:
    bool execute(SpriteClass*sprite, SpriteClass*);
};

bool SelfDestructionCommand::execute(SpriteClass*sprite, SpriteClass*){
    sprite->damage_taken = sprite->energy;
	sprite->damage_taken_type = DAMAGE_ALL;
    return false;
}


class MakeSoundCommand: public Command{
public:
    MakeSoundCommand(int sound_index):sound_index(sound_index){}
    bool execute(SpriteClass*sprite, SpriteClass*);
private:
    int sound_index = -1;
};

bool MakeSoundCommand::execute(SpriteClass*sprite, SpriteClass*){
    Play_GameSFX(sprite->prototype->sounds[this->sound_index],100, (int)sprite->x, (int)sprite->y,
    sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);
    return true;
}


class WaitCommand: public Command{
public:
    WaitCommand(int wait_time): wait_time(wait_time){};
    bool execute(SpriteClass*sprite, SpriteClass*);
private:
    int wait_time = 0;
};

bool WaitCommand::execute(SpriteClass*sprite, SpriteClass*){
    if(sprite->command_timer==-1){
        sprite->command_timer = this->wait_time;
    }

    if(sprite->command_timer==0){
        sprite->command_timer=-1;
        return true;
    }
    else{
        --sprite->command_timer;
        return false;
    }
}

double getCommandXPos(const nlohmann::json& j, int prototypeWidth){
    return j.get<double>() * 32 + ((double)prototypeWidth)/2;
}

double getCommandYPos(const nlohmann::json& j, int prototypeHeight){
    return j.get<double>() * 32 + ((double)prototypeHeight)/2;
}

void Parse_Commands(const nlohmann::json& j_in, std::vector<Command*>& commands_v, int prototypeWidth, int prototypeHeight){
    if(!j_in.is_array()){
        throw PExcept::PException("Commands field has to be an array!");
    }

    int state = 0;
    double target_x = 0., target_y = 0.;
    for(const nlohmann::json& j: j_in){
        switch (state)
        {
        case 0:
            if(j.is_string()){
                std::string command_name = j.get<std::string>();
                if(command_name=="waypoint_x"){
                    state = 1;
                }
                else if(command_name=="waypoint_y"){
                    state = 2;
                }
                else if(command_name=="waypoint_xy"){
                    state = 3;
                }
                else if(command_name=="waypoint_orig_xy"){
                    commands_v.push_back(new WaypointOrigXY());
                }
                else if(command_name=="waypoint_seen_player"){
                    commands_v.push_back(new WaypointSeenPlayer());
                }
                else if(command_name=="make_sound"){
                    state = 5;
                }
                else if(command_name=="die" || command_name=="self_destruction"){
                    commands_v.push_back(new SelfDestructionCommand());
                }
                else if(command_name=="transform"){
                    commands_v.push_back(new TransformationCommand());
                }
                else if(command_name=="wait"){
                    state = 6;
                }
            }
            break;
        case 1:
            if(j.is_number()){
                target_x = getCommandXPos(j, prototypeWidth);
                commands_v.push_back(new WaypointX(target_x));
            }
            state = 0;
            break;
        case 2:
            if(j.is_number()){
                target_y = getCommandYPos(j, prototypeHeight);
                commands_v.push_back(new WaypointY(target_y));
            }
            state = 0;
            break;
        
        case 3:
            if(j.is_number()){
                target_x = getCommandXPos(j, prototypeWidth);
                state = 4;
            }
            else{
                state = 0;
            }
            break;
        case 4:
            if(j.is_number()){
                target_y = getCommandYPos(j, prototypeHeight);
                commands_v.push_back(new Waypoint(target_x, target_y));
            }
            state=0;
            break;

        case 5:
            if(j.is_string()){
                auto it = PrototypeClass::SoundTypesDict.find(j.get<std::string>());
                if(it!=PrototypeClass::SoundTypesDict.end()){
                    if(it->second>=SPRITE_SOUNDS_NUMBER){
                        std::ostringstream os;
                        os<<"Sprite sound index: "<<it->second<<" out of array";
                        throw PExcept::PException(os.str());
                    }
                    commands_v.push_back(new MakeSoundCommand(it->second));
                }
            }

            state=0;
            break;
        case 6:
            if(j.is_number_integer()){
                int wait_time = j.get<int>();
                commands_v.push_back(new WaitCommand(wait_time));
            }
            state=0;
            break;
        default:
            break;
        }
    }
}

}