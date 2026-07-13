//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "screen.hpp"
#include "game/spriteclass.hpp"

#include <string>

class PlayingScreen: public Screen{
public:
    PlayingScreen();
    ~PlayingScreen();

    void Init()override;
    void Loop()override;
    void Draw();

    void onKeyPressed(const PInput::Key& key)override;
private:
    void drawDevStuff();
	void drawQuickSaveToast();
	void showQuickSaveToast(const std::string& text, bool is_error = false);
	void updatePolishAnimations();
	void loadLatestQuickSave();

    void Draw_InGame_DebugInfo();
    void Draw_InGame_DevKeys();
    void Draw_InGame_Lower_Menu();
    void Draw_InGame_UI();

    bool draw_debug_info = false;
    int debug_drawn_sprites = 0;
    int debug_active_sprites = 0;

    bool takingScreenshot = false;
    bool goingToTheMenu = false;

	std::string quickSaveToastText;
	int quickSaveToastTimer = 0;
	bool quickSaveToastError = false;

	int lastEnergy = -1;
	int lastScore = -1;
	int energyPulseTimer = 0;
	int scorePulseTimer = 0;
	bool energyPulseIsDamage = false;
};
