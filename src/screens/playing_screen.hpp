//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "screen.hpp"
#include "game/spriteclass.hpp"

class PlayingScreen: public Screen{
public:
    PlayingScreen();
    ~PlayingScreen();

    void Init();
    void Loop();
    void Draw();
private:
    void drawDevStuff();

    void Draw_InGame_DebugInfo();
    void Draw_InGame_DevKeys();
    void Draw_InGame_Gifts();
    void Draw_InGame_Lower_Menu();
    void Draw_InGame_UI();

    bool draw_debug_info = false;
    int debug_drawn_sprites = 0;
    int debug_active_sprites = 0;
};