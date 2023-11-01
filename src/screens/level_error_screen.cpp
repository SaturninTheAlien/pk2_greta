#include "screens.hpp"
#include "system.hpp"
#include "engine/PUtils.hpp"
#include "engine/PInput.hpp"
#include "gui.hpp"

static bool change_to_next_screen = false;

void Screen_LevelError_Init(){
    change_to_next_screen = false;
	Fade_in(FADE_FAST);

}

void Screen_LevelError(){
    if(key_delay==0){
        if(Clicked() || Gui_touch) {
            change_to_next_screen = true;
            Fade_out(FADE_SLOW);
        }
    }
}