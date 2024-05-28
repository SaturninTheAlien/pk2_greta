//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/PFile.hpp"


enum {

    BACKGROUND_STATIC,
    BACKGROUND_PARALLAX_VERT,
    BACKGROUND_PARALLAX_HORI,
    BACKGROUND_PARALLAX_VERT_AND_HORI

};

class Background{
public:
    ~Background(){
        this->clear();
    }

    int scrolling = BACKGROUND_STATIC;

    void load(PFile::Path path);
    void clear();
    void draw(int camera_x, int camera_y);
private:
    int picture = -1;
};