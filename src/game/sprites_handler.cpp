#include "sprites_handler.hpp"
#include "spriteclass.hpp"
#include "engine/PLog.hpp"
#include "mapclass.hpp"
#include "physics.hpp"
#include "game.hpp"

void SpritesHandler::clearAll(){
    for (int i = 0; i < MAX_PROTOTYYPPEJA; i++) {
		Level_Prototypes_List[i] = nullptr;
	}

    for (SpriteClass* sprite : Sprites_List) {
		if(sprite!=nullptr){
			delete sprite;
		}
	}

	Sprites_List.clear();
	bgSprites_List.clear();
	fgSprites_List.clear();
	Player_Sprite = nullptr;

    this->prototypesHandler.clear();
}

PrototypeClass* SpritesHandler::getLevelPrototype(int index){
    if(index<0 || index>= MAX_PROTOTYYPPEJA) return nullptr;
	else return Level_Prototypes_List[index];
}

void SpritesHandler::loadLevelPrototype(const std::string& name, int id) {

	if (id<0 || id >= MAX_PROTOTYYPPEJA) {
		PLog::Write(PLog::ERR, "PK2", "Invalid prototype id");
		return;
	}

	PrototypeClass* protot = this->prototypesHandler.loadPrototype(name);
    this->Level_Prototypes_List[id] = protot;
}


void SpritesHandler::loadAllLevelPrototypes(const MapClass& map){

    for (u32 i = 0; i < PK2MAP_MAP_MAX_PROTOTYPES; i++) {
		if (strcmp(map.sprite_filenames[i], "") != 0) {
            this->loadLevelPrototype(map.sprite_filenames[i], i);
			/*PFile::Path path = Episode->Get_Dir(Game->map.sprite_filenames[i]);
			if (Level_Prototypes_get(Game->map.sprite_filenames[i], i) == nullptr) {
				PLog::Write(PLog::WARN, "PK2", "Can't load sprite %s. It will not appear", Game->map.sprite_filenames[i]);
			}*/
		}
	}
}

bool Compare_bgSprites(SpriteClass* s1, SpriteClass* s2) {
	return (s1->prototype->parallax_type < s2->prototype->parallax_type); 
}

void SpritesHandler::sortBg(){
    this->bgSprites_List.sort(Compare_bgSprites);
}

void SpritesHandler::onGameStart(){
    for (SpriteClass* sprite : Sprites_List) {
		sprite->a = 0;

		for(const SpriteAI::AI_Class& ai: sprite->prototype->AI_f){

			if(ai.trigger==AI_TRIGGER_GAME_START || ai.trigger==AI_TRIGGER_SPAWN){
				ai.func(sprite);
			}
		}
	}
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

int SpritesHandler::onTickUpdate(){
    if(Player_Sprite!=nullptr && Player_Sprite->energy>0){
		AI_Functions::player_invisible = Player_Sprite;

		if(Player_Sprite->invisible_timer>0){
			AI_Functions::player = nullptr;
		}
		else{
			AI_Functions::player = Player_Sprite;
		}

	}
	else{
		AI_Functions::player = nullptr;
		AI_Functions::player_invisible = nullptr;
	}
	
	const int ACTIVE_BORDER_X = 320;
	const int ACTIVE_BORDER_y = 240;

	int active_sprites = 0;

	//Activate sprite if it is next to the screen
	for (SpriteClass* sprite : Sprites_List) {
		if(sprite->respawn_timer>0){
			sprite->active=false;
			--sprite->respawn_timer;

			if(sprite->respawn_timer==0){
				SpriteOnRespawn(sprite);
			}

		}
		else if(sprite->prototype->always_active){
			sprite->active=true;
		}
		else if (sprite->x < Game->camera_x + 640 + ACTIVE_BORDER_X &&
			sprite->x > Game->camera_x - ACTIVE_BORDER_X &&
			sprite->y < Game->camera_y + 480 + ACTIVE_BORDER_y &&
			sprite->y > Game->camera_y - ACTIVE_BORDER_y){
				sprite->active = true;
			}
			
		else{
			sprite->active = false;
		}	
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
	fgSprites_List.remove_if([](SpriteClass*s){return s->removed;});
	bgSprites_List.remove_if([](SpriteClass*s){return s->removed;});
	Sprites_List.remove_if([&](SpriteClass*s){return this->spriteDestructed(s);});


	return active_sprites;
}

void SpritesHandler::addSprite(PrototypeClass* protot, int is_Player_Sprite, double x, double y, SpriteClass* parent, bool isbonus) {

	SpriteClass* sprite = new SpriteClass(protot, is_Player_Sprite, x, y);
	Sprites_List.push_back(sprite);

	if (is_Player_Sprite){
		Player_Sprite = sprite;
		AI_Functions::player_invisible = sprite;
		if(sprite->invisible_timer==0){
			AI_Functions::player = sprite;
		}
		else{
			AI_Functions::player = nullptr;
		}
	}

	if(isbonus) { //If it is a bonus dropped by enemy
		sprite->orig_x = sprite->x;
		sprite->orig_y = sprite->y;

		if(sprite->weight!=0)sprite->jump_timer = 1;
		
		sprite->damage_timer = 35;//25

		if(protot->weight==0 && protot->max_speed==0){
			sprite->a = 0;
			sprite->b = 0;
		}
		else{
			sprite->a = 3 - rand()%7;
		}
	} else {

		sprite->x = x + 16 + 1;
		sprite->y += sprite->prototype->height/2;
		sprite->orig_x = sprite->x;
		sprite->orig_y = sprite->y;
		sprite->parent_sprite = parent;
	}

	if (protot->type == TYPE_BACKGROUND){
		this->add_bg(sprite);
	}
	else if(protot->type == TYPE_FOREGROUND){
		this->add_fg(sprite);
	}
}

void SpritesHandler::addSpriteAmmo(PrototypeClass* protot, double x, double y, SpriteClass* emo) {

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

	if (protot->type == TYPE_BACKGROUND){
		this->add_bg(sprite);
	}
	else if(protot->type == TYPE_FOREGROUND){
		this->add_fg(sprite);
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