//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include <sstream>
#include "menu_screen.hpp"

#include "settings/settings.hpp"
#include "settings/config_txt.hpp"

#include "gfx/touchscreen.hpp"
#include "game/game.hpp"
#include "gfx/text.hpp"
#include "sfx.hpp"
#include "language.hpp"
#include "episode/episodeclass.hpp"
#include "episode/mapstore.hpp"
#include "episode/save_legacy.hpp"
#include "system.hpp"
#include "version.hpp"

#include "engine/Piste.hpp"
#include "engine/PFilesystem.hpp"
#include "engine/PString.hpp"

#include <cstring>
#include <stdexcept>
#include <SDL_system.h>

void MenuScreen::Draw_BGSquare(int left, int top, int right, int bottom, u8 pvari){
	
	if (Episode)
		return;

	if (bg_square.left < left)
		bg_square.left++;

	if (bg_square.left > left)
		bg_square.left--;

	if (bg_square.right < right)
		bg_square.right++;

	if (bg_square.right > right)
		bg_square.right--;

	if (bg_square.top < top)
		bg_square.top++;

	if (bg_square.top > top)
		bg_square.top--;

	if (bg_square.bottom < bottom)
		bg_square.bottom++;

	if (bg_square.bottom > bottom)
		bg_square.bottom--;

	left = bg_square.left;
	right = bg_square.right;
	top	= bg_square.top;
	bottom = bg_square.bottom;

	left   += (int)(sin_table( degree*2)     / 2.0);
	right  += (int)(cos_table( degree*2)     / 2.0);
	top	   += (int)(sin_table((degree*2)+20) / 2.0);
	bottom += (int)(cos_table((degree*2)+40) / 2.0);

	//PDraw::screen_fill(left,top,right,bottom,38);

	double mult_y = (bottom - top) / 19.0;
	double mult_x = (right - left) / 19.0;
	double dbl_y = top;
	double dbl_x = left;
	bool dark = true;
	int vari = 0;

	//Draw squares
	for (int y=0;y<19;y++) {
		dbl_x = left;

		for (int x=0;x<19;x++) {
			
			vari = (x / 4) + (y / 3);
			if (dark) vari /= 2;

			PDraw::screen_fill(int(dbl_x),int(dbl_y),int(dbl_x+mult_x),int(dbl_y+mult_y),pvari+vari);
			
			dbl_x += mult_x;
			dark = !dark;

		}

		dbl_y += mult_y;
	}

	//Draw borders
	PDraw::screen_fill(left-1,top-1,right+1,top+2,0);
	PDraw::screen_fill(left-1,top-1,left+2,bottom+1,0);
	PDraw::screen_fill(left-1,bottom-2,right+1,bottom+1,0);
	PDraw::screen_fill(right-2,top-1,right+1,bottom+1,0);

	PDraw::screen_fill(left-1+1,top-1+1,right+1+1,top+2+1,0);
	PDraw::screen_fill(left-1+1,top-1+1,left+2+1,bottom+1+1,0);
	PDraw::screen_fill(left-1+1,bottom-2+1,right+1+1,bottom+1+1,0);
	PDraw::screen_fill(right-2+1,top-1+1,right+1+1,bottom+1+1,0);

	PDraw::screen_fill(left,top,right,top+1,153);
	PDraw::screen_fill(left,top,left+1,bottom,144);
	PDraw::screen_fill(left,bottom-1,right,bottom,138);
	PDraw::screen_fill(right-1,top,right,bottom,138);
}

bool MenuScreen::Draw_BoolBox(int x, int y, bool muuttuja, bool active) {
	
	PDraw::RECT img_src, img_dst = {x , y, 0, 0};

	if(muuttuja) img_src = {504,124,31,31};
	else img_src = {473,124,31,31};

	if(active){
		PDraw::image_cutclip(game_assets,img_src,img_dst);
	} else{
		PDraw::image_cutcliptransparent(game_assets,img_src,img_dst,50, 255);
		return false;
	}

	if (PInput::mouse_x > x && PInput::mouse_x < x+30 && PInput::mouse_y > y && PInput::mouse_y < y+31){
		if (Clicked()){

			Play_MenuSFX(sfx_global.menu_sound, 100);
			key_delay = 20;
			return true;
		}
	}

	return false;
}

void MenuScreen::drawBoolBoxGroup(bool& value, bool&changed, const char* text_true, const char* text_false){
	if (value){
		if (Draw_Menu_Text(text_true,180,my)) {
			value = false;
			changed = true;
		}
	} else{
		if (Draw_Menu_Text(text_false,180,my)) {
			value = true;
			changed = true;
		}
	}
	if (Draw_BoolBox(100, my, value, true)) {
		value = !value;
		changed = true;
	}
	my += 30;
}


int MenuScreen::Draw_BackNext(int x, int y) {
	int val = 45;

	int randx = rand()%3 - rand()%3;
	int randy = rand()%3 - rand()%3;

	bool mouse_on1 = PInput::mouse_x > x && PInput::mouse_x < x + 31
		&& PInput::mouse_y > y && PInput::mouse_y < y + 31
		&& !mouse_hidden;

	bool mouse_on2 = PInput::mouse_x > x+val && PInput::mouse_x < x+val + 31 
		&& PInput::mouse_y > y && PInput::mouse_y < y + 31
		&& !mouse_hidden;

	if (mouse_on1 || chosen_menu_id == selected_menu_id) {
		PDraw::image_cutclip(game_assets,x+randx,y+randy,566,124,566+31,124+31);
		chosen_menu_id = selected_menu_id;
	} else
		PDraw::image_cutclip(game_assets,x,y,566,124,566+31,124+31);

	if (mouse_on2 || chosen_menu_id == selected_menu_id+1) {
		PDraw::image_cutclip(game_assets,x+val+randx,y+randy,535,124,535+31,124+31);
		chosen_menu_id = selected_menu_id+1;
	} else
		PDraw::image_cutclip(game_assets,x+val,y,535,124,535+31,124+31);

	int ret = 0;

	int c = Clicked();
	if ((c == 1 && mouse_on1) || (c > 1 && chosen_menu_id == selected_menu_id)) {
	
		Play_MenuSFX(sfx_global.menu_sound, 100);
		key_delay = 7;
		ret = 1;
	
	} else if ((c == 1 && mouse_on2) || (c > 1 && chosen_menu_id == selected_menu_id + 1)) {
	
		Play_MenuSFX(sfx_global.menu_sound, 100);
		key_delay = 7;
		ret = 2;
	
	}

	selected_menu_id += 2;

	return ret;
}


int MenuScreen::Draw_Radio(int x, int y, int num, int sel) {

	const PDraw::RECT sel_src = {504,124,31,31};
	const PDraw::RECT uns_src = {473,124,31,31};
	
	int val = 35;

	int randx = rand()%3 - rand()%3;
	int randy = rand()%3 - rand()%3;

	bool mouse_on_y = PInput::mouse_y > y && PInput::mouse_y < y + 31 && !mouse_hidden;

	int ret = -1;
	int c = Clicked();

	for (int i = 0; i < num; i++) {

		int xn = x + i * (val + 31);
		int yn = y;

		bool mouse_on = mouse_on_y && PInput::mouse_x > xn && PInput::mouse_x < xn + 31;

		if (mouse_on) {
			chosen_menu_id = selected_menu_id + i;
		}

		if (chosen_menu_id == selected_menu_id + i) {
			xn += randx;
			yn += randy;
		}

		if ((c == 1 && mouse_on) || (c > 1 && chosen_menu_id == selected_menu_id + i)) {

			Play_MenuSFX(sfx_global.menu_sound, 100);
			key_delay = 10;
			sel = i;
			ret = i;

		}
		
		PDraw::RECT img_dst = { xn, yn, 0, 0};
		if (sel == i)
			PDraw::image_cutclip(game_assets,sel_src,img_dst);
		else
			PDraw::image_cutclip(game_assets,uns_src,img_dst);

	}

	selected_menu_id += num;
	return ret;

}

void MenuScreen::Draw_Menu_Main() {

	//TODO Test it!
	//int my = Settings.touchscreen_mode? 260 : 240;//250;
	int my = 240;
	Draw_BGSquare(160, 200, 640-180, 410, 224);

	if (Episode){
		if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_continue),180,my)){
			if (Game)
				next_screen = SCREEN_GAME;
			else
				next_screen = SCREEN_MAP;

		}
		my += 20;
	}

	if(config_txt.save_slots){
		if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_new_game),180,my)){
			strncpy(menu_name, tekstit->Get_Text(PK_txt.player_default_name), 20);
			menu_name[19] = '\0';

			menu_name_index = strlen(menu_name);
			menu_name_last_mark = ' ';
			
			editing_name = true;
			PInput::StartKeyboard();

			menu_nyt = MENU_NAME;
			key_delay = 30;
		}
		my += 20;

		if (Episode){
			if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_save_game),180,my)){
				menu_nyt = MENU_TALLENNA;
			}
			my += 20;
		}

		if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_load_game),180,my)){
			menu_nyt = MENU_LOAD;
		}
		my += 20;	
	}
	else{
		if (Draw_Menu_Text("Play episode",180,my)){
			menu_nyt = MENU_EPISODES;
		}
		my += 20;
	}

	if (Draw_Menu_Text("load language",180,my)){
		menu_nyt = MENU_LANGUAGE;
	}
	my += 20;

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_controls),180,my)){
		menu_nyt = MENU_CONTROLS;
	}
	my += 20;

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_graphics),180,my)){
		menu_nyt = MENU_GRAPHICS;
	}
	my += 20;

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_sounds),180,my)){
		menu_nyt = MENU_SOUNDS;
	}
	my += 20;

	if (Settings.touchscreen_mode && Game) {
		if (Draw_Menu_Text("map",180,my)) {
			next_screen = SCREEN_MAP;

			delete Game;
			Game = nullptr;
		}
		my += 20;
	}
	else{

		#ifdef __ANDROID__

		if (Draw_Menu_Text("Install zip",180,my)){
			Android_InstallZipEpisode();
		}
		my += 20;

		#endif


		if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_exit),180,my)){
			Fade_Quit();
		}
		my += 20;
	}

	
}

void MenuScreen::Draw_Menu_Name() {

	bool mouse_on_text = false;
	size_t nameSize = strlen(menu_name);

	int keyboard_size = 0;
	Draw_BGSquare(90, 160, 640-90, 480-80, 224);

	/*if(Settings.touchscreen_mode)
		keyboard_size = 180;
	else
		keyboard_size = 0;

	if (Settings.touchscreen_mode)
		Draw_BGSquare(90, 20, 640-90, 220, 224);
	else
		Draw_BGSquare(90, 160, 640-90, 480-80, 224);*/
	
	int tx_start = 180;
	int tx_end = tx_start + 15*20;
	int ty_start = 254 - keyboard_size;
	int ty_end = ty_start + 18;

	if (PInput::mouse_x >= tx_start && PInput::mouse_x <= tx_end && PInput::mouse_y >= ty_start && PInput::mouse_y <= ty_end)
		mouse_on_text = true; //Mouse is in text

	if (mouse_on_text && PInput::MouseLeft() && key_delay == 0) {

		PInput::StartKeyboard();
		editing_name = true;
		menu_name_index = (PInput::mouse_x - 180) / 15; //Set text cursor with the mouse
		key_delay = 10;
	
	}

	if (menu_name_index >= 20)
		menu_name_index = 19;

	if (menu_name_index >= nameSize)
		menu_name_index = nameSize;

	if (menu_name_index < 0)
		menu_name_index = 0;

	
	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.playermenu_type_name),tx_start,ty_start - 30);

	PDraw::screen_fill(tx_start-2, ty_start-2, tx_end+4, ty_end+4, 0);
	PDraw::screen_fill(tx_start, ty_start, tx_end, ty_end, mouse_on_text? 54:50);

	if (editing_name) { //Draw text cursor
		int mx = menu_name_index*15 + tx_start + rand() % 2;
		PDraw::screen_fill(mx-2, ty_start, mx+6+3, ty_end+3, 0);
		PDraw::screen_fill(mx-1, ty_start, mx+6, ty_end, 96+16);
		PDraw::screen_fill(mx+4, ty_start, mx+6, ty_end, 96+8);
	}

	WavetextSlow_Draw(menu_name,fontti2,tx_start,ty_start-1);
	PDraw::font_writealpha_s(fontti3,menu_name,tx_start,ty_start-1,15);

	if (editing_name) {

		char in;

		// TO DO
		// allow entering UTF-8 characters
		int key = PInput::ReadKeyboardNav();
		PInput::ReadKeyboardInputOld(&in);

		if (in != '\0') {

			for(uint j = sizeof(menu_name) - 1; j > menu_name_index; j--)
				menu_name[j] = menu_name[j-1];
			
			menu_name[menu_name_index] = in;
			menu_name_index++;
			menu_name[19] = '\0';

		}

		if (PInput::Keydown(PInput::JOY_START) && key_delay == 0 && editing_name) {
			editing_name = false;
			PInput::EndKeyboard();
		}

		if (key == PInput::DEL) {
			for (int c=menu_name_index;c<19;c++)
				menu_name[c] = menu_name[c+1];
			menu_name[19] = '\0';
		}

		if (key == PInput::BACK && menu_name_index != 0) {
			for (int c=menu_name_index-1;c<19;c++)
				menu_name[c] = menu_name[c+1];
			menu_name[19] = '\0';
			if(menu_name[menu_name_index] == '\0') menu_name[menu_name_index-1] = '\0';
			menu_name_index--;
		}

		if (key == PInput::RETURN && menu_name[0] != '\0') {
			editing_name = false;
			PInput::EndKeyboard();

			chosen_menu_id = 1;
		}

		if (key == PInput::LEFT) {
			menu_name_index--;
			key_delay = 8;
		}

		if (key == PInput::RIGHT) {
			menu_name_index++;
			key_delay = 8;
		}

	}


	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.playermenu_continue),tx_start,ty_start + 50)) {

		editing_name = false;
		PInput::EndKeyboard();
		
		menu_nyt = MENU_EPISODES;
		//chosen_menu_id = selected_menu_id = 1;

	}

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.playermenu_clear),tx_start + 180,ty_start + 50)) {
		memset(menu_name,'\0',sizeof(menu_name));
		menu_name_index = 0;
	}

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_exit),tx_start,ty_start + 100)) {
		menu_nyt = MENU_MAIN;
		menu_name_index = 0;
		editing_name = false;
		PInput::EndKeyboard();
	}

}

void MenuScreen::Draw_Menu_Load() {

	int my = 0, vali = 0;
	//char number[32];
	//char jaksoc[8];
	//char ind[8];

	Draw_BGSquare(40, 70, 640-40, 410, 67);

	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.loadgame_title),50,90);
	PDraw::font_write(fontti1,id_code,500,90);
	PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.loadgame_info),50,110);
	my = -20;

	for ( int i = 0; i < SAVE_SLOTS_NUMBER; i++ ) {

		std::ostringstream os;
		
		if (i == 10) {
			
			if(PK2save::saves_slots[i].empty)
				break;
			os<<"bk. ";
			my += 13;

		} else {
			os<<i+1<<". ";
		}
		
		if (PK2save::saves_slots[i].empty){
			os<<"empty";
		}
		else{
			os<<PK2save::saves_slots[i].name;
		}

		std::string number = os.str();
		

		if (Draw_Menu_Text(number.c_str(),100,150+my)) {
			if (!PK2save::saves_slots[i].empty) {
				if (Game) {
					delete Game;
					Game = nullptr;
				}
				if (Episode) {
					delete Episode;
					Episode = nullptr;
				}

				Episode = new EpisodeClass(i);
				next_screen = SCREEN_MAP;
			}
		}

		if (!PK2save::saves_slots[i].empty) {

			vali = 0;
			vali += PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.loadgame_episode),400,150+my);
			vali += PDraw::font_write(fontti1,PK2save::saves_slots[i].episode,400+vali,150+my);
			
			vali = 0;
			if (PK2save::saves_slots[i].next_level != UINT32_MAX) {
				vali += PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.loadgame_level),400+vali,160+my);
				//sprintf(jaksoc, "%u", PK2save::saves_slots[i].next_level);
				vali += PDraw::font_write(fontti1,std::to_string(PK2save::saves_slots[i].next_level),400+vali,160+my);
			} else {
				vali += PDraw::font_write(fontti1,"completed",400+vali,160+my);
			}
		}

		my += 22;
	}

	my += 20;

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_return),180,400))
		menu_nyt = MENU_MAIN;

}

void MenuScreen::Draw_Menu_Save() {

	int my = 0, vali = 0;

	Draw_BGSquare(40, 70, 640-40, 410, 224);

	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.savegame_title),50,90);
	PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.savegame_info),50,110);
	my = -20;

	for (int i = 0; i < SAVE_SLOTS_NUMBER - 1; i++) {
		std::ostringstream os;
		os<<i+1<<". ";

		if (PK2save::saves_slots[i].empty){
			os<<"empty";
		}
		else{
			os<<PK2save::saves_slots[i].name;
		}

		std::string number = os.str();

		if (Draw_Menu_Text(number.c_str(),100,150+my))
			PK2save::Save_Record(i);

		if (!PK2save::saves_slots[i].empty) {

			vali = 0;
			vali += PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.savegame_episode),400,150+my);
			vali += PDraw::font_write(fontti1,PK2save::saves_slots[i].episode,400+vali,150+my);
			
			vali = 0;
			if (PK2save::saves_slots[i].next_level != UINT32_MAX) {
				vali += PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.savegame_level),400+vali,160+my);
				//sprintf(jaksoc, "%u", PK2save::saves_slots[i].next_level);
				vali += PDraw::font_write(fontti1,std::to_string(PK2save::saves_slots[i].next_level),400+vali,160+my);
			} else {
				vali += PDraw::font_write(fontti1,"completed",400+vali,160+my);
			}
		}

		my += 22;
	}

	my += 20;

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_return),180,400))
		menu_nyt = MENU_MAIN;

}

void MenuScreen::Draw_Menu_Graphics() {

	//int mx = 0, my = 150, option;

	int mx = 0, option;
	this->my = 150;
	
	static bool moreOptions = false;
	bool save_settings = false;

	Draw_BGSquare(40, 70, 640-40, 410, 224);

	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.gfx_title),50,90);

	if(moreOptions){
		bool wasFullScreen = Settings.isFullScreen;
		int  oldfps = Settings.fps;

		if (Settings.isFullScreen){
			if (Draw_Menu_Text("fullscreen mode is on",180,my)){
				Settings.isFullScreen = false;
			}
		} else{
			if (Draw_Menu_Text("fullscreen mode is off",180,my)){
				Settings.isFullScreen = true;
			}
		}
		if (Draw_BoolBox(100, my, Settings.isFullScreen, true)) {
			Settings.isFullScreen = !Settings.isFullScreen;
		}
		my += 40;

		//TODO - Fix touch position when screen fit

		mx = 100;
		PDraw::font_write(fontti1, "vsync", mx, my);
		mx += 15 + 51;
		PDraw::font_write(fontti1, "60fps", mx, my);
		mx += 15 + 51;
		PDraw::font_write(fontti1, "85fps", mx, my);
		mx += 15 + 51;
		PDraw::font_write(fontti1, "120fps", mx, my);
		my += 10;

		option = Draw_Radio(100, my, 4, Settings.fps);

		if (option != -1)
			Settings.fps = option;

		my += 31 + 5;

		if (!Settings.touchscreen_mode) {
			mx = 100;
			PDraw::font_write(fontti1, "nearest", mx, my);
			mx += 15 + 51;
			PDraw::font_write(fontti1, "linear", mx, my);
			mx += 15 + 51;

			/*  Temporarily disabled  */
			// PDraw::font_write(fontti1, "crt", mx, my);
			// mx += 15 + 51;
			// PDraw::font_write(fontti1, "hqx", mx, my);
			my += 10;

			option = Draw_Radio(100, my, 2, Settings.shader_type);

			if (option != -1) {
				if (option != Settings.shader_type) {

					int ret = Set_Screen_Mode(option);
					if (ret == 0) {
						Settings.shader_type = option;
						save_settings = true;
					}

				}
			}
		}

		//Add more options here



		if(wasFullScreen != Settings.isFullScreen) {// If fullscreen changes
			save_settings = true;
			PRender::set_fullscreen(Settings.isFullScreen);
		}

		if (Settings.fps != oldfps) {
			int ret = -1;
			save_settings = true;
			if (Settings.fps == SETTINGS_VSYNC)
				ret = Piste::set_fps(-1);
			else if (Settings.fps == SETTINGS_60FPS)
				ret = Piste::set_fps(60);
			else if (Settings.fps == SETTINGS_85FPS)
				ret = Piste::set_fps(85);
			else if (Settings.fps == SETTINGS_120FPS)
				ret = Piste::set_fps(120);
			
			if (ret != 0) {
				PLog::Write(PLog::ERR, "PK2", "FPS mode not supported, changing to 60fps");
				Piste::set_fps(60);
				Settings.fps = SETTINGS_60FPS;
				if (oldfps == SETTINGS_60FPS)
					save_settings = false;
			}	
		}

		if (Draw_Menu_Text("back",100,360)) {
			moreOptions = false;
			chosen_menu_id = 0; //Set menu cursor to 0
		}

	}
	else {
		this->my = 150;

		this->drawBoolBoxGroup(Settings.transparent_text,
							save_settings,
							tekstit->Get_Text(PK_txt.gfx_tmenus_on),
							tekstit->Get_Text(PK_txt.gfx_tmenus_off));

		this->drawBoolBoxGroup(Settings.draw_itembar,
							save_settings,
							tekstit->Get_Text(PK_txt.gfx_items_on),
							tekstit->Get_Text(PK_txt.gfx_items_off));
		
		this->drawBoolBoxGroup(Settings.draw_gui,
							save_settings,
							"In-Game GUI is enabled",
							"In-Game GUI is disabled");

		this->drawBoolBoxGroup(Settings.touchscreen_mode,
							save_settings,
							"Touchscreen mode on",
							"Touchscreen mode off");

		this->drawBoolBoxGroup(Settings.double_speed,
							save_settings,
							tekstit->Get_Text(PK_txt.gfx_speed_double),
							tekstit->Get_Text(PK_txt.gfx_speed_normal));

		//if (!Settings.touchscreen_mode)
			if (Draw_Menu_Text("more",100,360)){
				moreOptions = true;
				chosen_menu_id = 0; //Set menu cursor to 0
			}

	}

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_return),180,400)){
		menu_nyt = MENU_MAIN;
		moreOptions = false;
	}

	if (save_settings)
		Settings_Save();

}

void MenuScreen::Draw_Menu_Sounds() {

	Draw_BGSquare(40, 70, 640-40, 410, 224);

	int my = 0;
	int kdelay = 5;

	u8 sfx_volume = Settings.sfx_max_volume;
	u8 mus_volume = Settings.music_max_volume;

	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.sound_title),50,90);
	my += 20;

	PDraw::screen_fill(404,224+my,404+Settings.sfx_max_volume,244+my,0);
	PDraw::screen_fill(400,220+my,400+Settings.sfx_max_volume,240+my,81);

	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.sound_sfx_volume),180,200+my);
	my += 20;

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.sound_less),180,200+my)) {
		key_delay = kdelay;
		if (Settings.sfx_max_volume > 0)
			Settings.sfx_max_volume -= 5;
	}

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.sound_more),180+8*15,200+my)) {
		key_delay = kdelay;
		if (Settings.sfx_max_volume < 100)
			Settings.sfx_max_volume += 5;
	}

	my += 40;

	PDraw::screen_fill(404,224+my,404+Settings.music_max_volume,244+my,0);
	PDraw::screen_fill(400,220+my,400+Settings.music_max_volume,240+my,112);

	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.sound_music_volume),180,200+my);
	my += 20;

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.sound_less),180,200+my)) {
		key_delay = kdelay;
		if (Settings.music_max_volume > 0)
			Settings.music_max_volume -= 5;
	}

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.sound_more),180+8*15,200+my)) {
		key_delay = kdelay;
		if (Settings.music_max_volume < 100)
			Settings.music_max_volume += 5;
	}
	
	my += 20;

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_return),180,400))
		menu_nyt = MENU_MAIN;
	
	
	if (Settings.sfx_max_volume < 0)
		Settings.sfx_max_volume = 0;

	if (Settings.sfx_max_volume > 100)
		Settings.sfx_max_volume = 100;
	
	if (Settings.music_max_volume < 0)
		Settings.music_max_volume = 0;

	if (Settings.music_max_volume > 100)
		Settings.music_max_volume = 100;

	if (mus_volume != Settings.music_max_volume) {

		if (!Game || !Game->music_stopped)
			PSound::set_musicvolume(Settings.music_max_volume);
		Settings_Save();

	}
	if (sfx_volume != Settings.sfx_max_volume) {
		
		Settings_Save();

	}

}

void MenuScreen::Draw_Menu_Controls() {
	
	int my = 130;
	bool save_settings = false;

	Draw_BGSquare(40, 70, 640-40, 410, 224);

	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.controls_title),50,90);

	if (menu_lue_kontrollit > 0){
		PDraw::screen_fill(299,my-16+menu_lue_kontrollit*20,584,4+my+menu_lue_kontrollit*20,0);
		PDraw::screen_fill(295,my-20+menu_lue_kontrollit*20,580,my+menu_lue_kontrollit*20,50);
	}

	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.controls_moveleft),100,my);my+=20;
	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.controls_moveright),100,my);my+=20;
	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.controls_jump),100,my);my+=20;
	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.controls_duck),100,my);my+=20;
	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.controls_walkslow),100,my);my+=20;
	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.controls_eggattack),100,my);my+=20;
	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.controls_doodleattack),100,my);my+=20;
	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.controls_useitem),100,my);my+=20;

	my = 130;
	PDraw::font_write(fontti2,PInput::KeyName(Input->left),380,my);my+=20;
	PDraw::font_write(fontti2,PInput::KeyName(Input->right),380,my);my+=20;
	PDraw::font_write(fontti2,PInput::KeyName(Input->jump),380,my);my+=20;
	PDraw::font_write(fontti2,PInput::KeyName(Input->down),380,my);my+=20;
	PDraw::font_write(fontti2,PInput::KeyName(Input->walk_slow),380,my);my+=20;
	PDraw::font_write(fontti2,PInput::KeyName(Input->attack1),380,my);my+=20;
	PDraw::font_write(fontti2,PInput::KeyName(Input->attack2),380,my);my+=20;
	PDraw::font_write(fontti2,PInput::KeyName(Input->open_gift),380,my);my+=20;

	/*
	if (PInput::mouse_x > 310 && PInput::mouse_x < 580 && PInput::mouse_y > 130 && PInput::mouse_y < my-20){
		menu_lue_kontrollit = (PInput::mouse_y - 120) / 20;

		if (menu_lue_kontrollit < 0 || menu_lue_kontrollit > 8)
			menu_lue_kontrollit = 0;
		else
			key_delay = 25;


	}*/

	if (menu_lue_kontrollit == 0){
		if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.controls_edit),100,my)) {
			menu_lue_kontrollit = 1;
			chosen_menu_id = 0; //Set menu cursor to 0
		}
	}

	my += 30;

	if (menu_lue_kontrollit == 0){
		if (Input == &Settings.keyboard) {

			if (Draw_Menu_Text("use game controller",100,my)) {
				Settings.using_controller = SET_TRUE;
				Input = &Settings.joystick;
				chosen_menu_id = 0; //Set menu cursor to 0
				save_settings = true;
			}

		} else {

			if (Draw_Menu_Text("use keyboard",100,my)) {
				Settings.using_controller = SET_FALSE;
				Input = &Settings.keyboard;
				chosen_menu_id = 0; //Set menu cursor to 0
				save_settings = true;
			}

		}
	}

	my += 20;

	if (Draw_Menu_Text("get default",100,my)) {

		if (Input == &Settings.keyboard) {
			Input->left      = PInput::LEFT;
			Input->right     = PInput::RIGHT;
			Input->jump      = PInput::UP;
			Input->down      = PInput::DOWN;
			Input->walk_slow = PInput::LALT;
			Input->attack1   = PInput::LCONTROL;
			Input->attack2   = PInput::LSHIFT;
			Input->open_gift = PInput::SPACE;
		}
		else {
			Input->left      = PInput::JOY_LEFT;
			Input->right     = PInput::JOY_RIGHT;
			Input->jump      = PInput::JOY_UP;
			Input->down      = PInput::JOY_DOWN;
			Input->walk_slow = PInput::JOY_Y;
			Input->attack1   = PInput::JOY_A;
			Input->attack2   = PInput::JOY_B;
			Input->open_gift = PInput::JOY_LEFTSHOULDER;
		}

		menu_lue_kontrollit = 0;
		chosen_menu_id = 0;
		save_settings = true;
	}

	my += 20;

	// TODO - Change this
	if (Settings.vibration > 0){
		if (Draw_Menu_Text("turn on vibration",100,my)){
			Settings.vibration = 0;
			PInput::SetVibration(Settings.vibration);
			save_settings = true;
		}
	} else {
		if (Draw_Menu_Text("turn off vibration",100,my)){
			Settings.vibration = 0xFFFF/2;
			PInput::SetVibration(Settings.vibration);
			save_settings = true;
		}
	}
	my += 20;
	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_return),180,my)){
		menu_nyt = MENU_MAIN;
		menu_lue_kontrollit = 0;
		chosen_menu_id = 0;
	}

	u8 k = 0;

	if (key_delay == 0 && menu_lue_kontrollit > 0){
		
		if (Input == &Settings.keyboard)
			k = PInput::GetKeyKeyboard();
		else
			k = PInput::GetKeyController();

		if (k == PInput::ESCAPE || k == PInput::RETURN || k == PInput::JOY_START || k == PInput::JOY_GUIDE
		/*|| PInput::Keydown(PInput::ESCAPE) || PInput::Keydown(PInput::RETURN) || PInput::Keydown(PInput::JOY_START)*/) {
		
			menu_lue_kontrollit = 0;
			chosen_menu_id = 0;
			key_delay = 20;
			save_settings = true;
		
		} else {

			if (k != 0) {
				switch(menu_lue_kontrollit){
					case 1 : Input->left      = k; break;
					case 2 : Input->right     = k; break;
					case 3 : Input->jump      = k; break;
					case 4 : Input->down      = k; break;
					case 5 : Input->walk_slow = k; break;
					case 6 : Input->attack1   = k; break;
					case 7 : Input->attack2   = k; break;
					case 8 : Input->open_gift = k; break;
					default: Play_MenuSFX(sfx_global.moo_sound,100); break;
				}

				key_delay = 20;
				menu_lue_kontrollit++;
			}

			if (menu_lue_kontrollit > 8) {
				menu_lue_kontrollit = 0;
				chosen_menu_id = 0;
				save_settings = true;
			}

		}
	}

	if (save_settings)
		Settings_Save();

}

void MenuScreen::Draw_Menu_Episodes() {
	int my = 0;

	Draw_BGSquare(80, 130, 640-80, 450, 224);

	PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.episodes_choose_episode),50,90);
	my += 80;

	size_t size = episodes.size();
	if (size > 10) {
		
		//char luku[36];
		int vali = 90;
		int x = 50;//500,
		int y = 50;//300;
		
		//vali += PDraw::font_write(fontti1,"page:",x,y+40);
		//sprintf(luku, "%i", episode_page + 1);
		vali += PDraw::font_write(fontti1,std::to_string(episode_page + 1),x+vali,y+20);
		vali += PDraw::font_write(fontti1,"/",x+vali,y+20);
		//sprintf(luku, "%i", int(size/10) + 1);
		vali += PDraw::font_write(fontti1,std::to_string(int(size/10) + 1),x+vali,y+20);

		int nappi = Draw_BackNext(x,y);

		if (nappi == 1 && episode_page > 0)
			episode_page--;

		if (nappi == 2 && (episode_page*10)+10 < size)
			episode_page++;
	}

	for (uint i = episode_page*10; i < episode_page*10 + 10; i++) {
		if (i >= size)
			break;
		
		if (Draw_Menu_Text( episodes[i].name.c_str(), 110, 90+my)) {
			if (Game) {
				delete Game;
				Game = nullptr;
			}
			if (Episode) {
				delete Episode;
				Episode = nullptr;
			}

			if(config_txt.save_slots){
				Episode = new EpisodeClass(menu_name, episodes[i]);
			}
			else{
				Episode = new EpisodeClass(config_txt.player, episodes[i]);
			}

			
			next_screen = SCREEN_MAP;
		}

		if (episodes[i].is_zip)
			PDraw::font_write(fontti1, episodes[i].zipfile, 450, 95+my);
		else
			PDraw::font_write(fontti1, "original game", 450, 95+my);
		
		my += 20;
	}

	/* sivu / kaikki */
	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_return),180,400)){
		menu_nyt = MENU_MAIN;
		my += 20;
	}

	//PDraw::font_write(fontti1,tekstit->Get_Text(PK_txt.episodes_get_more),140,440);

}

void MenuScreen::Draw_Menu_Language() {

	Draw_BGSquare(110, 130, 640-110, 450, 224);

	PDraw::font_write(fontti2,"select a language:",50,100);

	int my = 150;

	uint end = langlist.size();
	if (end > langlistindex + 10)
		end = langlistindex + 10;

	for ( uint i = langlistindex; i < end; i++ ) {

		std::string lang_name = PString::removeSuffix(langlist[i], ".txt");

		if(Draw_Menu_Text(lang_name.c_str(),150,my)) {

			Load_Language(langlist[i]);

			try{
				Settings.language = langlist[i];
				Settings_Save();
			}
			catch(const std::exception& e){
				PLog::Write(PLog::ERR, "PK2 Fonts", e.what());
				Load_Language(Settings.language);
				Load_Fonts(tekstit);
			}
		}
		
		my += 20;
	}

	my = 380;
	if(langlist.size() > 10) {
		int direction = Draw_BackNext(400,my-20);

		if(direction == 1)
			if(langlistindex > 0)
				langlistindex--;
		
		if(direction == 2)
			if(langlistindex < langlist.size() - 10)
				langlistindex++;

	}

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_return),180,400))
		menu_nyt = MENU_MAIN;

}

void MenuScreen::Draw_Menu_Data() {

/*#ifdef __ANDROID__

	int align_left = 80;
	int align_right = 640 - 70;

	Draw_BGSquare(-0, 30, 640+0, 480-30, 100);

	// TODO - data menu
	int my = 80;

	PDraw::font_write(fontti1, id_code, align_right - 7*8, my);
	my += 15;

	PDraw::font_write(fontti2, "data location:", align_left, my);

	if (external_dir) {
		if (Draw_Menu_Text("external",align_right - 15*8, my)) {

			Move_DataPath(Internal_Path);
			external_dir = false;

		}

	} else {
		if (Draw_Menu_Text("internal",align_right - 15*8, my)) {

			if (PUtils::ExternalWriteable()) {
				Move_DataPath((External_Path));
				external_dir = true;
			} else {
				if (SDL_AndroidRequestPermission("android.permission.WRITE_EXTERNAL_STORAGE")) {
					Move_DataPath((External_Path));
					external_dir = true;
				}
			}

		}

	}

	my += 25;
	PDraw::font_write(fontti1, data_path.c_str(), align_right - data_path.size()*8, my);

#endif*/

	if (Draw_Menu_Text(tekstit->Get_Text(PK_txt.mainmenu_return),180,400))
		menu_nyt = MENU_MAIN;

}

void MenuScreen::Draw() {

	PDraw::image_clip(bg_screen);

	selected_menu_id = 1;

	switch (menu_nyt) {
		case MENU_MAIN     : Draw_Menu_Main();     break;
		case MENU_EPISODES : Draw_Menu_Episodes(); break;
		case MENU_GRAPHICS : Draw_Menu_Graphics(); break;
		case MENU_SOUNDS   : Draw_Menu_Sounds();   break;
		case MENU_CONTROLS : Draw_Menu_Controls(); break;
		case MENU_NAME     : Draw_Menu_Name();     break;
		case MENU_LOAD     : Draw_Menu_Load();     break;
		case MENU_TALLENNA : Draw_Menu_Save();     break;
		case MENU_LANGUAGE : Draw_Menu_Language(); break;
		case MENU_DATA     : Draw_Menu_Data();     break;
		default            : Draw_Menu_Main();     break;
	}

	if (!Episode)
		PDraw::font_write(fontti1, PK2_VERSION_STR_MENU, 0, 470);

	if (!mouse_hidden)
		Draw_Cursor(PInput::mouse_x, PInput::mouse_y);
}



void MenuScreen::Init() {

	TouchScreenControls.change(UI_CURSOR);
	
	PDraw::set_offset(640, 480);
	
	langlistindex = 0;

	if (!Episode) {

		std::optional<PFile::Path> menu_path = PFilesystem::FindVanillaAsset("menu.bmp", PFilesystem::GFX_DIR);
		if(!menu_path.has_value()){
			throw std::runtime_error("\"menu.bmp\" not found!");
		}

		PDraw::image_load_with_palette(bg_screen, default_palette, *menu_path, true);
		PDraw::palette_set(default_palette);

		std::optional<PFile::Path> song9_path = PFilesystem::FindVanillaAsset("song09.xm", PFilesystem::MUSIC_DIR);
		if(!song9_path.has_value()){
			throw std::runtime_error("\"song09.xm\" not found!");
		}

		PSound::start_music(*song9_path);
		PSound::set_musicvolume(Settings.music_max_volume);
	
	} else {

		PDraw::create_shadow(bg_screen, 640, 480);

	}

	menu_nyt = MENU_MAIN;

	bg_square.left = 320 - 5;
	bg_square.top = 240 - 5;
	bg_square.right = 320 + 5;
	bg_square.bottom = 240 + 5;

	chosen_menu_id = 1;

}

void MenuScreen::Loop() {
	
	if (key_delay == 0 && menu_lue_kontrollit == 0) {
		if (PInput::Keydown(PInput::UP) || (PInput::Keydown(PInput::LEFT) && !editing_name)
			|| PInput::Keydown(PInput::JOY_UP) || (PInput::Keydown(PInput::JOY_LEFT) && !editing_name)){
			chosen_menu_id--;
			mouse_hidden = true;

			if (chosen_menu_id < 1)
				chosen_menu_id = selected_menu_id-1;

			key_delay = 9;
		}

		if (PInput::Keydown(PInput::DOWN) || (PInput::Keydown(PInput::RIGHT) && !editing_name)
			|| PInput::Keydown(PInput::JOY_DOWN) || (PInput::Keydown(PInput::JOY_RIGHT) && !editing_name)){
			chosen_menu_id++;
			mouse_hidden = true;

			if (chosen_menu_id > selected_menu_id-1)
				chosen_menu_id = 1;

			key_delay = 9;
		}

		static bool wasPressed = false;

		if (!wasPressed && PInput::Keydown(PInput::ESCAPE) && menu_nyt == MENU_MAIN) {
			mouse_hidden = true;
			if(chosen_menu_id == selected_menu_id-1)
				Fade_Quit();
			else
				chosen_menu_id = selected_menu_id-1;
		}

		wasPressed = PInput::Keydown(PInput::ESCAPE);

	}

	if (menu_lue_kontrollit > 0) {
		chosen_menu_id = 0;
	}

	if (menu_nyt != MENU_NAME){ //TODO - Why run this every frame?
		editing_name = false;
		PInput::EndKeyboard();
	}
	int menu_ennen = menu_nyt;

	static int mx, my, mb;
	int cx = PInput::mouse_x, 
	    cy = PInput::mouse_y,
		cb = PInput::MouseLeft() || PInput::MouseRight();

	if (mx != cx || my != cy || mb != cb)
		mouse_hidden = false;

	mx = cx;
	my = cy;
	mb = cb;

	Draw();

	if (menu_nyt != menu_ennen)
		chosen_menu_id = 0;

	degree = 1 + degree % 360;

	if (Settings.double_speed)
		degree = 1 + degree % 360;

}