//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "system.hpp"

#include "engine/PLog.hpp"
#include "engine/PUtils.hpp"
#include "engine/PInput.hpp"
#include "engine/PDraw.hpp"
#include "engine/PRender.hpp"
#include "settings.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

int screen_width  = 800;
int screen_height = 480;

bool audio_multi_thread = true;
int audio_buffer_size = 1024;

#ifdef __ANDROID__

bool external_dir;

std::string External_Path;
std::string Internal_Path;

#endif

char id_code[8] = "";

std::string data_path;

int game_assets = -1;
int game_assets2 = -1;
int bg_screen = -1;

int key_delay = 0;
bool mouse_hidden = false;

double cos_table[360];
double sin_table[360];
int degree = 0;
int degree_temp = 0;

bool test_level = false;
bool dev_mode = false;

bool show_fps = false;
bool speedrun_mode = false;

static float alpha = 1;
static float fade_speed = 0;

static const float thunder_sheet[] = {
	1.00,
	2.00,
	5.00,
	6.00,
	7.00,
	4.00,
	7.00,
	4.00,
	2.00,
	1.00,
	7.00,
	5.00,
	2.00,
	1.00,
};
static const float thunder_size = sizeof(thunder_sheet) / sizeof(float);

static int thunder_index = thunder_size;

void Fade_out(float speed){
    alpha = 1;
    fade_speed = -speed;
}
void Fade_in(float speed){
    alpha = 0;
    fade_speed = speed;
}
void StartLightningEffect() {
	thunder_index = 0;
}

bool Is_Fading() {
  if (alpha > 0 && fade_speed < 0)
    return true;

  if (alpha < 100 && fade_speed > 0)
    return true;

  return false;
}

float Screen_Alpha() {

	return alpha;

}

void Update_Colors() {

	if (Is_Fading()) {

        alpha += fade_speed;
        if(alpha < 0) alpha = 0;
        if(alpha > 1) alpha = 1;
    
    }

	if (thunder_index < thunder_size)
		thunder_index++;

	float thunder = (thunder_index == thunder_size) ? 1 : thunder_sheet[thunder_index];

    PDraw::set_rgb(alpha * thunder, alpha * thunder, alpha * thunder);

}

void Id_To_String(u32 id, char* string, std::size_t n) {

	if (!string) return;

	snprintf(string, n, "_%06x", id & 0xFFFFFF);

}

void Calculate_SinCos(){

	for ( int i = 0; i < 360; i++ ) {
	
		cos_table[i] = cos(M_PI*2*i/180) * 33;
		sin_table[i] = sin(M_PI*2*i/180) * 33;
	
	}

}

int Clicked() {

	if (key_delay) return 0;

	if (PInput::MouseLeft())
		return 1;

	if (PInput::Keydown(PInput::SPACE) || PInput::Keydown(PInput::RETURN))
		return 2;

	if (PInput::Keydown(PInput::JOY_A) || PInput::Keydown(PInput::JOY_START))
		return 3;
	
	return 0;

}

void Draw_Cursor(int x, int y) {

	PDraw::image_cutclip(game_assets,x,y,621,461,640,480);
	
}

void Prepare_DataPath() {

	PFile::CreateDirectory(data_path);
	PFile::CreateDirectory(data_path + "scores" PE_SEP);
	PFile::CreateDirectory(data_path + "mapstore" PE_SEP);
}

//TODO - Receive Episode, organize this
bool FindAsset(PFile::Path* path, const char* default_dir) {

	const std::string filename = path->GetFileName();

	if(path->Find()){
		return true;
	}

	path->SetSubpath(default_dir);

	//PLog::Write(PLog::INFO, "PK2", "Trying %s", path->c_str());

	if(path->Find()){
		return true;
	}

	path->SetPath(default_dir);
	path->SetFile(filename);

	//PLog::Write(PLog::INFO, "PK2", "Trying %s", path->c_str());

	if(path->Find()){
		return true;
	}

	if(path->Is_Zip()){

		*path = PFile::Path(default_dir + filename);
		//PLog::Write(PLog::INFO, "PK2", "Trying %s", path->c_str());
		if (path->Find()) {
			return true;

		}

	}

	PLog::Write(PLog::INFO, "PK2", "Can't find %s", path->c_str());
	return false;
}

int Set_Screen_Size(int w, int h) {

	PDraw::set_buffer_size(w, h);
	PRender::set_window_size(w, h);
	screen_width = w;
	screen_height = h;

	return 0;

}

int Set_Screen_Mode(int mode) {

	int err = -1;

	if (mode == SETTINGS_MODE_NEAREST) {
		err = PRender::set_shader(PRender::SHADER_NEAREST);
	} else if (mode == SETTINGS_MODE_LINEAR) {
		err = PRender::set_shader(PRender::SHADER_LINEAR);
	} else if (mode == SETTINGS_MODE_CRT) {
		err = PRender::set_shader(PRender::SHADER_CRT);
	} else if (mode == SETTINGS_MODE_HQX) {
		err = PRender::set_shader(PRender::SHADER_HQX);
	}

	if (err == 1)
		return -1;

	if (mode == SETTINGS_MODE_CRT) {
		Set_Screen_Size(640, 480);
		PRender::set_screen_fill(false);
	} else {
		Set_Screen_Size(800, 480);
		PRender::set_screen_fill(true);
	}

	return 0;
	
}