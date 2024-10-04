//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "intro_screen.hpp"

#include "gfx/text.hpp"
#include "gfx/touchscreen.hpp"
#include "game/game.hpp"
#include "language.hpp"
#include "system.hpp"
#include "settings/settings.hpp"
#include "exceptions.hpp"

#include "engine/PDraw.hpp"
#include "engine/PInput.hpp"
#include "engine/PSound.hpp"
#include "engine/PFilesystem.hpp"
#include "engine/PLog.hpp"

#include <stdexcept>

void IntroScreen::Draw(){

	u32 pistelogo_alku  = 300;
	u32 pistelogo_loppu = pistelogo_alku + 500;
	u32 tekijat_alku    = pistelogo_loppu + 80;
	u32 tekijat_loppu   = tekijat_alku + 700;
	u32 testaajat_alku  = tekijat_loppu + 80;
	u32 testaajat_loppu = testaajat_alku + 700;
	u32 kaantaja_alku   = testaajat_loppu + 100;
	u32 kaantaja_loppu  = kaantaja_alku + 300;
	u32 sdl_alku        = kaantaja_loppu + 100;
	u32 sdl_loppu       = sdl_alku + 400;

	PDraw::image_cutclip(bg_screen, 280, 80, 280, 80, 640, 480);

	if (intro_counter / 10 % 50 == 0)
		PDraw::image_cutclip(bg_screen,353, 313, 242, 313, 275, 432);

	if (intro_counter > pistelogo_alku && intro_counter < pistelogo_loppu) {

		//int x = intro_counter - pistelogo_alku - 194;
		int kerroin = 120 / (intro_counter - pistelogo_alku);
		int x = 120 - kerroin;

		if (x > 120)
			x = 120;

		PDraw::image_cutclip(bg_screen,/*120*/x,230, 37, 230, 194, 442);

		CreditsText_Draw(tekstit->Get_Text(PK_txt.intro_presents), fontti1, 230, 400, pistelogo_alku, pistelogo_loppu-20, intro_counter);

	}

	if (intro_counter > tekijat_alku) {
		CreditsText_Draw(tekstit->Get_Text(PK_txt.intro_a_game_by),fontti1, 120, 230, tekijat_alku, tekijat_loppu, intro_counter);
		CreditsText_Draw("janne kivilahti",			fontti1, 120, 250, tekijat_alku+20, tekijat_loppu+20, intro_counter);
		CreditsText_Draw(tekstit->Get_Text(PK_txt.intro_original), fontti1, 120, 275, tekijat_alku+40, tekijat_loppu+40, intro_counter);
		CreditsText_Draw("antti suuronen 1998",		fontti1, 120, 295, tekijat_alku+50, tekijat_loppu+50, intro_counter);
	}

	if (intro_counter > testaajat_alku) {
		CreditsText_Draw(tekstit->Get_Text(PK_txt.intro_tested_by),fontti1, 120, 230, testaajat_alku, testaajat_loppu, intro_counter);
		CreditsText_Draw("antti suuronen",			fontti1, 120, 250, testaajat_alku+10, testaajat_loppu+10, intro_counter);
		CreditsText_Draw("toni hurskainen",			fontti1, 120, 260, testaajat_alku+20, testaajat_loppu+20, intro_counter);
		CreditsText_Draw("juho rytkönen",				fontti1, 120, 270, testaajat_alku+30, testaajat_loppu+30, intro_counter);
		CreditsText_Draw("annukka korja",				fontti1, 120, 280, testaajat_alku+40, testaajat_loppu+40, intro_counter);
		CreditsText_Draw(tekstit->Get_Text(PK_txt.intro_thanks_to),fontti1, 120, 300, testaajat_alku+70, testaajat_loppu+70, intro_counter);
		CreditsText_Draw("oskari raunio",				fontti1, 120, 310, testaajat_alku+70, testaajat_loppu+70, intro_counter);
		CreditsText_Draw("assembly organization",		fontti1, 120, 320, testaajat_alku+70, testaajat_loppu+70, intro_counter);
	}

	if (intro_counter > kaantaja_alku) {
		CreditsText_Draw(tekstit->Get_Text(PK_txt.intro_translation), fontti1, 120, 230, kaantaja_alku, kaantaja_loppu, intro_counter);
		CreditsText_Draw(tekstit->Get_Text(PK_txt.intro_translator),  fontti1, 120, 250, kaantaja_alku+20, kaantaja_loppu+20, intro_counter);
	}

	if (intro_counter > sdl_alku) {
		CreditsText_Draw("Greta Engine by", fontti1, 120, 230, sdl_alku, sdl_loppu, intro_counter);
		CreditsText_Draw("Saturnin The Alien", fontti1, 120, 250, sdl_alku+20, sdl_loppu+20, intro_counter);
		CreditsText_Draw("https://github.com/SaturninTheAlien/pk2_greta", fontti1, 120, 270, sdl_alku+20, sdl_loppu+20, intro_counter);
		CreditsText_Draw("and the piste gamez community", fontti1, 120, 290, sdl_alku+40, sdl_loppu+40, intro_counter);
		CreditsText_Draw("https://discord.gg/yDncSxp", fontti1, 120, 310, sdl_alku+40, sdl_loppu+40, intro_counter);

		CreditsText_Draw("powered by", fontti1, 120, 390, sdl_alku+70, sdl_loppu+70, intro_counter);
		CreditsText_Draw("SDL 2",  fontti1, 120, 410, sdl_alku+70, sdl_loppu+70, intro_counter);
	}
}

void IntroScreen::Init() {

	TouchScreenControls.change(UI_TOUCH_TO_START);
	
	PDraw::set_offset(640, 480);

	std::optional<PFile::Path> intro_bmp = PFilesystem::FindVanillaAsset("intro.bmp", PFilesystem::GFX_DIR);
	if(!intro_bmp.has_value()){
		throw std::runtime_error("\"intro.bmp\" not found!");
	}

	PDraw::image_load_with_palette(bg_screen, default_palette, *intro_bmp, false);
	PDraw::palette_set(default_palette);

	std::optional<PFile::Path> intro_xm = PFilesystem::FindVanillaAsset("intro.xm", PFilesystem::MUSIC_DIR);
	if(!intro_xm.has_value()){
		throw std::runtime_error("\"intro.xm\" not found!");
	}

	if (PSound::start_music(*intro_xm) == -1){
		PLog::Write(PLog::ERR, "PK2", "Can't load intro.xm");
	}

	PSound::set_musicvolume(Settings.music_max_volume);

	intro_counter = 0;

	Fade_in(FADE_FAST);
}

void IntroScreen::Loop() {
	
	Draw();

	degree = 1 + degree % 360;

	intro_counter++;

	if (!closing_intro) {

		if (Clicked() || TouchScreenControls.touch || intro_counter >= 3500){
			closing_intro = true;
			Fade_out(FADE_SLOW);
		}

	} else {

		if (!Is_Fading()){
			next_screen = SCREEN_MENU;
		}

	}
}