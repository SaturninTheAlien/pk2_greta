//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "game/sprites.hpp"

#include "engine/PSound.hpp"
#include "engine/PUtils.hpp"
#include "engine/PLog.hpp"

#include "game/game.hpp"
#include "episode/episodeclass.hpp"
#include "physics.hpp"
#include "system.hpp"

#include <algorithm>
#include <cstring>
#include <vector>

PrototypeClass* Level_Prototypes_List[MAX_PROTOTYYPPEJA] = {nullptr};

std::list<SpriteClass*> Sprites_List;
std::list<SpriteClass*> bgSprites_List;
SpriteClass* Player_Sprite = nullptr;

void Level_Prototypes_ClearAll() {

	for (int i = 0; i < MAX_PROTOTYYPPEJA; i++) {
		Level_Prototypes_List[i] = nullptr;
	}
}

PrototypeClass* Level_Prototypes_Get(int index){
	if(index<0 || index>= MAX_PROTOTYYPPEJA) return nullptr;
	else return Level_Prototypes_List[index];
}


PrototypeClass* Level_Prototypes_get(const std::string& name, int id) {

	if (id<0 || id >= MAX_PROTOTYYPPEJA) {
		PLog::Write(PLog::ERR, "PK2", "Invalid prototype id");
		return nullptr;
	}

	PrototypeClass* protot = Prototype_Load(name);
	Level_Prototypes_List[id] = protot;
	return protot;
}

int Level_Prototypes_LoadAll() {

	for (u32 i = 0; i < PK2MAP_MAP_MAX_PROTOTYPES; i++) {
		if (strcmp(Game->map.sprite_filenames[i], "") != 0) {

			PFile::Path path = Episode->Get_Dir(Game->map.sprite_filenames[i]);
			if (Level_Prototypes_get(Game->map.sprite_filenames[i], i) == nullptr) {
				PLog::Write(PLog::WARN, "PK2", "Can't load sprite %s. It will not appear", Game->map.sprite_filenames[i]);
			}
		}
	}
	return 0;
}

void Sprites_add_bg(SpriteClass* sprite) {

	bgSprites_List.push_back(sprite);

}

bool Compare_bgSprites(SpriteClass* s1, SpriteClass* s2) {

	return (s1->prototype->parallax_type < s2->prototype->parallax_type); 
	
}

void Sprites_sort_bg() {
	
	bgSprites_List.sort(Compare_bgSprites);

}

void Sprites_start_directions() {
	for (SpriteClass* sprite : Sprites_List) {
		sprite->a = 0;

		for(const int& ai: sprite->prototype->AI_v){
			switch (ai)
			{
			case AI_RANDOM_START_DIRECTION:{
				while (sprite->a == 0) {
					sprite->a = ((rand()%2 - rand()%2) * sprite->prototype->max_speed) / 3.5;//2;
				}
			}
			break;

			case AI_RANDOM_START_DIRECTION_VERT:{
				while (sprite->b == 0) {
					sprite->b = ((rand()%2 - rand()%2) * sprite->prototype->max_speed) / 3.5;//2;
				}
			}
			break;

			case AI_START_DIRECTIONS_TOWARDS_PLAYER:{

				if (sprite->x < Player_Sprite->x)
					sprite->a = sprite->prototype->max_speed / 3.5;

				if (sprite->x > Player_Sprite->x)
					sprite->a = (sprite->prototype->max_speed * -1) / 3.5;
			}

			break;

			case AI_START_DIRECTIONS_TOWARDS_PLAYER_VERT:{
				if (sprite->y < Player_Sprite->y)
					sprite->b = sprite->prototype->max_speed / -3.5;

				if (sprite->y > Player_Sprite->y)
					sprite->b = sprite->prototype->max_speed / 3.5;
			}
			
			default:
				break;
			}
		}

		/*if (sprite->HasAI(AI_RANDOM_START_DIRECTION)){
			while (sprite->a == 0) {
				sprite->a = ((rand()%2 - rand()%2) * sprite->prototype->max_speed) / 3.5;//2;
			}
		}

		if (sprite->HasAI(AI_RANDOM_START_DIRECTION_VERT)){
			while (sprite->b == 0) {
				sprite->b = ((rand()%2 - rand()%2) * sprite->prototype->max_speed) / 3.5;//2;
			}
		}

		if (sprite->HasAI(AI_START_DIRECTIONS_TOWARDS_PLAYER)){

			if (sprite->x < Player_Sprite->x)
				sprite->a = sprite->prototype->max_speed / 3.5;

			if (sprite->x > Player_Sprite->x)
				sprite->a = (sprite->prototype->max_speed * -1) / 3.5;
		}

		if (sprite->HasAI(AI_START_DIRECTIONS_TOWARDS_PLAYER_VERT)){

			if (sprite->y < Player_Sprite->y)
				sprite->b = sprite->prototype->max_speed / -3.5;

			if (sprite->y > Player_Sprite->y)
				sprite->b = sprite->prototype->max_speed / 3.5;
		}*/
	}
}

void Sprites_add(PrototypeClass* protot, int is_Player_Sprite, double x, double y, SpriteClass* emo, bool isbonus) {

	SpriteClass* sprite = new SpriteClass(protot, is_Player_Sprite, x, y);
	Sprites_List.push_back(sprite);

	if (is_Player_Sprite) Player_Sprite = sprite;

	if(isbonus) { //If it is a bonus dropped by enemy
		sprite->orig_x = sprite->x;
		sprite->orig_y = sprite->y;
		sprite->jump_timer = 1;
		sprite->a = 3 - rand()%7;
		sprite->damage_timer = 35;//25

	} else {

		sprite->x = x + 16 + 1;
		sprite->y += sprite->prototype->height/2;
		sprite->orig_x = sprite->x;
		sprite->orig_y = sprite->y;
		
	}

	if (protot->type == TYPE_BACKGROUND)
		Sprites_add_bg(sprite);


	sprite->parent_sprite = emo;

	/*if (emo != nullptr)
		sprite->parent_sprite = emo;
	else
		sprite->parent_sprite = sprite;*/
	
}

void Sprites_add_ammo(PrototypeClass* protot, double x, double y, SpriteClass* emo) {

	//SpriteClass(proto, is_Player_Sprite,false,x-proto->width/2,y);
	SpriteClass* sprite = new SpriteClass(protot, false, x, y);
	Sprites_List.push_back(sprite);

	//sprite->x += sprite->prototype->width;
	//sprite->y += sprite->prototype->height/2;

	if (protot->HasAI(AI_THROWABLE_WEAPON)){
		if ((int)emo->a == 0){
			// If the "shooter" is a player or the speed of the projectile is zero
			if (emo->player == 1 || sprite->prototype->max_speed == 0){
				if (!emo->flip_x)
					sprite->a = sprite->prototype->max_speed;
				else
					sprite->a = -sprite->prototype->max_speed;
			}
			else { // or, in the case of an enemy
				if (!emo->flip_x)
					sprite->a = 1 + rand()%(int)sprite->prototype->max_speed;
				else
					sprite->a = -1 - rand()%-(int)sprite->prototype->max_speed;
			}
		}
		else{
			if (!emo->flip_x)
				sprite->a = sprite->prototype->max_speed + emo->a;
			else
				sprite->a = -sprite->prototype->max_speed + emo->a;

			//sprite->a = emo->a * 1.5;

		}

		sprite->jump_timer = 1;
	}
	else
	if (protot->HasAI(AI_EGG) || protot->HasAI(AI_EGG2)){
		sprite->y = emo->y+10;
		sprite->a = emo->a / 1.5;
	}
	else{
		if (!emo->flip_x)
			sprite->a = sprite->prototype->max_speed;
		else
			sprite->a = -sprite->prototype->max_speed;
	}

	if (emo != nullptr){
		sprite->parent_sprite = emo;
		sprite->enemy = emo->enemy;
	}
	else{
		sprite->parent_sprite = nullptr;
	}

	if (protot->type == TYPE_BACKGROUND)
		Sprites_add_bg(sprite);

}

bool Sprite_Destructed (SpriteClass* sprite) { 

	if (sprite == Player_Sprite) // Never remove the player
		return false;
	
	if (sprite->removed) {
		delete sprite;
		return true;
	}

	return false;
	
}

bool bgSprite_Destructed (SpriteClass* sprite) { 
	
	if (sprite->removed)
		return true;

	return false;

}

void Sprites_changeSkullBlocks(){
	for(SpriteClass* sprite: Sprites_List){
		if(sprite==nullptr||sprite->energy<=0)continue;

		for(const int& ai:sprite->prototype->AI_v){
			switch (ai)
			{
			case AI_DIE_IF_SKULL_BLOCKS_CHANGED:{
				sprite->damage_taken = sprite->energy + 1;
				sprite->damage_taken_type = DAMAGE_ALL;
			}	
			break;
			
			case AI_TRANSFORM_IF_SKULL_BLOCKS_CHANGED:{
				PrototypeClass * transformation = sprite->prototype->transformation;
				if(transformation!=nullptr){
					sprite->prototype = transformation;
					sprite->initial_weight = transformation->weight;
					sprite->animation_index = 0;
					sprite->ammo1 = transformation->ammo1;
					sprite->ammo2 = transformation->ammo2;
					sprite->enemy = transformation->enemy;

					sprite->current_command = 0;
				}
			}
			break;
			
			default:
				break;
			}
		}
	}
}


int Update_Sprites() {
	
	const int ACTIVE_BORDER_X = 320;
	const int ACTIVE_BORDER_y = 240;

	int active_sprites = 0;

	//Activate sprite if it is next to the screen
	for (SpriteClass* sprite : Sprites_List) {

		if(sprite->prototype->always_active){
			sprite->active=true;
		}
		else if (sprite->x < Game->camera_x + 640 + ACTIVE_BORDER_X &&
			sprite->x > Game->camera_x - ACTIVE_BORDER_X &&
			sprite->y < Game->camera_y + 480 + ACTIVE_BORDER_y &&
			sprite->y > Game->camera_y - ACTIVE_BORDER_y)
			sprite->active = true;
		else
			sprite->active = false;
	
	}

	// Update bonus first to get energy change
	for (SpriteClass* sprite : Sprites_List) {
		if (sprite->active && !sprite->removed) {
			if (sprite->prototype->type == TYPE_BONUS) {
				UpdateBonusSprite(sprite);
				active_sprites++;
			}
		}
	}

	for (SpriteClass* sprite : Sprites_List) {
		if (sprite->active && !sprite->removed) {
			if (sprite->prototype->type != TYPE_BONUS && sprite->prototype->type != TYPE_BACKGROUND) {
				UpdateSprite(sprite);
				active_sprites++;
			}
		}
	}

	// Clean destructed sprites
	bgSprites_List.remove_if(bgSprite_Destructed);
	Sprites_List.remove_if(Sprite_Destructed);

	/*int count = 0;
	for (SpriteClass* sprite : Sprites_List) {
		if (sprite->energy > 0) {
			if (sprite->prototype->type == TYPE_BONUS)
				count++;
			if (sprite->prototype->bonus > -1 && sprite->prototype->bonuses_number > 0)
				count += sprite->prototype->bonuses_number;

		}
	}

	PLog::Write(PLog::DEBUG, "PK2", "%i", count);
	*/
	return active_sprites;

}

void Sprites_clear() {

	for (SpriteClass* sprite : Sprites_List) {
		delete sprite;
	}

	Sprites_List.clear();
	bgSprites_List.clear();
	Player_Sprite = nullptr;
}