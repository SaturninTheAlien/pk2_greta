//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once
#include "tileset.hpp"

class LevelSector{
public:
    LevelSector(std::size_t height, std::size_t width);
    ~LevelSector();

    /**
     * Do not copy objects
    */
    LevelSector(const LevelSector&)=delete;
    LevelSector& operator=(const LevelSector&)=delete;


    std::size_t getWidth()const{
        return this->mWidth;
    }

    std::size_t getHeight()const{
        return this->mHeight;
    }
    std::size_t size()const{
        return this->mSize;
    }

    void calculateEdges();
    void drawBackgroundTiles(int camera_x, int camera_y, int block_animation_frame)const;
    void drawForegroundTiles(int camera_x, int camera_y, int block_animation_frame,
    int arrows_block_degree, int button1_timer_y, int button2_timer_y, int button3_timer_y)const;

    u8* background_tiles = nullptr;
    u8* foreground_tiles = nullptr;
    u8* sprite_tiles = nullptr;
    bool * edges = nullptr;

    Tileset* tileset1 = nullptr;
    Tileset* tileset2 = nullptr;


private:
    std::size_t mHeight, mWidth, mSize;

};