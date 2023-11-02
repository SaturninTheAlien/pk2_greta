//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "screen.hpp"

class MapScreen: public Screen{
public:
    MapScreen();

    void Init();
    void Loop();
    void Draw();
private:
    void Play_Music();

    bool going_to_game = false;
    int PK_Draw_Map_Button(int x, int y, int type);
};