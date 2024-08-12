//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include <iostream>
#include "touchscreen.hpp"

#include "system.hpp"
#include "settings/settings.hpp"
#include "game/gifts.hpp"
#include "game/game.hpp"
#include "engine/PRender.hpp"
#include "engine/PInput.hpp"

#include <cmath>


PK2TouchScreenButton::PK2TouchScreenButton(PRender::RECT src, PRender::RECT dst, u8 alpha):
src(src), dst(dst), alpha(alpha){

}

void PK2TouchScreenButton::draw()const{
	if(this->active) {
		if (this->src.w == 0 || this->src.h == 0)return;

		PRender::FRECT dst;
		dst.x = float(this->dst.x) / 1920;
		dst.y = float(this->dst.y) / 1080;
		dst.w = float(this->dst.w) / 1920;
		dst.h = float(this->dst.h) / 1080;

		PRender::FRECT src;
		src.x = float(this->src.x) / 1024;
		src.y = float(this->src.y) / 1024;
		src.w = float(this->src.w) / 1024;
		src.h = float(this->src.h) / 1024;

		PRender::render_ui(src, dst, float(this->alpha) / 255);
	}
}

const static int PadBt_x = 315;
const static int PadBt_y = 675;

const static int Alpha = 110;

const static int PadBt_a = 20;
const static int PadBt_r = 890;
const static int PadBt_b = PadBt_y + sqrt(PadBt_r*PadBt_r - PadBt_x*PadBt_x);

void PK2TouchScreenControls::set(bool flag){

	gui_padbg.active = flag;
	gui_padbt.active = flag;

	gui_up.active = flag;
	gui_down.active = flag;
	gui_doodle.active = flag;
	gui_egg.active = flag;
	gui_gift.active = flag;

	gui_menu.active = flag;
	gui_tab.active = flag;

}

void PK2TouchScreenControls::change(int ui_mode) {

	UI_mode = ui_mode;

	switch(ui_mode){
		case UI_TOUCH_TO_START:
			gui_touch.active = true;
			this->set(false);
		break;
		case UI_CURSOR:
			gui_touch.active = false;
			this->set(false);
		break;
		case UI_GAME_BUTTONS:
			gui_touch.active = true;
			this->set(true);
		break;
	}

}

void PK2TouchScreenControls::load() {
	if(loaded)return;

	int button_w = 230, button_h = 220;

	int w = button_w * 0.8;
	int h = button_h * 0.8;

	PFile::Path path("gfx" PE_SEP "touchscreen.png");
	PRender::load_ui_texture(path);

	PRender::RECT src, dst;

	src.x = 0, src.y = 0, src.w = 0,    src.h = 0;
	dst.x = 0, dst.y = 0, dst.w = 1920, dst.h = 1080;
	gui_touch = PK2TouchScreenButton(src, dst, Alpha);


	src.x = 0, src.y = 0,   src.w = 0,   src.h = 0;
	dst.x = 0, dst.y = 930, dst.w = 530, dst.h = 150;
	gui_tab = PK2TouchScreenButton(src, dst, Alpha);

	src.x = button_w * 1, src.y = button_h * 1, src.w = button_w, src.h = button_h;
	dst.x = 50, dst.y = 130, dst.w = w, dst.h = h;
	gui_menu = PK2TouchScreenButton(src, dst, Alpha);
	
	src.x = button_w * 0, src.y = button_h * 3, src.w = 640, src.h = 360;
	dst.x = 90, dst.y = 610, dst.w = 577, dst.h = 353;
	gui_padbg = PK2TouchScreenButton(src, dst, Alpha);

	src.x = button_w * 0, src.y = button_h * 2, src.w = 175, src.h = 175;
	dst.x = PadBt_x, dst.y = PadBt_y, dst.w = 152, dst.h = 156;
	gui_padbt = PK2TouchScreenButton(src, dst, Alpha);

	int y = 650;
	const int dy = 200;

	src.x = button_w * 2, src.y = button_h * 0, src.w = button_w, src.h = button_h;
	dst.x = 1630, dst.y = y, dst.w = w, dst.h = h;
	gui_up = PK2TouchScreenButton(src, dst, Alpha);

	y -= dy;
	src.x = button_w * 0, src.y = button_h * 0, src.w = button_w, src.h = button_h;
	dst.x = 1630, dst.y = y, dst.w = w, dst.h = h;
	gui_doodle = PK2TouchScreenButton(src, dst, Alpha);

	y -= dy;
	src.x = button_w * 2, src.y = button_h * 1, src.w = button_w, src.h = button_h;
	dst.x = 1630, dst.y = y, dst.w = w, dst.h = h;
	gui_gift = PK2TouchScreenButton(src, dst, Alpha);

	y = 720;
	src.x = button_w * 1, src.y = button_h * 0, src.w = button_w, src.h = button_h;
	dst.x = 1410, dst.y = y, dst.w = w, dst.h = h;
	gui_down = PK2TouchScreenButton(src, dst, Alpha);

	y -= dy;
	src.x = button_w * 0, src.y = button_h * 1, src.w = button_w, src.h = button_h;
	dst.x = 1410, dst.y = y, dst.w = w, dst.h = h;
	gui_egg = PK2TouchScreenButton(src, dst, Alpha);

}

bool PK2TouchScreenControls::readGui(const PK2TouchScreenButton& gui) {
	if (!gui.active)
		return false;

	for (PInput::touch_t touch : PInput::touchlist) {

		float touch_x = touch.pos_x * 1920;
		float touch_y = touch.pos_y * 1080;

		if(touch_x > gui.dst.x && touch_x < gui.dst.x + gui.dst.w)
		if(touch_y > gui.dst.y && touch_y < gui.dst.y + gui.dst.h)
		if(!pad_grab || pad_id != touch.id)
			return true;

	}

	return false;

}

// Hold pad on digital pad mode
float PK2TouchScreenControls::holdPad(float pos_x, int* button) {

	//pos_x varies from 0 to 1
	//x varies from 0 to 3.2
	float x = 3.2 * pos_x;

	float a =  3.98;
	float b = 25.00;

	//Make the middle poits more close
	if (x < 1.00) {
		x = x + a*(x-0.00)*(x-0.50)*(x-1.00);
		*button = 0;
	
	} else if (x < 1.40) {
		x = x + b*(x-1.00)*(x-1.20)*(x-1.40);
		*button = 1;
	
	} else if (x < 1.80) {
		x = x + b*(x-1.40)*(x-1.60)*(x-1.80);
		*button = 2;
	
	} else if (x < 2.20) {
		x = x + b*(x-1.80)*(x-2.00)*(x-2.20);
		*button = 3;
	
	} else if (x < 3.2) {
		x = x + a*(x-2.20)*(x-2.70)*(x-3.20);
		*button = 4;
	
	} else {
		x = 3.2;
		*button = 4;
	}

	if (x < 0.00)
		x = 0.00;
	if (x > 3.20)
		x = 3.20;
	
	return x / 3.20;

}


int PK2TouchScreenControls::getPad() {

	int button = 2;

	if (!pad_grab) {
		for (PInput::touch_t touch : PInput::touchlist) {
			
			float touch_x = touch.pos_x * 1920;
			float touch_y = touch.pos_y * 1080;

			if(touch_x > gui_padbg.dst.x && touch_x < gui_padbg.dst.x + gui_padbg.dst.w &&
				touch_y > gui_padbg.dst.y && touch_y < gui_padbg.dst.y + gui_padbg.dst.h) {

				pad_grab = true;
				pad_id = touch.id;
				break;

			}

		}
	
	}
	
	if (pad_grab) {

		PInput::touch_t* last_touch = nullptr;
		for (PInput::touch_t& touch : PInput::touchlist) {

			if(touch.id == pad_id)
				last_touch = &touch;
			
		}

		if (!last_touch) {

			pad_grab = false;
			gui_padbt.dst.x = PadBt_x;
			gui_padbt.dst.y = PadBt_y;

		} else {

			float touch = (last_touch->pos_x * 1920 - gui_padbg.dst.x) / gui_padbg.dst.w;
			float hold_touch = this->holdPad(touch, &button);

			gui_padbt.dst.x = gui_padbg.dst.x + hold_touch * gui_padbg.dst.w;
			
			gui_padbt.dst.x -= gui_padbt.dst.w / 2;

			if (gui_padbt.dst.x < gui_padbg.dst.x)
				gui_padbt.dst.x = gui_padbg.dst.x;

			if (gui_padbt.dst.x > gui_padbg.dst.x + gui_padbg.dst.w - gui_padbt.dst.w)
				gui_padbt.dst.x = gui_padbg.dst.x + gui_padbg.dst.w - gui_padbt.dst.w;
			
			int x = gui_padbt.dst.x - PadBt_a;
			gui_padbt.dst.y = PadBt_b - sqrt(PadBt_r*PadBt_r - x*x);

			/*if (button == 0) {
				ret = 100;
			} else if (button == 1) {
				ret = 20;
			} else if (button == 2) {
				ret = 0;
			} else if (button == 3) {
				ret = -20;
			} else if (button == 4) {
				ret = -100;
			}*/

			/*ret = gui_padbg.x + (gui_padbg.w - gui_padbt.w) / 2;
			ret -= gui_padbt.x;
			ret /= float(gui_padbg.w - gui_padbt.w) / 2;
			ret *= 100;*/

		}

	}

	return button;

}


void PK2TouchScreenControls::reset(){
	doodle_alpha = 0; egg_alpha = 0; gift_alpha = 0; 
	doodle_active = false; egg_active = false; gift_active = false;
}

void PK2TouchScreenControls::update() {

	if(!this->loaded){
		this->load();
	}

	const int DELTA = 5;

	this->pad_button = 2;

	this->up     = false;
	this->down   = false;
	this->egg    = false;
	this->doodle = false;
	this->gift   = false;

	this->menu  = false;
	this->touch = false;
	this->tab   = false;

	if (UI_mode == UI_GAME_BUTTONS) {

		float alpha = Screen_Alpha();

		{
			SpriteClass* Player_Sprite = Game->playerSprite;

			doodle_active = Player_Sprite->ammo2 != nullptr;
			egg_active = Player_Sprite->ammo1 != nullptr;
			gift_active = Gifts_Count() > 0;

			if (doodle_active) {
				doodle_alpha += DELTA;
				if (doodle_alpha > Alpha) doodle_alpha = Alpha;
			} else {
				doodle_alpha -= DELTA;
				if (doodle_alpha < 0) doodle_alpha = 0;
			}

			if (egg_active) {
				egg_alpha += DELTA;
				if (egg_alpha > Alpha) egg_alpha = Alpha;
			} else {
				egg_alpha -= DELTA;
				if (egg_alpha < 0) egg_alpha = 0;
			}

			if (gift_active) {
				gift_alpha += DELTA;
				if (gift_alpha > Alpha) gift_alpha = Alpha;
			} else {
				gift_alpha -= DELTA;
				if (gift_alpha < 0) gift_alpha = 0;
			}

			gui_doodle.alpha = doodle_alpha * alpha;
			gui_egg.alpha = egg_alpha * alpha;
			gui_gift.alpha = gift_alpha * alpha;

			gui_doodle.active = doodle_alpha != 0;
			gui_egg.active = egg_alpha != 0;
			gui_gift.active = gift_alpha != 0;
		}

		gui_up.alpha = Alpha * alpha;
		gui_down.alpha = Alpha * alpha;
		gui_padbg.alpha = Alpha * alpha;
		gui_padbt.alpha = Alpha * alpha;
		gui_menu.alpha = Alpha * alpha;

		this->pad_button = this->getPad();

		this->up = this->readGui(gui_up);
		this->down = this->readGui(gui_down);
		this->egg = this->readGui(gui_egg);
		this->doodle = this->readGui(gui_doodle);
		this->gift = this->readGui(gui_gift);

		this->menu = this->readGui(gui_menu);
		this->tab = this->readGui(gui_tab);

	} else if (UI_mode == UI_TOUCH_TO_START) {

		this->touch = this->readGui(gui_touch);
	
	}

	this->any = egg || doodle || gift || up || down || menu || tab;

	this->gui_doodle.draw();
	this->gui_down.draw();
	this->gui_egg.draw();
	this->gui_gift.draw();
	this->gui_menu.draw();
	this->gui_padbg.draw();
	this->gui_padbt.draw();
	this->gui_tab.draw();
	this->gui_touch.draw();
	this->gui_up.draw();
}

PK2TouchScreenControls TouchScreenControls;