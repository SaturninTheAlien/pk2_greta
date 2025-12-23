#include "screen.hpp"
#include "engine/Piste.hpp"
#include "sfx.hpp"
#include "system.hpp"
#include "gfx/text.hpp"
#include "language.hpp"

bool Screen::closing_game = false;
int Screen::next_screen = 0;

void Screen::fadeQuit() {

	if(!closing_game) Fade_out(FADE_FAST);
	closing_game = true;
	PSound::set_musicvolume(0);
	
}

bool Screen::drawMenuText(int id, int x, int y){
	return drawMenuTextS(tekstit->Get_Text(id), x, y);
}

bool Screen::drawMenuTextS(const std::string& text, int x, int y) {

	const int TEXT_H = 20; 

	int length = text.size() * 15;

	bool mouse_on = PInput::mouse_x > x && PInput::mouse_x < x + length 
		&& PInput::mouse_y > y && PInput::mouse_y < y + TEXT_H
		&& !mouse_hidden;

	if ( mouse_on ) {

		Wavetext_Draw(text.c_str(), fontti3, x, y);//

		int c = Clicked();
		if ( (c == 1 && mouse_on) || (c > 1) ) {

			Play_MenuSFX(sfx_global.menu_sound, 100);
			key_delay = 20;	
			return true;

		}
	} else {
		WavetextSlow_Draw(text.c_str(), fontti2, x, y);	
	}
	return false;
}