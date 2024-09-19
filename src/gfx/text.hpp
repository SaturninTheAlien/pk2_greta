//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/PLang.hpp"
#include <string>

//Fonts
extern int fontti1;
extern int fontti2;
extern int fontti3;
extern int fontti4;
extern int fontti5;

void Load_Fonts(PLang* lang);

int CreditsText_Draw(const char *text, int font, int x, int y, u32 start, u32 end, u32 time);
int Wavetext_Draw(const char *text, int fontti, int x, int y, char end = '\0');
int WavetextLap_Draw(const char *text, int fontti, int x, int y, float lap, char end = '\0');
int WavetextSlow_Draw(const char *text, int fontti, int x, int y, char end = '\0');
int ShadowedText_Draw(const std::string& text, int x, int y);


void Fadetext_Init();
void Fadetext_New(int font, const std::string&text , u32 x, u32 y, u32 timer);
int Fadetext_Draw();
void Fadetext_Update();