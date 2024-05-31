//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#define BACKGROUND_PARTICLES_NUMBER 300


namespace BG_Particles{

void Init(int weather);
void Draw(int camera_x, int camera_y);
void Update(int camera_x, int camera_y);

}

