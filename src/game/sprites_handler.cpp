#include "sprites_handler.hpp"
#include "spriteclass.hpp"

#include "engine/PLog.hpp"
#include "engine/PDraw.hpp"
#include "engine/PInput.hpp"

#include "physics.hpp"
#include "episode/episodeclass.hpp"
#include <limits.h>
#include <sstream>
#include "system.hpp"

void SpritesHandler::clearAll(){
    for (SpriteClass* sprite : Sprites_List) {
		if(sprite!=nullptr){
			delete sprite;
		}
	}

	Sprites_List.clear();
	bgSprites_List.clear();
	fgSprites_List.clear();
	Player_Sprite = nullptr;
}

int Get_BG_Parallax_Score(SpriteClass* s){

	PrototypeClass*prototype = s->prototype;
	for(const int& ai:prototype->AI_v){
		if(ai==AI_BACKGROUND_BRING_TO_FRONT){
			return INT_MAX;
		}
		else if(ai==AI_BACKGROUND_SEND_TO_BACK){
			return INT_MIN;
		}
	}

	int parallax = prototype->parallax_type;

	return parallax==0 ? INT_MAX - 1 : parallax;
}

bool Compare_bgSprites(SpriteClass* s1, SpriteClass* s2) {
	return Get_BG_Parallax_Score(s1) < Get_BG_Parallax_Score(s2);
}

int Get_FG_Parallax_Score(SpriteClass* s){

	PrototypeClass*prototype = s->prototype;
	for(const int& ai:prototype->AI_v){
		if(ai==AI_BACKGROUND_BRING_TO_FRONT){
			return INT_MIN;
		}
		else if(ai==AI_BACKGROUND_SEND_TO_BACK){
			return INT_MAX;
		}
	}

	int parallax = prototype->parallax_type;

	return parallax==0 ? INT_MAX - 1 : parallax;
}

bool Compare_fgSprites(SpriteClass* s1, SpriteClass * s2){
	return Get_FG_Parallax_Score(s2) < Get_FG_Parallax_Score(s1);
}

void SpritesHandler::sortBg(){
    this->bgSprites_List.sort(Compare_bgSprites);
	this->fgSprites_List.sort(Compare_fgSprites);
}


void SpritesHandler::onSkullBlocksChanged(){
    for(SpriteClass* sprite: Sprites_List){
		if(sprite==nullptr||sprite->energy<=0)continue;

		for(const SpriteAI::AI_Class& ai: sprite->prototype->AI_f){
			if(ai.trigger==AI_TRIGGER_SKULLS_CHANGED){
				ai.func(sprite);
			}
		}
	}
}

void SpritesHandler::onEvent1(){
	for(SpriteClass* sprite: Sprites_List){
		if(sprite==nullptr||sprite->energy<=0)continue;

		for(const SpriteAI::AI_Class& ai: sprite->prototype->AI_f){
			if(ai.trigger==AI_TRIGGER_EVENT1){
				ai.func(sprite);
			}
		}
	}
}

void SpritesHandler::onEvent2(){
	for(SpriteClass* sprite: Sprites_List){
		if(sprite==nullptr||sprite->energy<=0)continue;

		for(const SpriteAI::AI_Class& ai: sprite->prototype->AI_f){
			if(ai.trigger==AI_TRIGGER_EVENT2){
				ai.func(sprite);
			}
		}
	}
}

int SpritesHandler::onTickUpdate(int camera_x, int camera_y){
	const int ACTIVE_BORDER_X = 320;
	const int ACTIVE_BORDER_y = 240;

	int active_sprites = 0;

	//Activate sprite if it is next to the screen
	for (SpriteClass* sprite : Sprites_List) {
		if(sprite->prototype->always_active){
			sprite->active=true;
		}
		else if (sprite->x < camera_x + 640 + ACTIVE_BORDER_X &&
			sprite->x > camera_x - ACTIVE_BORDER_X &&
			sprite->y < camera_y + 480 + ACTIVE_BORDER_y &&
			sprite->y > camera_y - ACTIVE_BORDER_y){
				sprite->active = true;
			}
			
		else if(sprite->active){
			sprite->active = false;
			SpriteOffscreen(sprite);
		}	
	}

	for (SpriteClass* sprite : Sprites_List) {
		if(sprite->active){
			if(sprite->respawn_timer>0){
				--sprite->respawn_timer;

				if(sprite->respawn_timer==0){
					SpriteOnRespawn(sprite);
				}
			}

			// Update bonus first to get energy change
			if (sprite->active && !sprite->removed) {
				if (sprite->prototype->type == TYPE_BONUS && sprite->respawn_timer==0) {
					UpdateBonusSprite(sprite);
					active_sprites++;
				}
			}
		}
	}

	for (SpriteClass* sprite : Sprites_List) {
		if (sprite->active && !sprite->removed) {
			if (sprite->prototype->type <= TYPE_TELEPORT) {

				if(sprite->respawn_timer==0 || sprite->prototype->destruction_effect>=100){
					UpdateSprite(sprite);
					active_sprites++;
				}
			}
		}
	}

	// Clean destructed sprites
	fgSprites_List.remove_if([](SpriteClass*s){return s->removed || s->prototype->type!=TYPE_FOREGROUND;});
	bgSprites_List.remove_if([](SpriteClass*s){return s->removed || s->prototype->type!=TYPE_BACKGROUND;});
	Sprites_List.remove_if([&](SpriteClass*s){return this->spriteDestructed(s);});


	return active_sprites;
}

SpriteClass* SpritesHandler::mCreateSprite(PrototypeClass* prototype, bool player, double x, double y, SpriteClass*parent_sprite){
	if(prototype==nullptr){
		throw std::runtime_error("Cannot create a sprite from a null prototype");
	}
	
	SpriteClass* sprite = new SpriteClass(prototype, player, x, y, this->mLevelSector, parent_sprite);
	this->Sprites_List.push_back(sprite);

	for(const SpriteAI::AI_Class& ai: sprite->prototype->AI_f){
		if(ai.trigger==AI_TRIGGER_SPAWN || ai.trigger==AI_TRIGGER_TRANSFORMATION){
			ai.func(sprite);
		}
	}

	if (prototype->type == TYPE_BACKGROUND){
		this->mAddBG(sprite);
	}
	else if(prototype->type == TYPE_FOREGROUND){
		this->mAddFG(sprite);
	}

	return sprite;
}

SpriteClass* SpritesHandler::addPlayer(PrototypeClass*prototype, double x, double y){
	SpriteClass* sprite = this->mCreateSprite(prototype, true, x, y, nullptr);
	sprite->initial_update = true;
	sprite->original = true;

	this->Player_Sprite = sprite;

	AI_Functions::player_invisible = sprite;
	if(sprite->invisible_timer==0){
		AI_Functions::player = sprite;
	}
	else{
		AI_Functions::player = nullptr;
	}

	sprite->orig_x = sprite->x;
	sprite->orig_y = sprite->y;

	return sprite;
}

void SpritesHandler::addLevelSprite(PrototypeClass*prototype, double x, double y){
	SpriteClass* sprite  = this->mCreateSprite(prototype, false, x, y, nullptr);
	sprite->initial_update = true;
	sprite->original = true;

	/**
	 * @brief 
	 * Legacy feature
	 * Why does it exist ?
	 */
	sprite->x = x + 16 + 1;
	sprite->y += sprite->prototype->height/2;

	sprite->orig_x = sprite->x;
	sprite->orig_y = sprite->y;
}

void SpritesHandler::addDroppedBonusSprite(PrototypeClass*prototype, double x, double y){
	SpriteClass* sprite  = this->mCreateSprite(prototype, false, x, y, nullptr);

	if(sprite->weight>0)sprite->jump_timer = 1;
	
	sprite->damage_timer = 35;//25

	sprite->a = 3 - rand()%7;
}

void SpritesHandler::addGiftSprite(PrototypeClass* prototype){
	/**
	 * @brief 
	 * Fix spyrooster green bee bug.
	 */
	SpriteClass* parent = nullptr;
	if(this->Player_Sprite->enemy){
		parent = this->Player_Sprite;
	}
	
	SpriteClass* sprite  = this->mCreateSprite(prototype,
		false,
		Player_Sprite->x,
		Player_Sprite->y, parent);

	sprite->y += sprite->prototype->height/2;
}

void SpritesHandler::addProjectileSprite(PrototypeClass* prototype, double x, double y, SpriteClass* shooter){
	if(shooter==nullptr){
		PLog::Write(PLog::WARN, "PK2", "Trying to add ammo sprite with null shooter!");
		return;
	}

	SpriteClass* sprite  = this->mCreateSprite(prototype, false, x, y, shooter);

	if(prototype->type==TYPE_GAME_CHARACTER && shooter->HasAI(AI_BONUS)){
		sprite->damage_timer = 2;
	}

	sprite->parent_sprite = shooter;
	sprite->enemy = shooter->enemy;

	if(prototype->AI_p.empty()){ // No other projectile AIs, default behaviour
		if (!shooter->flip_x)
			sprite->a = sprite->prototype->max_speed;
		else
			sprite->a = -sprite->prototype->max_speed;
	}
	else{
		for(const SpriteAI::ProjectileAIClass& ai: prototype->AI_p){
			ai.func(sprite, shooter);
		}
	}

}

bool SpritesHandler::spriteDestructed(SpriteClass* sprite) { 

	if (sprite == Player_Sprite) // Never remove the player
		return false;
	
	if (sprite->removed) {
		delete sprite;
		return true;
	}

	return false;
}


static bool isSpriteVisible(const SpriteClass* sprite, int camera_x, int camera_y){
	return (sprite->x - sprite->prototype->picture_frame_width/2  < camera_x + screen_width &&
			sprite->x + sprite->prototype->picture_frame_width/2  > camera_x &&
			sprite->y - sprite->prototype->picture_frame_height/2 < camera_y + screen_height &&
			sprite->y + sprite->prototype->picture_frame_height/2 > camera_y);
}

void SpritesHandler::drawBGsprites(int camera_x, int camera_y, bool gamePaused, int& debug_drawn_sprites){
	for (SpriteClass* sprite : this->bgSprites_List) {

		double orig_x = sprite->orig_x;
		double orig_y = sprite->orig_y;

		double xl, yl;

		if (sprite->prototype->parallax_type != 0) {

			double parallax = double(sprite->prototype->parallax_type);

			xl =  orig_x - camera_x-screen_width/2 - sprite->prototype->width/2;
			xl /= parallax;
			yl =  orig_y - camera_y-screen_height/2 - sprite->prototype->height/2;
			yl /= parallax;
		}
		else {

			xl = yl = 0;

		}

		UpdateBackgroundSprite(sprite, yl);

		sprite->x = orig_x-xl;
		sprite->y = orig_y-yl;

		if (isSpriteVisible(sprite, camera_x, camera_y)) {
			sprite->draw(camera_x,camera_y);

			if (!gamePaused)
				sprite->HandleEffects();

			sprite->hidden = false;
			debug_drawn_sprites++;
		} else {
			if (!gamePaused)
				sprite->Animoi();
			sprite->hidden = true;
		}
	}
}

void SpritesHandler::drawFGsprites(int camera_x, int camera_y, bool gamePaused, int& debug_drawn_sprites){
	for(SpriteClass* sprite : this->fgSprites_List){

		double orig_x = sprite->orig_x;
		double orig_y = sprite->orig_y;

		double xl, yl;

		if (sprite->prototype->parallax_type != 0) {

			double parallax = double(-sprite->prototype->parallax_type);

			xl =  orig_x - camera_x-screen_width/2 - sprite->prototype->width/2;
			xl /= parallax;
			
			yl =  orig_y - camera_y-screen_height/2 - sprite->prototype->height/2;
			yl /= parallax;
		}
		else {

			xl = yl = 0;

		}

		UpdateBackgroundSprite(sprite, yl);

		sprite->x = orig_x-xl;
		sprite->y = orig_y-yl;


		if (isSpriteVisible(sprite, camera_x, camera_y)) {
			sprite->draw(camera_x,camera_y);

			if (!gamePaused)
				sprite->HandleEffects();

			sprite->hidden = false;
			debug_drawn_sprites++;
		} else {
			if (!gamePaused)
				sprite->Animoi();
			sprite->hidden = true;
		}
	}
}

void SpritesHandler::drawSprites(int camera_x, int camera_y, bool gamePaused, int& debug_drawn_sprites){
	for (SpriteClass* sprite : this->Sprites_List) {
		if (sprite->prototype->type == TYPE_BACKGROUND || sprite->prototype->type == TYPE_FOREGROUND)
			continue;
		
		if (sprite->removed)
			continue;

		if (isSpriteVisible(sprite, camera_x, camera_y)) {

			// Draw impact circle
			if (sprite->damage_timer > 0 && sprite->prototype->type != TYPE_BONUS && sprite->energy < 1){
				int framex = ((degree%12)/3) * 58;
				u32 hit_x = sprite->x-8;
				u32 hit_y = sprite->y-8;
				PDraw::image_cutclip(game_assets,hit_x-camera_x-28+8, hit_y-camera_y-27+8,1+framex,83,1+57+framex,83+55);
			}

			bool blinking = dev_mode && sprite->player && PInput::Keydown(PInput::Y);
			if(!blinking || degree % 2 == 0){
				sprite->draw(camera_x, camera_y);
			}

			// Draw stars on dead sprite
			if (sprite->energy < 1 && sprite->prototype->type != TYPE_PROJECTILE){
				int sx = (int)sprite->x;
				for (int stars=0; stars<3; stars++){
					double star_x = sprite->x - 8  + sin_table((stars*120+degree)*2)      / 3;
					double star_y = sprite->y - 18 + cos_table((stars*120+degree)*2 + sx) / 8;
					PDraw::image_cutclip(game_assets,star_x-camera_x, star_y-camera_y,1,1,11,11);
				}
			}

			if (!gamePaused)
				sprite->HandleEffects();

			debug_drawn_sprites++;

		} else {

			if (!gamePaused)
				sprite->Animoi();

			// Delete death body
			if (sprite->energy < 1 && sprite->respawn_timer==0)
				sprite->removed = true;
			
		}
	}
}