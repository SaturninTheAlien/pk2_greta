//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "screen.hpp"

enum MENU {

	MENU_MAIN,
    MENU_SETTINGS,
	MENU_EPISODES,
	MENU_CONTROLS,
	MENU_GRAPHICS,
	MENU_SOUNDS,
	MENU_NAME,
	MENU_LOAD,
	MENU_SAVE,
	MENU_LANGUAGE
};
namespace PDraw{
    class RECT;
}

class MenuScreen: public Screen{
public:
    void Init();
    void Loop();
    void Draw();
private:
    class MenuRect{
    public:
        int left = 0;
        int right = 0;
        int top = 0;
        int bottom = 0;
    }; 

    MenuRect bg_square;
    int menu_nyt = MENU_MAIN;
    int menu_lue_kontrollit = 0;

    uint menu_name_index = 0;
    char menu_name_last_mark = '\0';
    char menu_name[20] = "";
    uint episode_page = 0;
    uint langlistindex = 0;
    bool editing_name = false;

    void Draw_BGSquare(int left, int top, int right, int bottom, u8 pvari);
    bool Draw_BoolBox(int x, int y, bool muuttuja, bool active);
    int  Draw_BackNext(int x, int y);
    int  Draw_Radio(int x, int y, int num, int sel);

    int  my = 0;
    //bool changed = false;
    void drawBoolBoxGroup(bool& value, bool& changed, const std::string& text_true, const std::string& text_false);

    //bool drawButton(int x, int y, const PDraw::RECT& rect);
    void drawLinksMenu();
    bool drawButton(int x, int y, const PDraw::RECT& rect, const std::string& label);

    int btnKeyDelay = 0;
    bool linksMenuExpanded = false;


    void Draw_Menu_Main();
    void Draw_Menu_Name();
    void Draw_Menu_Load();
    void Draw_Menu_Save();
    void Draw_Menu_Graphics();
    void Draw_Menu_Sounds();
    void Draw_Menu_Controls();
    void Draw_Menu_Episodes();
    void Draw_Menu_Language();
    void Draw_Menu_Settings();
};