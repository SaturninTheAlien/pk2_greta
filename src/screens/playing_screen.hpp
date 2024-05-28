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
    bool Is_Sprite_Visible(const SpriteClass* sprite)const;

    void Draw_InGame_BGSprites();
    void Draw_InGame_FGSprites();

    void Draw_InGame_Sprites();
    void Draw_InGame_DebugInfo();
    void Draw_InGame_DevKeys();
    void Draw_InGame_Gifts();
    void Draw_InGame_Lower_Menu();
    void Draw_InGame_UI();
    //void Update_Camera();

    bool draw_debug_info = false;
    int debug_drawn_sprites = 0;
    int debug_active_sprites = 0;
};