//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "score_screen.hpp"

#include "engine/Piste.hpp"

#include "gfx/text.hpp"
#include "language.hpp"
#include "game/game.hpp"
#include "game/gifts.hpp"
#include "gfx/touchscreen.hpp"
#include "episode/episodeclass.hpp"
#include "sfx.hpp"
#include "system.hpp"

#include "settings/settings.hpp"
#include "settings/config_txt.hpp"


#include "episode/save.hpp"
#include "episode/save_legacy.hpp"

#include "engine/PDraw.hpp"
#include "engine/PInput.hpp"
#include "engine/PSound.hpp"
#include "engine/PFilesystem.hpp"

#include <stdexcept>

#include <cstring>

enum {
	COUNT_NOTHING,
	COUNT_BONUS,
	COUNT_TIME,
	COUNT_ENERGY,
	COUNT_GIFTS,
	COUNT_APPLES,
	COUNT_ENDED
};

int ScoreScreen::LevelScore_Compare(int level_id, int score, int apples, int time){
	int ret = 0;
	//Episode->getLevelFilename

	std::string levelFileName = Episode->getLevelFilename(level_id);

	LevelScore * levelScorePtr = Episode->scoresTable.getScoreByLevelName(levelFileName);
	if(levelScorePtr!=nullptr){

		//points
		if(score > levelScorePtr->bestScore){

			this->map_new_record = true;
			levelScorePtr->bestScore = score;
			levelScorePtr->topPlayer = Episode->player_name;

			++ret;
		}

		//apples
		if(apples > levelScorePtr->maxApples){

			levelScorePtr->maxApples = apples;
			++ret;
		}

		//time
		levelScorePtr->hasTime = Game->has_time;
		if(Game->has_time && time < levelScorePtr->bestTime){
			this->map_new_time_record = true;
			levelScorePtr->bestTime = time;
			levelScorePtr->fastestPlayer = Episode->player_name;
			++ret;			
		}


	}
	else{
		LevelScore newScore;
		//level number
		newScore.levelNumber = level_id;

		//level filename
		newScore.levelFileName = levelFileName;

		//points
		this->map_new_record = true;
		newScore.bestScore = score;
		newScore.topPlayer = Episode->player_name;

		//apples
		newScore.maxApples = apples;

		//time
		newScore.hasTime = Game->has_time;		
		if(Game->has_time){
			this->map_new_time_record = true;
			newScore.fastestPlayer = Episode->player_name;
		}

		ret+=3;
		Episode->scoresTable.addScore(newScore);
	}

	return ret;

}
int ScoreScreen::EpisodeScore_Compare(int score){
	int ret = 0;

	if (score > Episode->scoresTable.episodeTopScore) {

		Episode->scoresTable.episodeTopScore = score;
		Episode->scoresTable.episodeTopPlayer = Episode->player_name;
		this->episode_new_record = true;
		ret++;
	}

	return ret;

}

int ScoreScreen::Draw_ScoreCount() {

	//char luku[20];
	int x, y;

	PDraw::image_clip(bg_screen);

	/* BG Effect */
	for (int i = 0; i < 18; i++) {


		int kerroin = (int)(cos_table[(degree+i*3)%180]);

		x = (int)(sin_table(degree+i*10)*2)+kerroin;
		y = (int)(cos_table(degree+i*10)*2);//10 | 360 | 2
		//PDraw::image_clip(game_assets,320+x,240+y,157,46,181,79);
		int kuutio = (int)(sin_table(degree+i*3));
		if (kuutio < 0) kuutio = -kuutio;

		PDraw::screen_fill(320-x,240-y,320-x+kuutio,240-y+kuutio,COLOR_TURQUOISE+8);

	}
	for (int i = 0; i < 18; i++) {

		x = (int)(sin_table(degree+i*10)*3);
		y = (int)(cos_table(degree+i*10)*3);//10 | 360 | 3
		//PDraw::image_clip(game_assets,320+x,240+y,157,46,181,79);
		int kuutio = (int)(sin_table(degree+i*2))+18;
		if (kuutio < 0) kuutio = -kuutio;//0;//
		if (kuutio > 100) kuutio = 100;

		//PDraw::screen_fill(320+x,240+y,320+x+kuutio,240+y+kuutio,COLOR_TURQUOISE+10);
		PDraw::image_cutcliptransparent(game_assets, 247, 1, 25, 25, 320+x, 240+y, kuutio, 32);

	}
	/* --------- */

	int my = 60;

	ShadowedText_Draw(tekstit->Get_Text(PK_txt.score_screen_title), 100, my);
	my += 30;

	ShadowedText_Draw(tekstit->Get_Text(PK_txt.score_screen_level_score), 100, my);

	total_score = bonus_score + time_score + energy_score + gifts_score;
	
	//sprintf(luku, "%i", total_score);
	ShadowedText_Draw(std::to_string(total_score), 400, my);
	my += 70;

	ShadowedText_Draw(tekstit->Get_Text(PK_txt.score_screen_bonus_score), 100, my);
	
	//sprintf(luku, "%i", bonus_score);
	ShadowedText_Draw(std::to_string(bonus_score), 400, my);
	my += 30;

	ShadowedText_Draw(tekstit->Get_Text(PK_txt.score_screen_time_score), 100, my);
	
	//sprintf(luku, "%i", time_score);
	ShadowedText_Draw(std::to_string(time_score), 400, my);
	my += 30;

	ShadowedText_Draw(tekstit->Get_Text(PK_txt.score_screen_energy_score), 100, my);

	//sprintf(luku, "%i", energy_score);
	ShadowedText_Draw(std::to_string(energy_score), 400, my);
	my += 30;

	ShadowedText_Draw(tekstit->Get_Text(PK_txt.score_screen_item_score), 100, my);
	
	//sprintf(luku, "%i", gifts_score);
	ShadowedText_Draw(std::to_string(gifts_score), 400, my);
	my += 40;

	x = 110;
	for (int i = 0; i < MAX_GIFTS; i++) {
		
		if (Gifts_Get(i) != nullptr)	{
			Gifts_Draw(i, x, my);
			x += 38;
		}

	}
	my += 20;

	// Draw apples
	PDraw::set_mask(0, 0, 640, 480);
	if (Game->apples_count > 0) {

		uint i = 0;

		for (; i < apples_counted; i++) {

			if (apples_counted >= Game->apples_count)
				PDraw::image_cutclip(game_assets2, apples_xoffset + i * 32 + rand()%2, my + rand()%2, 61, 379, 87, 406);
			else
				PDraw::image_cutclip(game_assets2, apples_xoffset + i * 32, my, 61, 379, 87, 406);

		}
		for (; i < Game->apples_count; i++)
			PDraw::image_cutcliptransparent(game_assets2, 61, 379, 26, 26, apples_xoffset + i * 32, my, 20, 0);
		
	}
	PDraw::reset_mask();

	if (apples_counted >= 13 && apples_not_counted != 0)
		apples_xoffset -= 32.0 / 10;

	my += 40;

	if (counting_phase == COUNT_ENDED) {
		
		ShadowedText_Draw(tekstit->Get_Text(PK_txt.score_screen_total_score), 100, my);
		
		//sprintf(luku, "%i", Episode->player_score);
		ShadowedText_Draw(std::to_string(Episode->player_score), 400, my);
		my += 25;

		//if (apples_counted >= Game->apples_count && apples_counted > 0) {

		//	PDraw::font_write(fontti2, "You have found all apples!", 100 + rand()%2, my + rand()%2);
		//	my += 25;

		//}

		if (map_new_record) {

			PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.score_screen_new_level_hiscore),100+rand()%2,my+rand()%2);
			my += 25;

		}
		if (map_new_time_record) {

			PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.score_screen_new_level_best_time),100+rand()%2,my+rand()%2);
			my += 25;

		}
		if (episode_new_record) {

			PDraw::font_write(fontti2,tekstit->Get_Text(PK_txt.score_screen_new_episode_hiscore),100+rand()%2,my+rand()%2);
			my += 25;

		}
	}

	int _text_id = test_level?PK_txt.end_the_end:PK_txt.score_screen_continue;
	if (Draw_Menu_Text(tekstit->Get_Text(_text_id),15,430)) {
		going_to_map = true;
		PSound::set_musicvolume(0);
		Fade_out(FADE_SLOW);
	}

	if(!Settings.touchscreen_mode || dev_mode){
		Draw_Cursor(PInput::mouse_x, PInput::mouse_y);
	}
	
	return 0;
}


ScoreScreen::ScoreScreen(){

}

ScoreScreen::~ScoreScreen(){
	PDraw::image_delete(bg_screen);
}

void ScoreScreen::Init() {

	TouchScreenControls.change(UI_CURSOR);
	
	PDraw::set_offset(640, 480);

	std::optional<PFile::Path> menu_path = PFilesystem::FindAsset("menu.bmp", PFilesystem::GFX_DIR, ".png");
	if(!menu_path.has_value()){
		throw std::runtime_error("\"menu.bmp\" not found!");
	}

	PDraw::image_load_with_palette(bg_screen, default_palette, *menu_path, false);
	PDraw::palette_set(default_palette);

	PDraw::create_shadow(bg_screen, 640, 480);

	map_new_record = false;
	map_new_time_record = false;
	episode_new_record = false;

	Game->score += Game->score_increment;

	// Lasketaan pelaajan kokonaispisteet etuk�teen
	u32 temp_score = 0;
	temp_score += Game->score;
	temp_score += Game->timeout / 12; //(Game->timeout / 60) * 5;
	temp_score += Game->playerSprite->energy * 300;
	for (int i = 0; i < MAX_GIFTS; i++)
		if (Gifts_Get(i) != nullptr)
			temp_score += Gifts_Get(i)->score + 500;

	if (!Game->repeating)
		Episode->player_score += temp_score;

	apples_counted = 0;
	apples_not_counted = Game->apples_got;
	apples_xoffset = 100;

	total_score = 0;
	counting_phase = COUNT_NOTHING;
	counting_delay = 30;
	bonus_score = 0;
	time_score = 0;
	energy_score = 0;
	gifts_score = 0;

	if(!test_level){
		/* Check if broken level score and time record */
		int episode_result = EpisodeScore_Compare(Episode->player_score);
		int level_result = 
			LevelScore_Compare(Game->getLevelID(), temp_score, Game->apples_got, Game->level.map_time * TIME_FPS - Game->timeout);
		
		if (episode_result > 0 || level_result > 0) {

			Episode->saveScores();

		}

		if(config_txt.save_slots){
			// Save backup when finishing a level
			PK2save::Save_Record(10);
		}
		else{
			PK2save::SaveModern(Episode);
		}
	}

	PSound::set_musicvolume(Settings.music_max_volume);

	going_to_map = false;
	Fade_in(FADE_FAST);

}

void ScoreScreen::Loop() {

	Draw_ScoreCount();

	degree = 1 + degree % 360;

	if (counting_delay == 0) {
	
		if (bonus_score < Game->score) {

			counting_phase = COUNT_BONUS;
			counting_delay = 0;
			bonus_score += 10;

			if (degree%7==1)
				Play_MenuSFX(Episode->sfx.score_sound, 70);

			if (bonus_score >= Game->score){
				bonus_score = Game->score;
				counting_delay = 50;
			}

		} else if (time_score < Game->timeout / 12) {

			counting_phase = COUNT_TIME;
			counting_delay = 0;
			time_score += 5;

			if (degree%10==1)
				Play_MenuSFX(Episode->sfx.score_sound, 70);

			if (time_score >= Game->timeout / 12) {
				time_score = Game->timeout / 12;
				counting_delay = 50;
			}

		} else if (Game->playerSprite->energy > 0) {

			counting_phase = COUNT_ENERGY;
			counting_delay = 10;
			energy_score+=300;
			Game->playerSprite->energy--;

			Play_MenuSFX(Episode->sfx.score_sound, 70);

		} else if (Gifts_Count() > 0) {
			
			counting_phase = COUNT_GIFTS;
			counting_delay = 30;
			gifts_score += Gifts_Get(0)->score + 500;
			Gifts_Remove(0); 
			Play_MenuSFX(Episode->sfx.jump_sound, 100);

		} else if (apples_not_counted > 0) {

			counting_phase = COUNT_APPLES;
			counting_delay = 10;

			apples_counted++;
			apples_not_counted--;
			Play_MenuSFX(Episode->sfx.apple_sound, 70);

			if (apples_not_counted == 0)
				counting_delay = 20;
		
		} else {
			
			counting_phase = COUNT_ENDED;
		
		}
	}

	if (counting_delay > 0)
		counting_delay--;

	if (Clicked() || going_to_map){

		if(counting_phase == COUNT_ENDED && !going_to_map) {

			going_to_map = true;
			Fade_out(FADE_SLOW);
			PSound::set_musicvolume(0);
			key_delay = 20;

		} else {

			counting_phase = COUNT_ENDED;

			apples_counted += apples_not_counted;
			apples_not_counted = 0;
			if (Game->apples_got > 13)
				apples_xoffset = 100.0 - (Game->apples_got - 13) * 32;

			bonus_score = Game->score;

			time_score = Game->timeout / 12;

			energy_score += Game->playerSprite->energy * 300;
			Game->playerSprite->energy = 0;

			for (int i = 0; i < Gifts_Count(); i++)
				gifts_score += Gifts_Get(i)->score + 500;
			
			Gifts_Clean();

			key_delay = 20;

		}

	}

	if (going_to_map && !Is_Fading()){

		if(test_level){
			Piste::stop();
		}
		else if (Episode->isCompleted() && !Game->repeating && !Episode->no_ending) {
			next_screen = SCREEN_END;

		} else {

			next_screen = SCREEN_MAP;
		
		}

		delete Game;
		Game = nullptr;
	}

	if (Episode->glows)
		if (degree % 4 == 0)
			PDraw::rotate_palette(224,239);

}