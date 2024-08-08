//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include <sstream>
#include "playing_screen.hpp"


#include "engine/Piste.hpp"

#include <cstring>

#include "game/game.hpp"
#include "game/gifts.hpp"
#include "game/physics.hpp"

#include "gfx/text.hpp"

#include "gfx/particles.hpp"
#include "gfx/bg_particles.hpp"

#include "gfx/effect.hpp"
#include "gui.hpp"
#include "episode/episodeclass.hpp"
#include "language.hpp"
#include "sfx.hpp"
#include "system.hpp"
#include "settings/settings.hpp"

PlayingScreen::PlayingScreen(){

}

PlayingScreen::~PlayingScreen(){
	
}

void PlayingScreen::Draw_InGame_DebugInfo() {
	int vali, fy = 70;

	PDraw::set_offset(640, 480);

	vali = PDraw::font_write(fontti1,"sprites: ",10,fy);
	PDraw::font_write(fontti1,std::to_string(Game->spritePrototypes.size()),10+vali,fy);
	fy += 10;

	vali = PDraw::font_write(fontti1,"active sprites: ",10,fy);
	PDraw::font_write(fontti1,std::to_string(debug_active_sprites),10+vali,fy);
	fy += 10;

	vali = PDraw::font_write(fontti1,"animated sprites: ",10,fy);
	PDraw::font_write(fontti1,std::to_string(debug_drawn_sprites),10+vali,fy);
	fy += 10;

	for (std::size_t i = 0; i < 40; i++) {
		PDraw::font_write(fontti1,std::to_string(i),410,10+i*10);
		PrototypeClass*proto = Game->spritePrototypes.get(i);

		if (proto == nullptr) {
			PDraw::font_write(fontti1,"-",430,10+i*10);
		} else {
			PDraw::font_write(fontti1,proto->filename,430,10+i*10);

			if (degree < 90)
				PDraw::font_write(fontti1,proto->transformation_str,545,10+i*10);
			else if (degree < 180)
				PDraw::font_write(fontti1,proto->bonus_str,545,10+i*10);
			else if (degree < 270)
				PDraw::font_write(fontti1,proto->ammo1_str,545,10+i*10);
			else
				PDraw::font_write(fontti1,proto->ammo2_str,545,10+i*10);
		}
	}

	for (uint i = 0; i < Episode->level_count; i++)
		if (strcmp(Episode->levels_list[i].nimi,"")!=0)
			PDraw::font_write(fontti1,Episode->levels_list[i].nimi,0,240+i*10);


	SpriteClass* Player_Sprite = Game->playerSprite;
	
	PDraw::font_write(fontti1, std::to_string(Player_Sprite->x), 10, 410);
	PDraw::font_write(fontti1, std::to_string(Player_Sprite->y), 10, 420);
	PDraw::font_write(fontti1, std::to_string(Player_Sprite->b), 10, 430);
	PDraw::font_write(fontti1, std::to_string(Player_Sprite->a), 10, 440);
	PDraw::font_write(fontti1, Game->map_file, 10, 460);
	PDraw::font_write(fontti1, std::to_string(Player_Sprite->jump_timer), 270, 460);

	PDraw::font_write(fontti1, Episode->Get_Dir("").c_str(), 10, 470);

	PDraw::font_write(fontti1, std::to_string(Player_Sprite->super_mode_timer), 610, 470);
	PDraw::font_write(fontti1, std::to_string(Player_Sprite->invisible_timer), 610, 460);
	PDraw::font_write(fontti1, std::to_string(Game->button1), 610, 450);
	PDraw::font_write(fontti1, std::to_string(Game->button2), 610, 440);
	PDraw::font_write(fontti1, std::to_string(Game->button3), 610, 430);

	vali += PDraw::font_write(fontti1,std::to_string(Game->timeout),390,screen_height-10);

	PDraw::set_offset(screen_width, screen_height);
}

void PlayingScreen::Draw_InGame_DevKeys() {

	const char txt0[] = "dev mode";
	int char_w = PDraw::font_write(fontti1, txt0, 0, screen_height - 10) / strlen(txt0);
	int char_h = 10;

	const char help[] = "h: help";

	if (!PInput::Keydown(PInput::H)) {
		PDraw::font_write(fontti1, help, screen_width - strlen(help) * char_w, screen_height - 10);
		return;
	}
	const char txts[][32] = {
		"z: press buttons",
		"x: release buttons",
		"l: open locks",
		"k: open skull blocks",
		"t: toggle speed",
		"g: toggle transparency",
		"i: toggle debug info",
		"u: go up",
		"y: ghost mode",
		"r: back to start",
		"v: set invisible",
		"s: set super mode",
		"e: set energy to max",
		"a: set rooster",
		"end: end level",
		"mouse: move camera",
	};

	uint nof_txt = sizeof(txts) / 32;

	uint max_size = 0;
	
	for (uint i = 0; i < nof_txt; i++)
		if (strlen(txts[i]) > max_size) max_size = strlen(txts[i]);

	int posx = screen_width - max_size * char_w;
	int posy = screen_height - char_h * nof_txt;

	PDraw::screen_fill(posx - 4, posy - 4, screen_width, screen_height, 0);
	PDraw::screen_fill(posx - 2, posy - 2, screen_width, screen_height, 38);
	
	for (uint i = 0; i < nof_txt; i++)
		PDraw::font_write(fontti1, txts[i], posx, posy + i*10);
}




void PlayingScreen::Draw_InGame_Gifts() {

	int x,y;

	y = screen_height-35;//36
	x = Game->item_pannel_x + 35;//40

	for (int i=0;i<MAX_GIFTS;i++)
		if (Gifts_Get(i) != nullptr){
			Gifts_Draw(i, x, y);
			x += 38;
		}
}

void PlayingScreen::Draw_InGame_Lower_Menu() {
	//char luku[16];
	int x, y;

	//////////////
	// Draw time
	//////////////
	if (Game->has_time) {
		int vali = 0;
		float shown_time = float(Game->timeout) / 60;
		int min = int(shown_time/60);
		int sek = int(shown_time)%60;

		x = screen_width / 2 + 69;
		y = screen_height-39;
		PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.game_time),x,y-20);

		//sprintf(luku, "%i", min);
		vali += ShadowedText_Draw(std::to_string(min), x, y);
		vali += PDraw::font_write(fontti1,":",x+vali,y+9);

		if (sek < 10)
			vali += ShadowedText_Draw("0", x + vali, y);
		
		//sprintf(luku, "%i", sek);
		vali += ShadowedText_Draw(std::to_string(sek), x + vali, y);

	}

	/////////////////
	// Draw keys / enemies
	/////////////////
	if(Game->level.game_mode==GAME_MODE_KILL_ALL){
		x = screen_width / 2 + 210;
		y = screen_height - 39;
		PDraw::font_write(fontti1,"enemies:",x,y-20);
		ShadowedText_Draw(std::to_string(Game->enemies), x, y);

	}
	else if (Game->keys > 0){
		x = screen_width / 2 + 210;
		y = screen_height - 39;
		PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.game_keys),x,y-20);
		ShadowedText_Draw(std::to_string(Game->keys), x, y);
	}

	/////////////////
	// Draw Gifts
	/////////////////
	if (Gifts_Count() > 0 && Game->item_pannel_x < 10)
		Game->item_pannel_x++;

	if (Gifts_Count() == 0 && Game->item_pannel_x > -215)
		Game->item_pannel_x--;

	if (Game->item_pannel_x > -215)
		PDraw::image_cutclip(game_assets,Game->item_pannel_x,screen_height-60,
		                        1,216,211,266);
	if (Game->item_pannel_x > 5)
		PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.game_items),15,screen_height-65);

	Draw_InGame_Gifts();
}

void PlayingScreen::Draw_InGame_UI(){
	int vali = 20;
	int my = 14;

	/////////////////
	// Draw Energy
	/////////////////
	vali = PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.game_energy),60,my);

	SpriteClass* Player_Sprite = Game->playerSprite;
	ShadowedText_Draw(std::to_string(Player_Sprite->energy), 60 + vali, my);

	/////////////////
	// Draw Invisible
	/////////////////
	if(Player_Sprite->super_mode_timer > 0){
		vali = PDraw::font_write(fontti1,"supermode:",60,my+27);
		//sprintf(luku, "%i", Player_Sprite->invisible_timer/60);
		std::string super_mode_timer_s = std::to_string(Player_Sprite->super_mode_timer/60);
		PDraw::font_write(fontti2,super_mode_timer_s,60+vali+1,my+27+1);
		PDraw::font_write(fontti2,super_mode_timer_s,60+vali,my+27);
	}

	else if(Player_Sprite->invisible_timer > 0){
		vali = PDraw::font_write(fontti1,"invisible:",60,my+27);
		//sprintf(luku, "%i", Player_Sprite->invisible_timer/60);
		std::string invisible_timer_s = std::to_string(Player_Sprite->invisible_timer/60);
		PDraw::font_write(fontti2,invisible_timer_s,60+vali+1,my+27+1);
		PDraw::font_write(fontti2,invisible_timer_s,60+vali,my+27);
	}

	/////////////////
	// Draw Mini Apple
	/////////////////
	if (Game->apples_count > 0) {
		if (Game->apples_got == Game->apples_count)
			PDraw::image_cutcliptransparent(game_assets2, 
				45, 379, 13, 15, 
				my, my, 
				sin_table(degree)*1.5+60, COLOR_RED);
		else
			PDraw::image_cutcliptransparent(game_assets2, 
				45, 379, 13, 15, 
				my, my, 
				0, COLOR_GRAY);
	}

	/////////////////
	// Draw Score
	/////////////////
	vali = PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.game_score),230,my);
	//sprintf(luku, "%i", Game->score);		
	ShadowedText_Draw(std::to_string(Game->score), 230 + vali, my);

	/////////////////
	// Draw Ammunition
	/////////////////
	if (Player_Sprite->ammo2 != nullptr){
		PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.game_attack1), screen_width-170,my);
		Player_Sprite->ammo2->draw(screen_width-170,my+10,0);
	}

	if (Player_Sprite->ammo1 != nullptr){
		PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.game_attack2), screen_width-90,my+15);
		Player_Sprite->ammo1->draw(screen_width-90,my+25,0);
	}

	/////////////////
	// Draw Info
	/////////////////
	if (Game->info_timer > 0){
		int box_size = Game->info_text.size() * 8 + 8; // 300

		MAP_RECT alue = {screen_width/2-(box_size/2),60,screen_width/2+(box_size/2),60+20};

		if (Game->info_timer < 20){
			alue.top	+= (20 - Game->info_timer) / 2;
			alue.bottom -= (20 - Game->info_timer) / 2;
		}

		if (Game->info_timer > INFO_TIME - 20){
			alue.top	+= 10 - (INFO_TIME - Game->info_timer) / 2;
			alue.bottom -= 10 - (INFO_TIME - Game->info_timer) / 2;
		}

		PDraw::screen_fill(alue.left-1,alue.top-1,alue.right+1,alue.bottom+1,51);
		PDraw::screen_fill(alue.left,alue.top,alue.right,alue.bottom,38);

		if (Game->info_timer-11 >= 100)
			PDraw::font_write(fontti1,Game->info_text,alue.left+4,alue.top+4);
		else
			PDraw::font_writealpha_s(fontti1,Game->info_text,alue.left+4,alue.top+4,Game->info_timer-11);
	}
}

void PlayingScreen::Draw() {
	SpriteClass* Player_Sprite = Game->playerSprite;
	LevelSector* sector = Player_Sprite->level_sector;


	debug_drawn_sprites = 0;

	sector->drawBackground(Game->camera_x, Game->camera_y);

	sector->sprites.drawBGsprites(Game->camera_x, Game->camera_y, Game->paused, this->debug_drawn_sprites);

	if(Settings.draw_weather)BG_Particles::Draw(Game->camera_x, Game->camera_y);

	Game->level.drawBackgroundTiles(Game->camera_x,Game->camera_y, sector);

	sector->sprites.drawSprites(Game->camera_x, Game->camera_y, Game->paused, this->debug_drawn_sprites);

	Particles_DrawFront(Game->camera_x, Game->camera_y);

	Game->level.drawForegroundTiles(Game->camera_x,Game->camera_y, sector);

	sector->sprites.drawFGsprites(Game->camera_x, Game->camera_y, Game->paused, this->debug_drawn_sprites);

	if (Settings.draw_itembar)
		Draw_InGame_Lower_Menu();

	Fadetext_Draw();

	Draw_InGame_UI();

	if (draw_debug_info)
		Draw_InGame_DebugInfo();
	else {
		if (dev_mode)
			Draw_InGame_DevKeys();
		if (test_level)
			PDraw::font_write(fontti1, "testing level", 0, screen_height - 20);
		if (show_fps) {
			
			int fps = Piste::get_fps();
			int txt_size;

			if (fps >= 100)
				txt_size = PDraw::font_write(fontti1, "fps:", 570, 48);
			else
				txt_size = PDraw::font_write(fontti1, "fps: ", 570, 48);
			
			PDraw::font_write(fontti1, std::to_string(fps), 570 + txt_size, 48);
		
		}
		if (speedrun_mode) {
			PDraw::font_write(fontti1, std::to_string(Game->frame_count), 570, 38);
		}
	}

	if (Game->paused) {

		PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.game_paused),screen_width/2-82,screen_height/2-9);

	}

	if (Game->level_clear) {

		Wavetext_Draw(tekstit->Get_Text(PK_txt.game_clear),fontti2,screen_width/2-120,screen_height/2-9);

	} else if (Game->game_over) {
	
		if (Player_Sprite->energy < 1) {
			
			Wavetext_Draw(tekstit->Get_Text(PK_txt.game_ko),fontti2,screen_width/2-90,screen_height/2-9-10);

		} else if (Game->timeout < 1 && Game->has_time) {

			Wavetext_Draw(tekstit->Get_Text(PK_txt.game_timeout),fontti2,screen_width/2-67,screen_height/2-9-10);
		
		}

		Wavetext_Draw(tekstit->Get_Text(PK_txt.game_tryagain),fontti2,screen_width/2-75,screen_height/2-9+10);
	
	}
}

void PlayingScreen::Init(){

	if(PUtils::Is_Mobile()) {
		if (Settings.gui)
			GUI_Change(UI_GAME_BUTTONS);
		else 
			GUI_Change(UI_TOUCH_TO_START);
	}
	
	PDraw::set_offset(screen_width, screen_height);

	if (!Game->isStarted()) {

		Game->start();
		degree = 0;
	
	} else {
	
		degree = degree_temp;
	
	}
}

void PlayingScreen::Loop(){

	Game->update(this->debug_active_sprites);

	static bool skip_frame = false;

	if (Settings.double_speed) skip_frame = !skip_frame;
	else skip_frame = false;

	if (!skip_frame) {

		if (!skip_frame) {
			if(key_delay==0 && !Game->game_over&&!Game->level_clear){
				if (PInput::Keydown(PInput::ESCAPE) || PInput::Keydown(PInput::JOY_START) || Gui_menu || Gui_touch) {
					if(test_level)
						Fade_Quit();
					else {
						next_screen = SCREEN_MENU;
						degree_temp = degree;
					}
					key_delay = 20;
				}

				if (PInput::Keydown(PInput::I)) {

					if(dev_mode){
						draw_debug_info = !draw_debug_info;
					}
					else{
						show_fps = !show_fps;
					}					
					key_delay = 20;
				}
			}
		}

		this->Draw();

	} else {

		Piste::ignore_frame();

	}
	
	if (Game->exit_timer == 1 && !Is_Fading()) {
		if (Game->level_clear) {

			next_screen = SCREEN_SCORING;

		}
		else if(test_level){
			Piste::stop();
		}
		else {		
			delete Game;
			Game = nullptr;
			next_screen = SCREEN_MAP;
		}

	}

	// TODO - FIX
	if (next_screen == SCREEN_MENU) {

		int w, h;
		PDraw::image_getsize(bg_screen, w, h);
		if (w != screen_width) {
			PDraw::image_delete(bg_screen);
			bg_screen = PDraw::image_new(screen_width, screen_height);
		}
		PDraw::image_snapshot(bg_screen);

	}
}