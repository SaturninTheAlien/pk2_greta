//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once
#include <string>
#include "engine/PFile.hpp"

class Tileset{
public:
    Tileset()=default;
    ~Tileset(){
        this->clear();
    }
    /**
     * Do not allow copying
    */
    Tileset(const Tileset& ) = delete;
    Tileset& operator= (const Tileset& ) = delete;

    int calculateSplashColor();

    void clear();
    void loadImage(PFile::Path path);

    int getImage()const{
        return this->tiles;
    }

    void animateFire(int button1_timer, int color1, int color2);
    void animateWaterfall();
    void animateWaterSurface();
    void animateWater(int animation_timer);
    void animateRollUp();

    void make254Transparent();
    
    operator bool()const{
        return this->tiles != -1;
    }
private:
    int tiles = -1;
    int water_tiles = -1;
};