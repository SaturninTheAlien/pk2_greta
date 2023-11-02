//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "screen.hpp"

class IntroScreen:public Screen{
public:
    void Init();
    void Loop();
    void Draw();
private:
    uint intro_counter = 0;
    bool closing_intro = false;
};