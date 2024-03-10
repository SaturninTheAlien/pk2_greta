//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/platform.hpp"
#include "engine/PFile.hpp"

#include <vector>
#include <string>

typedef struct {
    int left, top, right, bottom;
} MAP_RECT;

#define PK2MAP_LAST_VERSION "1.3"

#define PK2MAP_MAP_WIDTH  256
#define PK2MAP_MAP_HEIGHT 224
#define PK2MAP_MAP_SIZE   PK2MAP_MAP_WIDTH * PK2MAP_MAP_HEIGHT

#define PK2MAP_MAP_MAX_PROTOTYPES 100

#define SWITCH_INITIAL_VALUE 2000

#define BLOCK_MAX_MASKS 150

enum {

    BLOCK_BARRIER_DOWN = 40,
    BLOCK_LIFT_HORI,
    BLOCK_LIFT_VERT,
    BLOCK_BUTTON2_UP,
    BLOCK_BUTTON3_RIGHT,
    BLOCK_BUTTON2_DOWN,
    BLOCK_BUTTON3_LEFT,
    BLOCK_LOCK,
    BLOCK_SKULL_FOREGROUND,
    BLOCK_SKULL_BACKGROUND,

    BLOCK_ANIM1 = 60,
    BLOCK_ANIM2 = 65,
    BLOCK_ANIM3 = 70,
    BLOCK_ANIM4 = 75,

    BLOCK_DRIFT_LEFT = 140,
    BLOCK_DRIFT_RIGHT,
    BLOCK_SCROLL_UP,
    BLOCK_HIDEOUT,
    BLOCK_FIRE,
    BLOCK_BUTTON1,
    BLOCK_BUTTON2,
    BLOCK_BUTTON3,
    BLOCK_START,
    BLOCK_EXIT,

};

enum {

    WEATHER_NORMAL,
    WEATHER_RAIN,
    WEATHER_LEAVES,
    WEATHER_RAIN_LEAVES,
    WEATHER_SNOW

};

enum {

    BACKGROUND_STATIC,
    BACKGROUND_PARALLAX_VERT,
    BACKGROUND_PARALLAX_HORI,
    BACKGROUND_PARALLAX_VERT_AND_HORI

};

enum BLOCKS {

	BLOCK_BACKGROUND,
	BLOCK_WALL

};


struct PK2BLOCK {

	u8   id;
	bool permeable;
	u8   left_side, right_side, top_side, bottom_side;
	int  left, right, top, bottom;
	bool water;
	bool border;

};

struct PK2BLOCKMASK {

	int ylos[32];
	int alas[32];
	int vasemmalle[32];
	int oikealle[32];

};

class LevelClass {
	private:

	int arrows_block_degree = 0;  // degree of movable blocks
	int tiles_animation_timer  = 0;  // timer for water, fire and waterfall animation
	int block_animation_frame = 0;  // block animations frame
	u32 button1_timer  = 0;  // button 1 timer
	u32 button2_timer  = 0;  // button 2 timer
	u32 button3_timer  = 0;  // button 3 timer

    public:

    /* Atributs ------------------------*/

    char     version[5]       = PK2MAP_LAST_VERSION;         // map version. eg "1.3"

    std::string tileset_name = "blox.bmp";                  // path of block palette .bmp
    std::string background_name = "default.bmp";            // path of map bg .bmp
    std::string music_name = "default.xm";                  // path of map music*/           

    std::string name = "untitled";
    std::string author = "unknown";
    /*char     name[40]   = "untitled";                       // map name
    char     author[40] = "unknown";                        // map author*/

    int      level_number          = 0;                            // level of the episode
    int      weather           = WEATHER_NORMAL;                // map climate
    int      map_time           = 0;                            // map time (in (dec)conds)
    int       extra          = 0;                            // extra config - not used
    int       background_scrolling = BACKGROUND_STATIC;            // bg movemant type
    u32      button1_time   = SWITCH_INITIAL_VALUE;         // button 1 time
    u32      button2_time   = SWITCH_INITIAL_VALUE;         // button 2 time
    u32      button3_time   = SWITCH_INITIAL_VALUE;         // button 3 time
    int      player_sprite_index = 0;                            // player prototype

    u8       background_tiles[PK2MAP_MAP_SIZE] = {255};              // map bg tiles 256*224
    u8       foreground_tiles[PK2MAP_MAP_SIZE] = {255};              // map fg tiles 256*224
    u8       sprite_tiles[PK2MAP_MAP_SIZE] = {255};              // map sprites 256*224
    
    
    //char     sprite_filenames[PK2MAP_MAP_MAX_PROTOTYPES][13] = {""}; // map prototype list .spr
    std::vector<std::string> sprite_prototype_names;
    bool     edges [PK2MAP_MAP_SIZE] = {false};            // map edges - calculated during game

    int      tiles_buffer      = -1;                        // index of block palette
    int      bg_tiles_buffer   = -1;
    int      background_buffer = -1;                        // index of bg image
    int      water_buffer      = -1;                        // index of water palette
    int      bg_water_buffer   = -1;

    int      icon_x = 0;                                         // map icon x pos
	int      icon_y = 0;                                         // map icon x pos
    int      icon_id = 0;                                      // map icon id

    std::string lua_script = "main.lua";                        // lua script


    /* Metodit --------------------------*/

    LevelClass();                                             // Oletusmuodostin
    ~LevelClass();                                            // Hajoitin

    void Load(PFile::Path path);                             // Load kartta
    void Load_Plain_Data(PFile::Path path, bool headerOnly);                  // Load kartta ilman grafiikoita

    int DrawBackgroundTiles(int kamera_x, int kamera_y);
    int DrawForegroundTiles(int kamera_x, int kamera_y);

    void SetTilesAnimations(int degree, int anim, u32 aika1, u32 aika2, u32 aika3);

    void Calculate_Edges();

    void SaveVersion20(const std::string & filename);
private:
    static void ReadVersion13Tiles(PFile::RW& file, u8* tiles); 

    void LoadVersion13(PFile::Path path, bool headerOnly);
    void LoadVersion20(PFile::Path path, bool headerOnly);


    int Load_BG(PFile::Path path);
    void Load_TilesImage(PFile::Path path);
    //int Load_BGSfx(PFile::Path path);

    void Animate_Fire(int tiles);
    void Animate_Waterfall(int tiles);
    void Animate_RollUp(int tiles);
    void Animate_WaterSurface(int tiles);
    void Animate_Water(int tiles, int water_tiles);
    
};
