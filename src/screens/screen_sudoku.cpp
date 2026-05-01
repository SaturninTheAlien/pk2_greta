#include "screen_sudoku.hpp"
#include "engine/PDraw.hpp"
#include "engine/PFilesystem.hpp"
#include "engine/PSound.hpp"
#include "engine/PLog.hpp"
#include "engine/PInput.hpp"
#include "system.hpp"
#include "game/game.hpp"
#include "keyboard_navigation.hpp"
#include "episode/episodeclass.hpp"
#include "gfx/touchscreen.hpp"
#include "gfx/text.hpp"
#include "settings/settings.hpp"

#include <sstream>

#include <iostream>


static void mWriteCentered(int font, const std::string& msg, int y){

    auto[width, _] = PDraw::font_get_text_size(font, msg);
    PDraw::font_write_line( font, msg, (screen_width - width)/2, y);

}

void SudokuScreen::Init(){

    TouchScreenControls.change(UI_CURSOR);
    this->mouse_hidden = true;

    //TouchScreenControls.change(UI_CURSOR);
    PDraw::create_shadow(bg_screen, 640, 480);

    std::optional<PFile::Path> music_path = PFilesystem::FindAsset("res/music/riddles.mp3", PFilesystem::MUSIC_DIR);
    if(music_path.has_value()){
        if (PSound::start_music(*music_path) == -1){
            PLog::Write(PLog::ERR, "PK2", "Can't start the music");				
        }			
    }
    else{
        PLog::Write(PLog::ERR, "PK2", "Can't load any  riddles.mp3");
        PSound::stop_music();
    }

    this->sudoku.loadDefault();
    this->showingError = false;
    this->finished = false;
    this->closing = false;
}

void SudokuScreen::Loop(){

    PK2gui::KeyNav::Nav nav = PK2gui::KeyNav::readKeyNav();
    switch (nav)
    {
    case PK2gui::KeyNav::UP:{
        mouse_hidden = true;
        this->selectedY--;
        if(this->selectedY<0){
            this->selectedY = 8;
        }
    }
    break;
    case PK2gui::KeyNav::LEFT:{
        mouse_hidden = true;
        this->selectedX--;
        if(this->selectedX<0){
            this->selectedX = 8;
        }
    }
    break;
    case PK2gui::KeyNav::RIGHT:{
        mouse_hidden = true;
        this->selectedX++;
        if(this->selectedX>8){
            this->selectedX = 0;
        }
    }
    break;

    case PK2gui::KeyNav::DOWN:{
        mouse_hidden = true;
        this->selectedY++;
        if(this->selectedY>8){
            this->selectedY = 0;
        }
    }
    break;
    default:
    break;
    }

    this->draw();

    degree = 1 + degree % 360;

	if (Settings.double_speed)
		degree = 1 + degree % 360;
}


void SudokuScreen::onKeyPressed(const PInput::Key& key){

    if(this->finished){
        if(!this->closing){
            this->closing = true;
            if(Game==nullptr){
                fadeQuit();
            } else {
                Fade_out(FADE_NORMAL);
                next_screen = SCREEN_GAME;
                Game->trollingActivated = false;
                Game->playerSprite->level_sector->startMusic();
            }
        }
    }
    else if (key == PInput::Key::ESCAPE || key == PInput::Key::JOY_START) {
		if(test_level)
			fadeQuit();
	}
    else if (key == PInput::Key::JOY_X || key == PInput::Key::JOY_STICK_LEFT || key == PInput::Key(SDL_SCANCODE_KP_PLUS)) {
        this->cycleCellValue(1);        
    }
    else if (key == PInput::Key::JOY_Y || key == PInput::Key::JOY_STICK_RIGHT || key == PInput::Key(SDL_SCANCODE_KP_MINUS)) {
        this->cycleCellValue(-1);
    }
    else if (key == PInput::Key::JOY_A){
        this->finish();
    }
    else {
        std::optional<int> number = key.getNumericValue();
        if(number.has_value()){
            this->setCellValue(number.value());
        }
        else {
            PK2gui::KeyNav::injectKey(key);
            Screen::onKeyPressed(key);
        }
    }
}


void SudokuScreen::setCellValue(int value){
    if(this->finished)return;

    const trolled::Cell& current = this->sudoku.get(this->selectedX, this->selectedY);
    if(!current.locked){
        this->sudoku.set(this->selectedX, this->selectedY, trolled::Cell(value, false));
    }
    else if(Episode!=nullptr){
        Play_MenuSFX(Episode->sfx.moo_sound,100);
    }
}
void SudokuScreen::cycleCellValue(int dv){
    if(this->finished)return;

    const trolled::Cell& current = this->sudoku.get(this->selectedX, this->selectedY);
    if(!current.locked){

        int nextValue = current.value + dv;
        if(nextValue > 9){
            nextValue = 1;
        } else if(nextValue < 1){
            nextValue = 9;
        }

        this->sudoku.set(this->selectedX, this->selectedY, trolled::Cell(nextValue, false));
    }
    else if(Episode!=nullptr){
        Play_MenuSFX(Episode->sfx.moo_sound,100);
    }
}




void SudokuScreen::draw(){

    PDraw::image_clip(bg_screen);

    int sudX = screen_width/2 - 150;

    int sudY = screen_height/2 - 135;

    this->drawSudoku(sudX, sudY);

    mWriteCentered(fontti2, "Trolling sudoku!", sudY - 60);
    if(finished){
        if(this->showingError){
            mWriteCentered(fontti1, "Press any key to continue", sudY - 35);
            mWriteCentered(fontti2, "Oops! You made a mistake!", sudY + 310);           
        }
    }
    else{

        if(this->mouse_hidden){
            mWriteCentered(fontti1, "Keyboard: ENTER: finish", sudY - 35);
            mWriteCentered(fontti1, "Gamepad: X/Y: numbers  A: finish", sudY - 20);
        }

        if (drawMenuTextS("Done!",sudX + 150, sudY + 310) || this->enterPressed){
            this->finish();
        }
    }


    this->drawMouseCursor();
}


void SudokuScreen::finish(){
    if(finished)return;

    this->finished = true;
    bool sudokuSuccess = this->sudoku.checkCorrectness(this->sudokuError);
    this->showingError = !sudokuSuccess;


    if(Episode!=nullptr && Game!=nullptr){

        Game->trolled = !sudokuSuccess;
        Game->trollingActivated = false;

        if(sudokuSuccess){
            Play_MenuSFX(Episode->sfx.doodle_sound,100);
            Fade_out(FADE_NORMAL);
            this->closing = true;
            next_screen = SCREEN_GAME;
            Game->playerSprite->level_sector->startMusic();

        } else {
            Play_MenuSFX(Episode->sfx.moo_sound,100);
        }
        
    }
}


void SudokuScreen::drawSudoku(int startX, int startY){

    int x = startX + 3;
    int y = startY + 3;

    for(int cellY=0;cellY<9;++cellY){
        for(int cellX=0;cellX<9;++cellX){
            this->drawSudokuCell(x, y, cellX, cellY);
            x += 32;

            if(cellX%3==2){
                x += 3;
            }

        }
        x = startX + 3;
        y += 32;

        if(cellY%3==2){
            y += 3;
        }
    }

    u8 frame_color = (u8)75;
    for(int i=0;i<=3;++i){
        int t = 99*i;
        PDraw::screen_fill(startX + t, startY, startX + t + 3, startY + 300, frame_color);
        PDraw::screen_fill(startX, startY + t, startX + 300, startY + t + 3, frame_color);
    }
}

void SudokuScreen::drawSudokuCell(int posX, int posY, int cellX, int cellY){

    const trolled::Cell& cell = this->sudoku.get(cellX, cellY);

    int x = cell.value >= 1 && cell.value <= 9 ? 594 - 35 * cell.value : 244;
    int y = cell.locked ? 196 : 231;

    PDraw::image_cutclip(global_gfx_texture,posX,posY, x, y, x+32, y+ 32);

    if(this->finished){

        if(this->showingError && cellX == this->sudokuError.x && cellY == this->sudokuError.y){
            PDraw::image_cutcliptransparent(global_gfx_texture, x, y, 32, 32, posX,posY, 100, 64);
        }
    }
    else{

        if(!mouse_hidden){
            const Point2D& mousePos = PInput::InputSystem::instance().getMousePos();
            if(mousePos.x > posX + 1 && mousePos.x < posX + 30
             && mousePos.y > posY + 1 && mousePos.y < posY + 30){

                this->selectedX = cellX;
                this->selectedY = cellY;
            }
        }


        bool selected = this->selectedX==cellX&&this->selectedY==cellY;
        if(selected){
            PDraw::image_cutcliptransparent(global_gfx_texture, x, y, 32, 32, posX,posY, 100, 128);
        }
    }
}