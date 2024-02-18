//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include <sstream>
#include "game/physics.hpp"

#include "game/game.hpp"

#include "game/gifts.hpp"
#include "settings.hpp"
#include "gfx/particles.hpp"
#include "gfx/effect.hpp"
#include "gfx/text.hpp"
#include "language.hpp"
#include "sfx.hpp"
#include "system.hpp"
#include "episode/episodeclass.hpp"
#include "gui.hpp"
#include "exceptions.hpp"


#include "engine/types.hpp"
#include "engine/PInput.hpp"
#include "engine/PSound.hpp"

#include <cstring>

static double sprite_left;
static double sprite_right;
static double sprite_upper;
static double sprite_bottom;

static int sprite_width;
static int sprite_height;

static bool oikealle;
static bool vasemmalle;
static bool ylos;
static bool alas;

static PK2BLOCK Block_Get(u32 x, u32 y) {

	PK2BLOCK block;
	//memset(&block, 0, sizeof(block));

	// Outside the screen
	if (x >= PK2MAP_MAP_WIDTH || y >= PK2MAP_MAP_HEIGHT) {
		
		block.id  = 255;
		block.permeable = true;
		block.left  = x*32;
		block.right  = x*32 + 32;
		block.top    = y*32;
		block.bottom    = y*32 + 32;
		block.water  = false;
		block.border = true;

		block.left_side = 0;
		block.right_side = 0;
		block.top_side = 0;
		block.bottom_side = 0;

		return block;

	}

	u8 i = Game->map.foreground_tiles[x+y*PK2MAP_MAP_WIDTH];

	if (i<150) { //If it is ground

		block        = Game->block_types[i];
		block.left  = x*32+Game->block_types[i].left;
		block.right  = x*32+32+Game->block_types[i].right;
		block.top    = y*32+Game->block_types[i].top;
		block.bottom    = y*32+32+Game->block_types[i].bottom;

	} else { //If it is sky - Need to reset
	
		block.id  = 255;
		block.permeable = true;
		block.left  = x*32;
		block.right  = x*32 + 32;
		block.top    = y*32;
		block.bottom    = y*32 + 32;
		block.water  = false;

		block.left_side = 0;
		block.right_side = 0;
		block.top_side = 0;
		block.bottom_side = 0;
	
	}

	i = Game->map.background_tiles[x+y*PK2MAP_MAP_WIDTH];

	if (i > 131 && i < 140)
		block.water = true;

	block.border = Game->map.edges[x+y*PK2MAP_MAP_WIDTH];

	return block;
}

static void Check_SpriteBlock(SpriteClass* sprite, const PK2BLOCK &block) {

	//left and right
	if (sprite_upper < block.bottom && sprite_bottom-1 > block.top){
		if (sprite_right+sprite->a-1 > block.left && sprite_left+sprite->a < block.right){
			// Tutkitaan onko sprite menossa oikeanpuoleisen palikan sis��n.
			if (sprite_right+sprite->a < block.right){
				// Onko block sein�?
				if (block.right_side == BLOCK_WALL){
					oikealle = false;
					if (block.id == BLOCK_LIFT_HORI)
						sprite->x = block.left - sprite_width/2;
				}
			}

			if (sprite_left+sprite->a > block.left){
				if (block.left_side == BLOCK_WALL){
					vasemmalle = false;

					if (block.id == BLOCK_LIFT_HORI)
						sprite->x = block.right + sprite_width/2;

				}
			}
		}
	}

	sprite_left = sprite->x-sprite_width/2;
	sprite_right = sprite->x+sprite_width/2;

	//ceil and floor

	if (sprite_left < block.right && sprite_right-1 > block.left){
		if (sprite_bottom+sprite->b-1 >= block.top && sprite_upper+sprite->b <= block.bottom){
			if (sprite_bottom+sprite->b-1 <= block.bottom){
				if (block.bottom_side == BLOCK_WALL){
					alas = false;

					if (block.id == BLOCK_LIFT_VERT)
						sprite->y = block.top - sprite_height /2;

					if (sprite_bottom-1 >= block.top && sprite->b >= 0)
						if (block.id != BLOCK_LIFT_HORI)
							sprite->y = block.top - sprite_height /2;
				}
			}

			if (sprite_upper+sprite->b > block.top){
				if (block.top_side == BLOCK_WALL){
					ylos = false;

					if (sprite_upper < block.bottom)
						if (block.id != BLOCK_LIFT_HORI)
							sprite->crouched = true;
				}
			}
		}
	}
}

void Check_MapBlock(SpriteClass* sprite, PK2BLOCK block) {

	//If sprite is in the block
	if (sprite->x <= block.right && sprite->x >= block.left && sprite->y <= block.bottom && sprite->y >= block.top){

		/**********************************************************************/
		/* Examine if block is water background                               */
		/**********************************************************************/
		if (block.water)
			sprite->in_water = true;
		else
			sprite->in_water = false;

		/**********************************************************************/
		/* Examine if it touches the fire                                     */
		/**********************************************************************/
		if (block.id == BLOCK_FIRE && Game->button1 == 0 && sprite->damage_timer == 0){
			sprite->damage_taken = 2;
			sprite->damage_taken_type = DAMAGE_FIRE;
		}

		/**********************************************************************/
		/* Examine if block is hideway			                              */
		/**********************************************************************/
		if (block.id == BLOCK_HIDEOUT)
			sprite->hidden = true;
		else
			sprite->hidden = false;
		

		/**********************************************************************/
		/* Examine if block is the exit                                       */
		/**********************************************************************/
		if (block.id == BLOCK_EXIT && sprite->energy>0) {
			if(Game->chick_mode){
				if(sprite->HasAI(AI_CHICK)){
					Game->Finish();
				}
			}
			else if(sprite->player){
				Game->Finish();
			}
		}
	}

	//If sprite is thouching the block
	if (sprite_left <= block.right-4 && sprite_right >= block.left+4 && sprite_upper <= block.bottom && sprite_bottom >= block.top+16){
		/**********************************************************************/
		/* Examine if it touches the fire                                     */
		/**********************************************************************/
		if (block.id == BLOCK_FIRE && Game->button1 == 0 && sprite->damage_timer == 0){
			sprite->damage_taken = 2;
			sprite->damage_taken_type = DAMAGE_FIRE;
		}
	}

	//Examine if there is a block on bottom
	if ((block.id<80 || block.id>139) && block.id != BLOCK_BARRIER_DOWN && block.id < 150){
		int mask_index = (int)(sprite->x+sprite->a) - block.left;

		if (mask_index < 0)
			mask_index = 0;

		if (mask_index > 31)
			mask_index = 31;

		block.top += Game->block_masks[block.id].alas[mask_index];

		if (block.top >= block.bottom-2)
			block.bottom_side = BLOCK_BACKGROUND;

		block.bottom -= Game->block_masks[block.id].ylos[mask_index];
	}

	//If sprite is thouching the block (again?)
	if (sprite_left <= block.right+2 && sprite_right >= block.left-2 && sprite_upper <= block.bottom && sprite_bottom >= block.top){
		/**********************************************************************/
		/* Examine if it is a key and touches lock wall                       */
		/**********************************************************************/
		if (block.id == BLOCK_LOCK && sprite->prototype->can_open_locks){
			Game->map.foreground_tiles[block.left/32+(block.top/32)*PK2MAP_MAP_WIDTH] = 255;
			Game->map.Calculate_Edges();

			sprite->removed = true;

			if (sprite->prototype->how_destroyed != FX_DESTRUCT_INDESTRUCTIBLE) {
				Game->keys--;
				if (Game->keys < 1)
					Game->Open_Locks();
			}

			Effect_Explosion(block.left+16, block.top+10, 0);
			Play_GameSFX(Episode->sfx.open_locks_sound,100, (int)sprite->x, (int)sprite->y, SOUND_SAMPLERATE, false);
		}

		/**********************************************************************/
		/* Make wind effects                                                  */
		/**********************************************************************/
		if (block.id == BLOCK_DRIFT_LEFT && vasemmalle)
			sprite->a -= 0.02;

		if (block.id == BLOCK_DRIFT_RIGHT && oikealle)
			sprite->a += 0.02;	//0.05

		/*********************************************************************/
		/* Examine if sprite is on the border to fall                        */
		/*********************************************************************/
		if (block.border && sprite->jump_timer <= 0 && sprite->y < block.bottom && sprite->y > block.top){
			/* && sprite_bottom <= block.bottom+2)*/ // onko sprite tullut borderlle
			if (sprite_left > block.left)
				sprite->edge_on_the_left = true;

			if (sprite_right < block.right)
				sprite->edge_on_the_right = true;
		}
	}

	//Examine walls on left and right

	if (sprite_upper < block.bottom && sprite_bottom-1 > block.top) {
		if (sprite_right+sprite->a-1 > block.left && sprite_left+sprite->a < block.right) {
			// Examine whether the sprite going in the right side of the block.
			if (sprite_right+sprite->a < block.right) {
				// Onko block sein�?
				if (block.right_side == BLOCK_WALL) {
					oikealle = false;

					if (block.id == BLOCK_LIFT_HORI)
						sprite->x = block.left - sprite_width/2;
				}
			}
			// Examine whether the sprite going in the left side of the block.
			if (sprite_left+sprite->a > block.left) {
				if (block.left_side == BLOCK_WALL) {
					vasemmalle = false;

					if (block.id == BLOCK_LIFT_HORI)
						sprite->x = block.right + sprite_width/2;

				}
			}
		}
	}

	sprite_left = sprite->x - sprite_width/2;
	sprite_right = sprite->x + sprite_width/2;

	//Examine walls on up and down

	if (sprite_left < block.right && sprite_right-1 > block.left) { //Remove the left and right blocks
		if (sprite_bottom+sprite->b-1 >= block.top && sprite_upper+sprite->b <= block.bottom) { //Get the up and down blocks
			if (sprite_bottom+sprite->b-1 <= block.bottom) { //Just in the sprite's foot
				if (block.bottom_side == BLOCK_WALL) { //If it is a wall
					alas = false;
					if (block.id == BLOCK_LIFT_VERT)
						sprite->y = block.top - sprite_height /2;

					if (sprite_bottom-1 >= block.top && sprite->b >= 0) {
						//sprite->y -= sprite_bottom - block.top;
						if (block.id != BLOCK_LIFT_HORI) {
							sprite->y = block.top - sprite_height /2;
						}
					}

					if (sprite->weight_button >= 1) { // Sprite can press the buttons
						if (block.id == BLOCK_BUTTON1 && Game->button1 == 0) {
							Game->button1 = Game->map.button1_time;
							Game->button_vibration = 64;
							Play_GameSFX(Episode->sfx.switch_sound, 100, (int)sprite->x, (int)sprite->y, SOUND_SAMPLERATE, false);
							PInput::Vibrate(1000);
						}

						if (block.id == BLOCK_BUTTON2 && Game->button2 == 0) {
							Game->button2 = Game->map.button2_time;
							Game->button_vibration = 64;
							Play_GameSFX(Episode->sfx.switch_sound, 100, (int)sprite->x, (int)sprite->y, SOUND_SAMPLERATE, false);
							PInput::Vibrate(1000);
						}

						if (block.id == BLOCK_BUTTON3 && Game->button3 == 0) {
							Game->button3 = Game->map.button3_time;
							Game->button_vibration = 64;
							Play_GameSFX(Episode->sfx.switch_sound, 100, (int)sprite->x, (int)sprite->y, SOUND_SAMPLERATE, false);
							PInput::Vibrate(1000);
						}
					}

				}
			}

			if (sprite_upper+sprite->b > block.top) {
				if (block.top_side == BLOCK_WALL) {
					ylos = false;

					if (sprite_upper < block.bottom) {
						if (block.id == BLOCK_LIFT_VERT && sprite->crouched) {
							sprite->damage_taken = 2;
							sprite->damage_taken_type = DAMAGE_IMPACT;
						}

						if (block.id != BLOCK_LIFT_HORI) {
							//if (sprite->crouched)
							//	sprite->y = block.bottom + sprite_height /2;

							sprite->crouched = true;
						}
					}
				}
			}
		}
	}
}


void SpriteOnDamage(SpriteClass* sprite){
	sprite->energy -= sprite->damage_taken;
	sprite->damage_timer = DAMAGE_TIME;

	if (sprite->damage_taken_type == DAMAGE_ELECTRIC)
		sprite->damage_timer *= 6;

	Play_GameSFX(sprite->prototype->sounds[SOUND_DAMAGE], 100, (int)sprite->x, (int)sprite->y,
					sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

	if (sprite->prototype->how_destroyed%100 == FX_DESTRUCT_FEATHERS)
		Effect_Destruction(FX_DESTRUCT_FEATHERS, (u32)sprite->x, (u32)sprite->y);

	if (sprite->prototype->type != TYPE_PROJECTILE){
		Particles_New(PARTICLE_STAR,sprite->x,sprite->y,-1,-1,60,0.01,128);
		Particles_New(PARTICLE_STAR,sprite->x,sprite->y, 0,-1,60,0.01,128);
		Particles_New(PARTICLE_STAR,sprite->x,sprite->y, 1,-1,60,0.01,128);
	}


	for(const SpriteAI::AI_Class& ai: sprite->prototype->AI_f){
		if(!ai.apply_to_creatures)continue;

		if(ai.trigger==AI_TRIGGER_DAMAGE){
			ai.func(sprite);
		}
	}
}

void SpriteOnRespawn(SpriteClass* sprite){
	double ax, ay;
	ax = sprite->x;//-sprite->prototype->width;
	ay = sprite->y-sprite->prototype->height/2.0;
	sprite->removed = false;
	sprite->energy = sprite->prototype->energy;
	Effect_Stars(ax, ay, sprite->prototype->color);
	/*for (int r=1;r<6;r++)
		Particles_New(PARTICLE_SPARK,ax+rand()%10-rand()%10, ay+rand()%10-rand()%10,0,0,rand()%100,0.1,32);*/
}

void SpriteOnDeath(SpriteClass* sprite){
	int how_destroyed = sprite->prototype->how_destroyed;

	for(const SpriteAI::AI_Class&ai: sprite->prototype->AI_f){

		if(ai.trigger==AI_TRIGGER_DEATH){
			ai.func(sprite);
		}
	}

	if (sprite->prototype->bonus != nullptr && sprite->prototype->bonuses_number > 0){
		if (sprite->prototype->bonus_always || rand()%4 == 1){

			int bonuses_number = sprite->prototype->bonuses_number;
			PrototypeClass* bonus = sprite->prototype->bonus;

			if(bonuses_number>1){
				for(int i=0;i<bonuses_number;++i){
					Game->spritesHandler.addSprite(bonus, 0, sprite->x+(10-rand()%21),sprite->y+(10-rand()%21), nullptr, true);
				}
			}
			else if(bonuses_number==1){
				Game->spritesHandler.addSprite(bonus, 0, sprite->x,sprite->y, nullptr, true);
			}			
		}
	}

	if (how_destroyed >= FX_DESTRUCT_ANIMATED)
		how_destroyed -= FX_DESTRUCT_ANIMATED;
	else
		sprite->removed = true;

	Effect_Destruction(how_destroyed, (u32)sprite->x, (u32)sprite->y);
	Play_GameSFX(sprite->prototype->sounds[SOUND_DESTRUCTION],100, (int)sprite->x, (int)sprite->y,
					sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

	if (sprite->prototype->type == TYPE_GAME_CHARACTER && sprite->prototype->score != 0 && sprite!=Game->spritesHandler.Player_Sprite){
		Fadetext_New(fontti2,std::to_string(sprite->prototype->score),(int)sprite->x-8,(int)sprite->y-8,80);
		Game->score_increment += sprite->prototype->score;
	}


	for(SpriteClass* sprite2: Game->spritesHandler.Sprites_List){
		if(sprite2->parent_sprite==sprite){
			sprite2->parent_sprite=nullptr;
			if(sprite2->HasAI(AI_DIE_WITH_MOTHER_SPPRITE)){
				sprite2->damage_taken = sprite2->energy;
				sprite2->damage_taken_type = DAMAGE_ALL;
			}
		}
	}
}

void DisplayInfo(SpriteClass* sprite, int info){
    if (AI_Functions::player_invisible!=nullptr &&
    (AI_Functions::player_invisible->x - sprite->x < 10
    && AI_Functions::player_invisible->x - sprite->x > -10) &&
    (AI_Functions::player_invisible->y - sprite->y < sprite->prototype->height
    && AI_Functions::player_invisible->y - sprite->y > -sprite->prototype->height)){
        
        //int info = ai.info_id;
        std::string sinfo = "info";
        if (info < 10) sinfo += '0';
        sinfo += std::to_string(info);

        int index = Episode->infos.Search_Id(sinfo.c_str());
        if (index != -1)
            Game->Show_Info(Episode->infos.Get_Text(index));
        else if(info<100)
            Game->Show_Info(tekstit->Get_Text(PK_txt.infos[info]));
    }
}

void UpdateSprite(SpriteClass* sprite){
	
	if (!sprite->prototype){
		throw PExcept::PException("Sprite with null prototype is not acceptable!");
	}

	sprite_width  = sprite->prototype->width;
	sprite_height = sprite->prototype->height;

	sprite_left = sprite->x - sprite_width  / 2;
	sprite_right = sprite->x + sprite_width  / 2;
	sprite_upper   = sprite->y - sprite_height / 2;
	sprite_bottom   = sprite->y + sprite_height / 2;

	double max_speed = sprite->prototype->max_speed;

	bool in_water = sprite->in_water;

	oikealle	 = true,
	vasemmalle	 = true,
	ylos		 = true,
	alas		 = true;

	sprite->crouched = false;

	sprite->edge_on_the_left = false;
	sprite->edge_on_the_right = false;


	/* Pistet��n vauhtia tainnutettuihin spriteihin */
	if (sprite->energy < 1)
		max_speed = 3;

	// Calculate the remainder of the sprite towards

	if (sprite->attack1_timer > 0)
		sprite->attack1_timer--;

	if (sprite->attack2_timer > 0)
		sprite->attack2_timer--;

	if (sprite->charging_timer > 0)	// aika kahden ampumisen (munimisen) v�lill�
		sprite->charging_timer --;

	if (sprite->mutation_timer > 0)	// time of mutation
		sprite->mutation_timer --;

	/*****************************************************************************************/
	/* Player-sprite and its controls                                                        */
	/*****************************************************************************************/

	bool add_speed = true;
	bool gliding = false;

	/*bool max_speed_available = sprite->HasAI(AI_MAX_SPEED_PLAYER) ||
		(sprite->swimming && sprite->HasAI(AI_MAX_SPEED_SWIMMING)) ||
		(sprite->super_mode_timer > 0 && sprite->HasAI(AI_MAX_SPEED_PLAYER_ON_SUPER));*/

	if (sprite->player && sprite->energy > 0){
		/* SLOW WALK */
		if (PInput::Keydown(Input->walk_slow)
		|| Gui_pad_button == 1 || Gui_pad_button == 3)
			add_speed = false;

		/* ATTACK 1 */
		if ((PInput::Keydown(Input->attack1) || Gui_egg) && sprite->charging_timer == 0 && sprite->ammo1 != nullptr)
			sprite->attack1_timer = sprite->prototype->attack1_time;
		/* ATTACK 2 */
		else if ((PInput::Keydown(Input->attack2) || Gui_doodle) && sprite->charging_timer == 0 && sprite->ammo2 != nullptr)
				sprite->attack2_timer = sprite->prototype->attack2_time;

		/* CROUCH */
		sprite->crouched = false;
		bool axis_couch = (Input == &Settings.joystick) && (PInput::GetAxis(1) > 0.5);
		if ((PInput::Keydown(Input->down) || Gui_down || axis_couch) && !sprite->can_move_down) {
			sprite->crouched = true;
			sprite_upper += sprite_height/1.5;
		}

		/* NAVIGATING*/
		int navigation = 0;

		if (Input == &Settings.joystick)
			navigation = PInput::GetAxis(0) * 100;

		if (Gui_pad_button == 0 || Gui_pad_button == 1)
			navigation = -100;
		else if (Gui_pad_button == 3 || Gui_pad_button == 4)
			navigation = 100;

		if (PInput::Keydown(Input->right))
			navigation = 100;
		
		if (PInput::Keydown(Input->left))
			navigation = -100;

		double a_lisays = 0.04;//0.08;

		if (add_speed) {
			if (rand()%20 == 1 && sprite->animation_index == ANIMATION_WALKING) // Draw dust
				Particles_New(PARTICLE_DUST_CLOUDS,sprite->x-8,sprite_bottom-8,0.25,-0.25,40,0,0);

			a_lisays += 0.09;//0.05
		}

		if (sprite->can_move_down)
			a_lisays /= 1.5;//2.0

		a_lisays *= double(navigation) / 100;

		if (sprite->max_speed_available)
			a_lisays *= max_speed;
				
		if (navigation > 0)
			sprite->flip_x = false;
		else if (navigation < 0)
			sprite->flip_x = true;

		if (sprite->crouched)	// Slow when couch
			a_lisays /= 10;

		sprite->a += a_lisays;

		/* JUMPING */
		if (sprite->prototype->weight > 0 && !sprite->swimming) {
			if (PInput::Keydown(Input->jump) || Gui_up) {
				if (!sprite->crouched) {
					if (sprite->jump_timer == 0)
						Play_GameSFX(Episode->sfx.jump_sound, 100, (int)sprite->x, (int)sprite->y,
									  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

					if (sprite->jump_timer <= 0)
						sprite->jump_timer = 1; //10;
				}
			} else {
				if (sprite->jump_timer > 0 && sprite->jump_timer < 45)
					sprite->jump_timer = 55;
			}

			/* dripping quietly down */
			bool axis_up = (Input == &Settings.joystick) && (PInput::GetAxis(1) < -0.5);
			if ((PInput::Keydown(Input->jump) || Gui_up || axis_up) && sprite->jump_timer >= 150/*90+20*/ &&
				sprite->prototype->can_glide)
				gliding = true;
		}
		/* MOVING UP AND DOWN */
		else { // if the player sprite-weight is 0 - like birds

			double speed = 0.15;
			if (sprite->max_speed_available)
				speed *= max_speed;

			if (PInput::Keydown(Input->jump) || Gui_up)
				sprite->b -= speed;

			if (PInput::Keydown(Input->down) || Gui_down)
				sprite->b += speed;

			sprite->jump_timer = 0;
		}

		/* AI */
		for(const SpriteAI::AI_Class& ai:sprite->prototype->AI_f){
			if(!ai.apply_to_player)continue;

			if( (sprite->energy>0 && ai.trigger==AI_TRIGGER_ALIVE) || ai.trigger==AI_TRIGGER_ANYWAY){
				ai.func(sprite);
			}
		}

		/* It is not acceptable that a player is anything other than the game character */
		if (sprite->prototype->type != TYPE_GAME_CHARACTER)
			sprite->energy = 0;
	}

	/*****************************************************************************************/
	/* Jump                                                                                  */
	/*****************************************************************************************/

	bool hyppy_maximissa = sprite->jump_timer >= 90;

	// Jos ollaan hyp�tty / ilmassa:
	if (sprite->jump_timer > 0) {
		if (sprite->jump_timer < 50-sprite->prototype->max_jump)
			sprite->jump_timer = 50-sprite->prototype->max_jump;

		if (sprite->jump_timer < 10)
			sprite->jump_timer = 10;

		if (!hyppy_maximissa) {
		// sprite->b = (sprite->prototype->max_jump/2 - sprite->jump_timer/2)/-2.0;//-4
		   sprite->b = -sin_table(sprite->jump_timer)/8;//(sprite->prototype->max_jump/2 - sprite->jump_timer/2)/-2.5;
			if (sprite->b > sprite->prototype->max_jump){
				sprite->b = sprite->prototype->max_jump/10.0;
				sprite->jump_timer = 90 - sprite->jump_timer;
			}

		}

		if (sprite->jump_timer < 180)
			sprite->jump_timer += 2;
	}

	if (sprite->jump_timer < 0)
		sprite->jump_timer++;

	if (sprite->b > 0 && !hyppy_maximissa)
		sprite->jump_timer = 90;//sprite->prototype->max_jump*2;

	/*****************************************************************************************/
	/* Hit recovering                                                                        */
	/*****************************************************************************************/

	if (sprite->damage_timer > 0)
		sprite->damage_timer--;

	/*****************************************************************************************/
	/* Timers                                                                                */
	/*****************************************************************************************/

	if (sprite->invisible_timer > 0) {
	
		sprite->invisible_timer--;

	}

	SpriteClass* Player_Sprite = Game->spritesHandler.Player_Sprite;
	
	if (sprite->super_mode_timer > 0) {

		sprite->super_mode_timer--;
		if (Player_Sprite->super_mode_timer == 0)
			//PSound::resume_music();
			PSound::start_music(PFile::Path("music" PE_SEP "song01.xm"));   // the problem is we dont have the music that was playing previously, fixlater
			
	
	}

	/*****************************************************************************************/
	/* Gravity effect                                                                        */
	/*****************************************************************************************/
	
	if (sprite->weight != 0 && (sprite->jump_timer <= 0 || sprite->jump_timer >= 45) && !sprite->swimming)
		sprite->b += sprite->weight/1.25;// + sprite->b/1.5;

	if (gliding && sprite->b > 0) // If gliding
		sprite->b /= 1.3;//1.5;//3

	/*****************************************************************************************/
	/* Speed limits                                                                          */
	/*****************************************************************************************/

	if (sprite->b > 4.0)//4
		sprite->b = 4.0;//4

	if (sprite->b < -4.0)
		sprite->b = -4.0;

	//Limit speed 1
	if (sprite->a > max_speed)
		sprite->a = max_speed;

	if (sprite->a < -max_speed)
		sprite->a = -max_speed;

	/*****************************************************************************************/
	/* Blocks colision -                                                                     */
	/*****************************************************************************************/

	if (sprite->prototype->check_tiles){ //Find the tiles that the sprite occupies

		int palikat_x_lkm = (int)((sprite_width) /32)+4; //Number of blocks
		int palikat_y_lkm = (int)((sprite_height)/32)+4;

		int map_vasen = (int)(sprite_left) / 32; //Position in tile map
		int map_yla   = (int)(sprite_upper)   / 32;

		/*****************************************************************************************/
		/* Going through the blocks around the sprite->                                           */
		/*****************************************************************************************/

		//palikat_lkm = palikat_y_lkm*palikat_x_lkm;
		for (int y = 0; y < palikat_y_lkm; y++){
			for (int x = 0; x < palikat_x_lkm; x++) {
				int p = x + y*palikat_x_lkm;
				if ( p < 300 )
					if (!(sprite == Player_Sprite && dev_mode && PInput::Keydown(PInput::Y))){
						PK2BLOCK block = Block_Get(map_vasen+x-1,map_yla+y-1);
						Check_MapBlock(sprite, block);
					}
			}
		}
	}
	/*****************************************************************************************/
	/* If the sprite is under water                                                          */
	/*****************************************************************************************/

	if (sprite->in_water) {

		if (!sprite->prototype->can_swim || sprite->energy < 1) {

			sprite->b /= 2.0;
			sprite->a /= 1.05;

			if (sprite->jump_timer > 0 && sprite->jump_timer < 90)
				sprite->jump_timer--;
		}

		if (rand()%80 == 1)
			Particles_New(PARTICLE_SPARK,sprite->x-4,sprite->y,0,-0.5-rand()%2,rand()%30+30,0,32);
	}

	if (in_water != sprite->in_water&&!sprite->initial_update) { // Sprite comes in or out from water
		Effect_Splash(sprite->x, sprite->y, 32);
		Play_GameSFX(Episode->sfx.splash_sound, 100, (int)sprite->x, (int)sprite->y, SOUND_SAMPLERATE, true);
	}

	/*****************************************************************************************/
	/* Sprite weight                                                                         */
	/*****************************************************************************************/

	sprite->weight = sprite->initial_weight;
	sprite->weight_button = sprite->weight;

	if(sprite->energy < 1){
		if(sprite->prototype->has_dead_weight){
			sprite->weight = sprite->prototype->dead_weight;
		}
		else if(sprite->weight == 0){ // Fall when is death
			sprite->weight = 1;
		}
	}

	/*****************************************************************************************/
	/* Sprite collision with other sprites                                                   */
	/*****************************************************************************************/

	int how_destroyed;
	double sprite2_yla; // kyykistymiseen liittyv�
	PK2BLOCK spritepalikka;

	//Compare this sprite with every sprite in the game
	for (SpriteClass* sprite2 : Game->spritesHandler.Sprites_List) {
		if (sprite2 != sprite && sprite2->active && !sprite2->removed) {
			if (sprite2->crouched)
				sprite2_yla = sprite2->prototype->height / 3;//1.5;
			else
				sprite2_yla = 0;

			if (sprite2->prototype->is_wall && sprite->prototype->check_tiles && sprite2->energy>0) { //If there is a block sprite active

				if (sprite->x-sprite_width/2 +sprite->a  <= sprite2->x + sprite2->prototype->width /2 &&
					sprite->x+sprite_width/2 +sprite->a  >= sprite2->x - sprite2->prototype->width /2 &&
					sprite->y-sprite_height/2+sprite->b <= sprite2->y + sprite2->prototype->height/2 &&
					sprite->y+sprite_height/2+sprite->b >= sprite2->y - sprite2->prototype->height/2)
				{
					spritepalikka.id = 0;
					spritepalikka.bottom   = (int)sprite2->y + sprite2->prototype->height/2;
					spritepalikka.right = (int)sprite2->x + sprite2->prototype->width/2;
					spritepalikka.left = (int)sprite2->x - sprite2->prototype->width/2;
					spritepalikka.top   = (int)sprite2->y - sprite2->prototype->height/2;

					spritepalikka.water  = false;

					spritepalikka.permeable = false;

					spritepalikka.right_side   = BLOCK_WALL;
					spritepalikka.left_side = BLOCK_WALL;
					spritepalikka.top_side       = BLOCK_WALL;
					spritepalikka.bottom_side       = BLOCK_WALL;

					if (!sprite->prototype->is_wall){
						if (!sprite2->prototype->is_wall_down)
							spritepalikka.bottom_side = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_up)
							spritepalikka.top_side = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_right)
							spritepalikka.right_side = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_left)
							spritepalikka.left_side = BLOCK_BACKGROUND;
					}

					if (sprite2->a > 0)
						spritepalikka.id = BLOCK_LIFT_HORI;

					if (sprite2->b > 0)
						spritepalikka.id = BLOCK_LIFT_VERT;

					if (!(sprite == Player_Sprite && dev_mode && PInput::Keydown(PInput::Y)))
						Check_SpriteBlock(sprite, spritepalikka); //Colision sprite and sprite block
				}
			}

			if (sprite->x <= sprite2->x + sprite2->prototype->width /2 &&
				sprite->x >= sprite2->x - sprite2->prototype->width /2 &&
				sprite->y/*top*/ <= sprite2->y + sprite2->prototype->height/2 &&
				sprite->y/*bottom*/ >= sprite2->y - sprite2->prototype->height/2 + sprite2_yla)
			{
				// sprites with same index change directions when touch
				if (sprite->prototype == sprite2->prototype &&
					sprite2->energy > 0/* && sprite->player == 0*/)
				{
					if (sprite->x < sprite2->x)
						oikealle = false;
					if (sprite->x > sprite2->x)
						vasemmalle = false;
					if (sprite->y < sprite2->y)
						alas = false;
					if (sprite->y > sprite2->y)
						ylos = false;
				}

				if (sprite->HasAI(AI_ARROW_BARRIER)) {

					if (sprite2->HasAI(AI_ARROW_RIGHT)) {
						sprite->a = sprite->prototype->max_speed / 3.5;
						sprite->b = 0;
					}
					else if (sprite2->HasAI(AI_ARROW_LEFT)) {
						sprite->a = sprite->prototype->max_speed / -3.5;
						sprite->b = 0;
					}

					if (sprite2->HasAI(AI_ARROW_UP)) {
						sprite->b = sprite->prototype->max_speed / -3.5;
						sprite->a = 0;
					}
					else if (sprite2->HasAI(AI_ARROW_DOWN)) {
						sprite->b = sprite->prototype->max_speed / 3.5;
						sprite->a = 0;
					}
				}

				/* sprites can exchange information about a player's whereabouts */ //TODO - test this sometime
	/*			if (sprite->seen_player_x != -1 && sprite2->seen_player_x == -1)
				{
					sprite2->seen_player_x = sprite->seen_player_x + rand()%30 - rand()%30;
					sprite->seen_player_x = -1;
				} */

				// If two sprites from different teams touch each other
				if (sprite->enemy != sprite2->enemy && sprite->parent_sprite != sprite2 &&
				!sprite->prototype->ambient && !sprite2->prototype->ambient) {
					
					if (sprite2->prototype->type != TYPE_BACKGROUND &&
						sprite->prototype->type   != TYPE_BACKGROUND &&
						sprite2->prototype->type != TYPE_TELEPORT &&
						sprite2->damage_timer == 0 &&
						sprite->damage_timer == 0 &&
						sprite2->energy > 0 &&
						sprite->energy > 0 &&
						sprite2->damage_taken < 1)
					{

						if (sprite->super_mode_timer > 0 && sprite2->super_mode_timer == 0) {
							sprite2->damage_taken = 500;
							sprite2->damage_taken_type = DAMAGE_ALL;
						}
						if (sprite2->super_mode_timer > 0 && sprite->super_mode_timer == 0) {
							sprite->damage_taken = 500;
							sprite->damage_taken_type = DAMAGE_ALL;
						}
						
						//Bounce on the sprite head
						if (sprite2->b > 2 && sprite2->weight >= 0.5 &&
							sprite2->y < sprite->y && !sprite->prototype->is_wall &&
							sprite->prototype->how_destroyed != FX_DESTRUCT_INDESTRUCTIBLE &&
							sprite2->CanDamageOnCollision(sprite))
						{
							if (sprite2->super_mode_timer)
								sprite->damage_taken = 500;
							else
								sprite->damage_taken = (int)(sprite2->weight+sprite2->b/4);
							sprite->damage_taken_type = DAMAGE_DROP;
							sprite2->jump_timer = 1;
							if (sprite2->HasAI(AI_EGG2)) // Egg bounced, then crack
								sprite2->damage_taken = sprite2->prototype->energy;
						}

						// If there is another sprite damaging
						if (sprite->prototype->damage > 0 && sprite2->prototype->type != TYPE_BONUS &&
						sprite->CanDamageOnCollision(sprite2)) {
							
							sprite2->damage_taken        = sprite->prototype->damage;
							sprite2->damage_taken_type = sprite->prototype->damage_type;
							
							if ( !(sprite2->player && sprite2->invisible_timer) ) //If sprite2 isn't a invisible player
								sprite->attack1_timer = sprite->prototype->attack1_time; //Then sprite attack??

							// The projectiles are shattered by shock
							if (sprite2->prototype->type == TYPE_PROJECTILE) {
								sprite->damage_taken = 1;//sprite2->prototype->damage;
								sprite->damage_taken_type = sprite2->prototype->damage_type;
							}

							if (sprite->prototype->type == TYPE_PROJECTILE) {
								sprite->damage_taken = 1;//sprite2->prototype->damage;
								sprite->damage_taken_type = sprite2->prototype->damage_type;
							}
						}
					}
				}

				// Calculate cumulative weight to determine if the sprite can push buttons.
				if (sprite->weight > 0)
					sprite->weight_button += sprite2->prototype->weight;

			}
		}
	}

	/*****************************************************************************************/
	/* If the sprite has suffered damage                                                     */
	/*****************************************************************************************/
	if (sprite->damage_taken != 0 && sprite->super_mode_timer != 0) {
		sprite->damage_taken = 0;
		sprite->damage_taken_type = DAMAGE_NONE;
	}

	if (sprite->damage_taken != 0 && sprite->energy > 0){
		
		if ((sprite->prototype->immunity_type != sprite->damage_taken_type
		|| sprite->prototype->immunity_type == DAMAGE_NONE)
		&& sprite->prototype->how_destroyed != FX_DESTRUCT_INDESTRUCTIBLE){
			SpriteOnDamage(sprite);
		}

		sprite->damage_taken = 0;
		sprite->damage_taken_type = DAMAGE_NONE;


		/*****************************************************************************************/
		/* If the sprite is destroyed                                                            */
		/*****************************************************************************************/

		if (sprite->energy < 1) {
			how_destroyed = sprite->prototype->how_destroyed;

			if (how_destroyed != FX_DESTRUCT_INDESTRUCTIBLE) {
				SpriteOnDeath(sprite);
			} else
				sprite->energy = 1;
		}
	}

	if (sprite->damage_timer == 0)
		sprite->damage_taken_type = DAMAGE_NONE;


	/*****************************************************************************************/
	/* Revisions                                                                             */
	/*****************************************************************************************/

	/*if (&sprite == Player_Sprite && dev_mode) {

		oikealle   = true;
		vasemmalle = true;
		ylos       = true;
		alas       = true;
		printf("%f\n", sprite->b);

	}*/

	if (!oikealle)
		if (sprite->a > 0)
			sprite->a = 0;

	if (!vasemmalle)
		if (sprite->a < 0)
			sprite->a = 0;

	if (!ylos){
		if (sprite->b < 0)
			sprite->b = 0;

		if (!hyppy_maximissa)
			sprite->jump_timer = 95;//sprite->prototype->max_jump * 2;
	}

	if (!alas)
		if (sprite->b >= 0){ //If sprite is falling
			if (sprite->jump_timer > 0){
				if (sprite->jump_timer >= 90+10){
					Play_GameSFX(Episode->sfx.pump_sound,30,(int)sprite->x, (int)sprite->y,
				                  int(25050-sprite->weight*3000),true);

					//Particles_New(	PARTICLE_DUST_CLOUDS,sprite->x+rand()%5-rand()%5-10,sprite_bottom+rand()%3-rand()%3,
					//			  0,-0.2,rand()%50+20,0,0);

					if (rand()%7 == 1) {
						Particles_New(PARTICLE_SMOKE,sprite->x+rand()%5-rand()%5-10,sprite_bottom+rand()%3-rand()%3,
									  	   0.3,-0.1,450,0,0);
						Particles_New(PARTICLE_SMOKE,sprite->x+rand()%5-rand()%5-10,sprite_bottom+rand()%3-rand()%3,
									  	   -0.3,-0.1,450,0,0);
					}

					if (sprite->weight > 1) {
						Game->vibration = 34 + int(sprite->weight * 20);
						PInput::Vibrate(500);
					}
				}

				sprite->jump_timer = 0;
			}

			sprite->b = 0;
		}

	/*****************************************************************************************/
	/* Set correct values                                                                    */
	/*****************************************************************************************/

	if (sprite->b > 4.0)
		sprite->b = 4.0;

	if (sprite->b < -4.0)
		sprite->b = -4.0;

	//Limit speed 2
	if (sprite->a > max_speed)
		sprite->a = max_speed;

	if (sprite->a < -max_speed)
		sprite->a = -max_speed;

	if (sprite->energy < 0)
		sprite->energy = 0;

	if (sprite->energy > sprite->prototype->energy)
		sprite->energy = sprite->prototype->energy;

	if (sprite->damage_timer == 0 || sprite->player) {
		sprite->x += sprite->a;
		sprite->y += sprite->b;
	}

	if (sprite == Player_Sprite || sprite->energy < 1) {
		double kitka = 1.04;

		if (Game->map.weather == WEATHER_RAIN || Game->map.weather == WEATHER_RAIN_LEAVES)
			kitka = 1.03; // Slippery ground in the rain

		if (Game->map.weather == WEATHER_SNOW)
			kitka = 1.01; // And even more on snow

		if (!alas)
			sprite->a /= kitka;
		else
			sprite->a /= 1.03;//1.02//1.05

		sprite->b /= 1.25;
	}

	sprite->can_move_right = oikealle;
	sprite->can_move_left = vasemmalle;
	sprite->can_move_down = alas;
	sprite->can_move_up = ylos;

	/*
	sprite->weight = sprite->prototype->weight;

	if (sprite->energy < 1 && sprite->weight == 0)
		sprite->weight = 1;*/

	if (sprite->jump_timer < 0)
		sprite->can_move_down = false;

	//sprite->crouched   = false;

	/*****************************************************************************************/
	/* AI                                                                                    */
	/*****************************************************************************************/
	
	if (!sprite->player) {

		for(SpriteAI::AI_Class& ai : sprite->prototype->AI_f){
			if(!ai.apply_to_creatures)continue;
			
			if(ai.trigger==AI_TRIGGER_ANYWAY){
				ai.func(sprite);
			}
			else if(sprite->energy>0){

				if(ai.info_id!=-1){
					DisplayInfo(sprite, ai.info_id);
				}
				else if(ai.trigger==AI_TRIGGER_ALIVE){
					ai.func(sprite);
				}
			}
		}
	}

	//if (kaiku == 1 && sprite->prototype->prototype == TYPE_PROJECTILE && sprite->prototype->damage_type == DAMAGE_NOISE &&
	//	sprite->prototype->sounds[SOUND_ATTACK1] > -1)
	//	Play_GameSFX(sprite->prototype->sounds[SOUND_ATTACK1],20, (int)sprite->x, (int)sprite->y,
	//				  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);


	/*****************************************************************************************/
	/* Set game area to sprite                                                               */
	/*****************************************************************************************/

	if (sprite->x < 0)
		sprite->x = 0;

	if (sprite->y < -sprite_height)
		sprite->y = -sprite_height;

	if (sprite->x > PK2MAP_MAP_WIDTH*32)
		sprite->x = PK2MAP_MAP_WIDTH*32;

	// If the sprite falls under the lower edge of the map
	if (sprite->y > PK2MAP_MAP_HEIGHT*32 + sprite_height) {

		sprite->y = PK2MAP_MAP_HEIGHT*32 + sprite_height;
		sprite->energy = 0;
		sprite->removed = true;

		if (sprite->weight_button >= 1)
			Game->vibration = 50;
	}

	if (sprite->a > max_speed)
		sprite->a = max_speed;

	if (sprite->a < -max_speed)
		sprite->a = -max_speed;


	/*****************************************************************************************/
	/* Attacks 1 and 2                                                                       */
	/*****************************************************************************************/

	// If the sprite is ready and isn't crouching
	if ((sprite->charging_timer == 0 || sprite->self_destruction) && !sprite->crouched ) {
		// the attack has just started
		if (sprite->attack1_timer == sprite->prototype->attack1_time) {
			// provides recovery time, after which the sprite can attack again
			sprite->charging_timer = sprite->prototype->charge_time;
			if(sprite->charging_timer == 0) sprite->charging_timer = 5;
			// if you don't have your own charging time ...
			if (sprite->ammo1 != nullptr && sprite->prototype->charge_time == 0)
			// ... and the projectile has, take charge_time from the projectile
				if (sprite->ammo1->projectile_charge_time > 0)
					sprite->charging_timer = sprite->ammo1->projectile_charge_time;

			// attack sound
			Play_GameSFX(sprite->prototype->sounds[SOUND_ATTACK1],100, (int)sprite->x, (int)sprite->y,
						  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

			if (sprite->ammo1 != nullptr) {
				Game->spritesHandler.addSpriteAmmo(sprite->ammo1,sprite->x, sprite->y, sprite);
		//		if (Level_Prototypes_List[sprite->ammo1].sounds[SOUND_ATTACK1] > -1)
		//			Play_GameSFX(Level_Prototypes_List[sprite->ammo1].sounds[SOUND_ATTACK1],100, (int)sprite->x, (int)sprite->y,
		//						  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);
			}
		}

		// Same as attack 1
		if (sprite->attack2_timer == sprite->prototype->attack2_time) {
			sprite->charging_timer = sprite->prototype->charge_time;
			if(sprite->charging_timer == 0) sprite->charging_timer = 5;
			if (sprite->ammo2 != nullptr && sprite->prototype->charge_time == 0)
				if (sprite->ammo2->projectile_charge_time > 0)
					sprite->charging_timer = sprite->ammo2->projectile_charge_time;

			Play_GameSFX(sprite->prototype->sounds[SOUND_ATTACK2],100,(int)sprite->x, (int)sprite->y,
						  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

			if (sprite->ammo2 != nullptr) {
				Game->spritesHandler.addSpriteAmmo(sprite->ammo2,sprite->x, sprite->y, sprite);

		//		if (Level_Prototypes_List[sprite->ammo2].sounds[SOUND_ATTACK1] > -1)
		//			Play_GameSFX(Level_Prototypes_List[sprite->ammo2].sounds[SOUND_ATTACK1],100, (int)sprite->x, (int)sprite->y,
		//						  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

			}
		}
	}

	// Random sounds
	if (sprite->prototype->sounds[SOUND_RANDOM] != -1 && rand()%200 == 1 && sprite->energy > 0)
		Play_GameSFX(sprite->prototype->sounds[SOUND_RANDOM],80,(int)sprite->x, (int)sprite->y,
					  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

	sprite->initial_update = false;
}

void UpdateBonusSprite(SpriteClass* sprite){

	SpriteClass* Player_Sprite = Game->spritesHandler.Player_Sprite;

	sprite_width  = sprite->prototype->width;
	sprite_height = sprite->prototype->height;

	sprite_left = sprite->x - sprite_width  / 2;
	sprite_right = sprite->x + sprite_width  / 2;
	sprite_upper   = sprite->y - sprite_height / 2;
	sprite_bottom   = sprite->y + sprite_height / 2;

	oikealle	= true,
	vasemmalle	= true,
	ylos		= true,
	alas		= true;

	bool in_water = sprite->in_water;
	if (sprite->damage_timer > 0)
		sprite->damage_timer--;

	if (sprite->charging_timer > 0)
		sprite->charging_timer--;

	if (sprite->mutation_timer > 0)	// aika muutokseen
		sprite->mutation_timer --;

	// Hyppyyn liittyv�t seikat

	if (Game->button_vibration + Game->vibration > 0 && sprite->jump_timer == 0)
		sprite->jump_timer = sprite->prototype->max_jump / 2;

	if (sprite->jump_timer > 0 && sprite->jump_timer < sprite->prototype->max_jump)
	{
		sprite->jump_timer ++;
		sprite->b = (sprite->prototype->max_jump - sprite->jump_timer)/-4.0;//-2
	}

	if (sprite->b > 0)
		sprite->jump_timer = sprite->prototype->max_jump;



	if (sprite->weight != 0)	// jos bonuksella on weight, tutkitaan ymp�rist�
	{
		// o
		//
		// |  Gravity
		// V

		sprite->b += sprite->weight + sprite->b/1.25;

		if (sprite->in_water)
		{
			if (sprite->b > 0)
				sprite->b /= 2.0;

			if (rand()%80 == 1)
				Particles_New(PARTICLE_SPARK,sprite->x-4,sprite->y,0,-0.5-rand()%2,rand()%30+30,0,32);
		}

		sprite->in_water = false;

		sprite->weight_button = sprite->weight;

		/* TOISET SPRITET */

		PK2BLOCK spritepalikka; 

		for (SpriteClass* sprite2 : Game->spritesHandler.Sprites_List) {
			if (sprite2 != sprite && !sprite2->removed && sprite->respawn_timer==0) {
				if (sprite2->prototype->is_wall && sprite->prototype->check_tiles) {
					if (sprite->x-sprite_width/2 +sprite->a <= sprite2->x + sprite2->prototype->width /2 &&
						sprite->x+sprite_width/2 +sprite->a >= sprite2->x - sprite2->prototype->width /2 &&
						sprite->y-sprite_height/2+sprite->b <= sprite2->y + sprite2->prototype->height/2 &&
						sprite->y+sprite_height/2+sprite->b >= sprite2->y - sprite2->prototype->height/2)
					{
						spritepalikka.id = 0;
						spritepalikka.bottom   = (int)sprite2->y + sprite2->prototype->height/2;
						spritepalikka.right = (int)sprite2->x + sprite2->prototype->width/2;
						spritepalikka.left = (int)sprite2->x - sprite2->prototype->width/2;
						spritepalikka.top   = (int)sprite2->y - sprite2->prototype->height/2;

						spritepalikka.bottom_side       = BLOCK_WALL;
						spritepalikka.top_side       = BLOCK_WALL;
						spritepalikka.right_side   = BLOCK_WALL;
						spritepalikka.left_side = BLOCK_WALL;

						if (!sprite2->prototype->is_wall_down)
							spritepalikka.bottom_side		 = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_up)
							spritepalikka.top_side		 = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_right)
							spritepalikka.right_side   = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_left)
							spritepalikka.left_side = BLOCK_BACKGROUND;


						spritepalikka.water  = false;

						spritepalikka.permeable = false;

						// Bonus accepts just walls?
						/*spritepalikka.right_side   = BLOCK_WALL;
						spritepalikka.left_side = BLOCK_WALL;
						spritepalikka.top_side       = BLOCK_WALL;
						spritepalikka.bottom_side       = BLOCK_WALL;*/

						Check_SpriteBlock(sprite, spritepalikka); //Colision bonus and sprite block
					}
				}

				if (sprite->x < sprite2->x + sprite2->prototype->width/2 &&
					sprite->x > sprite2->x - sprite2->prototype->width/2 &&
					sprite->y < sprite2->y + sprite2->prototype->height/2 &&
					sprite->y > sprite2->y - sprite2->prototype->height/2 &&
					sprite->damage_timer == 0)
				{
					if (sprite2->prototype->type != TYPE_BONUS &&
						!(sprite2 == Player_Sprite && sprite->prototype->how_destroyed != FX_DESTRUCT_INDESTRUCTIBLE))
						sprite->a += sprite2->a*(rand()%4);

					// lis�t��n spriten painoon sit� koskettavan toisen spriten weight
					sprite->weight_button += sprite2->prototype->weight;

					// samanmerkkiset spritet vaihtavat suuntaa t�rm�tess��n
					if (sprite->prototype == sprite2->prototype &&
						sprite2->energy > 0)
					{
						if (sprite->x < sprite2->x) {
							sprite2->a += sprite->a / 3.0;
							oikealle = false;
						}
						if (sprite->x > sprite2->x) {
							sprite2->a += sprite->a / 3.0;
							vasemmalle = false;
						}
					}

				}
			}
		}

		// Tarkistetaan ettei menn� mihink��n suuntaan liian kovaa.

		if (sprite->b > 4)
			sprite->b = 4;

		if (sprite->b < -4)
			sprite->b = -4;

		if (sprite->a > 3)
			sprite->a = 3;

		if (sprite->a < -3)
			sprite->a = -3;

		// Lasketaan

		if (sprite->prototype->check_tiles)
		{

			int palikat_x_lkm = (int)((sprite_width) /32)+4;
			int palikat_y_lkm = (int)((sprite_height)/32)+4;

			int map_vasen = (int)(sprite_left)/32;
			int map_yla   = (int)(sprite_upper)/32;

			// Tutkitaan t�rm��k� palikkaan

			for (int y = 0; y < palikat_y_lkm; y++)
				for (int x = 0; x < palikat_x_lkm; x++){
					PK2BLOCK block = Block_Get(map_vasen+x-1,map_yla+y-1);
					Check_MapBlock(sprite, block);
				}

		}

		if (in_water != sprite->in_water && !sprite->initial_update) {
			Effect_Splash((int)sprite->x,(int)sprite->y,32);
			Play_GameSFX(Episode->sfx.splash_sound, 100, (int)sprite->x, (int)sprite->y, SOUND_SAMPLERATE, true);
		}


		if (!oikealle)
		{
			if (sprite->a > 0)
				sprite->a = -sprite->a/1.5;
		}

		if (!vasemmalle)
		{
			if (sprite->a < 0)
				sprite->a = -sprite->a/1.5;
		}

		if (!ylos)
		{
			if (sprite->b < 0)
				sprite->b = 0;

			sprite->jump_timer = sprite->prototype->max_jump;
		}

		if (!alas)
		{
			if (sprite->b >= 0)
			{
				if (sprite->jump_timer > 0)
				{
					sprite->jump_timer = 0;
				//	if (/*sprite->b == 4*/!maassa)
				//		Play_GameSFX(pump_sound,20,(int)sprite->x, (int)sprite->y,
				//				      int(25050-sprite->prototype->weight*4000),true);
				}

				if (sprite->b > 2)
					sprite->b = -sprite->b/(3+rand()%2);
				else
					sprite->b = 0;
			}
			//sprite->a /= kitka;
			sprite->a /= 1.07;
		}
		else
		{
			sprite->a /= 1.02;
		}

		sprite->b /= 1.5;

		if (sprite->b > 4)
			sprite->b = 4;

		if (sprite->b < -4)
			sprite->b = -4;

		if (sprite->a > 4)
			sprite->a = 4;

		if (sprite->a < -4)
			sprite->a = -4;

		sprite->x += sprite->a;
		sprite->y += sprite->b;


		sprite->can_move_right = oikealle;
		sprite->can_move_left = vasemmalle;
		sprite->can_move_down = alas;
		sprite->can_move_up = ylos;
	}
	else	// jos spriten weight on nolla, tehd��n spritest� "kelluva"
	{
		sprite->y = sprite->orig_y + cos_table(degree+(sprite->orig_x+sprite->orig_y)) / 3.0;
	}

	sprite->weight = sprite->initial_weight;

	int how_destroyed;

	// Test if player touches bonus
	if (sprite->x < Player_Sprite->x + Player_Sprite->prototype->width/2 &&
		sprite->x > Player_Sprite->x - Player_Sprite->prototype->width/2 &&
		sprite->y < Player_Sprite->y + Player_Sprite->prototype->height/2 &&
		sprite->y > Player_Sprite->y - Player_Sprite->prototype->height/2 &&
		sprite->damage_timer == 0)
	{
		if (sprite->energy > 0 && Player_Sprite->energy > 0)
		{
			if (sprite->prototype->score != 0) {
				if(sprite->prototype->big_apple){
					Game->apples_got++;
				}

				Game->score_increment += sprite->prototype->score;
				
				if (!sprite->HasAI(AI_BONUS_CLOCK)) {
					int font = sprite->prototype->score >= 50? fontti2 : fontti1;
					Fadetext_New(font, std::to_string(sprite->prototype->score), (int)sprite->x-8,(int)sprite->y-8,100);
				}

			}

			if (sprite->HasAI(AI_BONUS_CLOCK)) {
				
				int increase_time = sprite->prototype->charge_time * TIME_FPS;
				Game->timeout += increase_time;

				float shown_time = float(increase_time) / 60;
				int minutes = int(shown_time / 60);
				int seconds = int(shown_time) % 60;

				std::ostringstream os;
				os<<minutes<<":";
				if(seconds<10){
					os<<"0";
				}
				os<<seconds;
				
				Fadetext_New(fontti1,os.str(),(int)sprite->x-15,(int)sprite->y-8,100);
			}

			if (sprite->HasAI(AI_BONUS_INVISIBILITY))
				Player_Sprite->invisible_timer = sprite->prototype->charge_time;

			if (sprite->HasAI(AI_BONUS_SUPERMODE)) {
				Player_Sprite->super_mode_timer = sprite->prototype->charge_time;
				//PSound::play_overlay_music();
				PSound::start_music(PFile::Path("music" PE_SEP "super.xm"));   // the problem is this will most likely overwrite the current music, fixlater
			}

			//Game->map.spritet[(int)(sprite->orig_x/32) + (int)(sprite->orig_y/32)*PK2MAP_MAP_WIDTH] = 255;

			if (sprite->prototype->how_destroyed != FX_DESTRUCT_INDESTRUCTIBLE)
				Player_Sprite->energy -= sprite->prototype->damage;

			how_destroyed = sprite->prototype->how_destroyed;

			if (how_destroyed != FX_DESTRUCT_INDESTRUCTIBLE)
			{
				if (how_destroyed >= FX_DESTRUCT_ANIMATED)
					how_destroyed -= FX_DESTRUCT_ANIMATED;
				else
				{
					if (sprite->prototype->can_open_locks)
					{
						Game->keys--;

						if (Game->keys < 1)
							Game->Open_Locks();
					}

					sprite->removed = true;
				}

				if (sprite->HasAI(AI_REBORN)) {

					sprite->respawn_timer = sprite->prototype->charge_time;
					sprite->energy = sprite->prototype->energy;
					sprite->removed = false;
					sprite->x = sprite->orig_x;
					sprite->y = sprite->orig_y;
				}

				if (sprite->prototype->bonus != nullptr)
					if (Gifts_Add(sprite->prototype->bonus))
						Game->Show_Info(tekstit->Get_Text(PK_txt.game_newitem));

				//potion transformation
				if (sprite->prototype->transformation != nullptr)
				{
					if (sprite->prototype->transformation->first_ai() != AI_BONUS)
					{
						/**
						 * @brief 
						 * Robohead turning into rooster is no longer considered enemy.
						 */
						if(Player_Sprite->enemy && !sprite->prototype->transformation->enemy){
							Player_Sprite->enemy = false;
						}

						Player_Sprite->prototype = sprite->prototype->transformation;
						Player_Sprite->ammo1 = Player_Sprite->prototype->ammo1;
						Player_Sprite->ammo2 = Player_Sprite->prototype->ammo2;
						Player_Sprite->initial_weight = Player_Sprite->prototype->weight;
						Player_Sprite->y -= Player_Sprite->prototype->height/2;
						
						int infotext = Episode->infos.Search_Id("pekka transformed");
						if (infotext != -1)
							Game->Show_Info(Episode->infos.Get_Text(infotext));
						//Game->Show_Info("pekka has been transformed!");
					}
				}

				if (sprite->prototype->ammo1 != nullptr) {
					Player_Sprite->ammo1 = sprite->prototype->ammo1;

					int infotext = Episode->infos.Search_Id("new egg attack");
					if (infotext != -1)
						Game->Show_Info(Episode->infos.Get_Text(infotext));
					else
						Game->Show_Info(tekstit->Get_Text(PK_txt.game_newegg));
				}

				if (sprite->prototype->ammo2 != nullptr) {
					Player_Sprite->ammo2 = sprite->prototype->ammo2;

					int infotext = Episode->infos.Search_Id("new doodle attack");
					if (infotext != -1)
						Game->Show_Info(Episode->infos.Get_Text(infotext));
					else
						Game->Show_Info(tekstit->Get_Text(PK_txt.game_newdoodle));
				}

				Play_GameSFX(sprite->prototype->sounds[SOUND_DESTRUCTION],100, (int)sprite->x, (int)sprite->y,
							  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

				Effect_Destruction(how_destroyed, (u32)sprite->x, (u32)sprite->y);
			}
		}
	}
	for(const SpriteAI::AI_Class& ai: sprite->prototype->AI_f){

		if(!ai.apply_to_bonuses)continue;

		if((sprite->energy>0 && ai.trigger==AI_TRIGGER_ALIVE)
		||  ai.trigger==AI_TRIGGER_ANYWAY){
			ai.func(sprite);
		}
	}

	/* The energy doesn't matter that the player is a bonus item */
	if (sprite->player)
		sprite->energy = 0;

	sprite->initial_update = false;
}

void UpdateBackgroundSprite(SpriteClass* sprite, double &yl){
	
	for(const SpriteAI::AI_Class& ai: sprite->prototype->AI_f){
		if(!ai.apply_to_backgrounds)continue;

		if((ai.trigger == AI_TRIGGER_ALIVE && sprite->energy>0)||ai.trigger == AI_TRIGGER_ANYWAY){

			if(ai.func==nullptr){
				switch (ai.id)
				{
				case AI_BACKGROUND_MOON:
					yl += screen_height/3+50;
					break;
				case AI_BACKGROUND_HORIZONTAL_PARALLAX:
					yl = 0;
					break;
				default:
					break;
				}
			}
			else{
				ai.func(sprite);
			}
		}
	}

	if (sprite->charging_timer > 0)
		sprite->charging_timer--;

	if (sprite->mutation_timer > 0)	// aika muutokseen
		sprite->mutation_timer --;

	/**
	 * @brief 
	 * To allow self destruction
	 */
	if(sprite->damage_taken_type == DAMAGE_ALL){
		sprite->damage_taken = 0;
		sprite->damage_taken_type = DAMAGE_NONE;
		sprite->energy = 0;
		SpriteOnDeath(sprite);
	}
}