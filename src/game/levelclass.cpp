//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "levelclass.hpp"
#include "exceptions.hpp"
#include <sstream>

#include "system.hpp"

#include "engine/PDraw.hpp"
#include "engine/PUtils.hpp"
#include "engine/PInput.hpp"
#include "engine/PLog.hpp"
#include "engine/PJson.hpp"

#include <cinttypes>
#include <cstring>
#include <cmath>
#include <array>

void LevelClass::SetTilesAnimations(int degree, int anim, u32 aika1, u32 aika2, u32 aika3) {

	arrows_block_degree = degree;
	block_animation_frame = anim;
	button1_timer = aika1;
	button2_timer = aika2;
	button3_timer = aika3;

}

LevelClass::LevelClass(){}

LevelClass::~LevelClass(){
	/*PDraw::image_delete(this->tiles_buffer);
	PDraw::image_delete(this->bg_tiles_buffer);
	PDraw::image_delete(this->background_buffer);
	PDraw::image_delete(this->water_buffer);
	PDraw::image_delete(this->bg_water_buffer);*/
}

void LevelClass::Load(PFile::Path path){
	this->Load_Plain_Data(path, false);

	path.SetFile(this->tileset_name);

	this->tileset1.clear();
	this->tileset1.loadImage(path);
	//this->tileset1.make254Transparent();
	if(this->splash_color == -1){
		this->splash_color = this->tileset1.calculateSplashColor();

	}


	path.SetFile(this->background_name);
	this->Load_BG(path);
	this->Calculate_Edges();
	this->calculateBlockTypes();	
}

void LevelClass::Load_Plain_Data(PFile::Path path, bool headerOnly) {
	try{
		char version[5];
		PFile::RW file = path.GetRW2("r");

		file.read(version, 4);
		version[4] = '\0';

		file.close();

		PLog::Write(PLog::DEBUG, "PK2", "Loading %s, version %s", path.c_str(), version);

		/**
		 * @brief 
		 * New GE level format.
		 * 1.4 intentionally skipped as it was used in PK2 Community Edition
		 */
		if(strcmp(version, "1.5")==0){
			this->LoadVersion15(path, headerOnly);
		}
		/**
		 * @brief 
		 * Legacy PK2 level format.
		 */
		else if (strcmp(version,"1.3")==0) {
			this->LoadVersion13(path, headerOnly);
		}

		else if (strcmp(version,"1.4")==0 ){
			/**
			 * @brief 
			 * TO DO
			 * Support PK2 Community Edition level format
			 */

			throw PExcept::PException("PK2 CE levels not supported yet!");
		}

		else{
			std::ostringstream os;
			os<<"Unsupported level format: \""<<version<<"\"";
			throw PExcept::PException(os.str());
		}

	}
	catch(const PFile::PFileException& e){
		PLog::Write(PLog::ERR,"PK2",e.what());
		throw PExcept::FileNotFoundException(path.c_str(), PExcept::MISSING_LEVEL);
	}
}


void LevelClass::ReadTiles(PFile::RW& file,
        u8 compression,
        u32 level_width,
        std::size_t level_size,
        u8* tiles){

	switch (compression)
	{
	case TILES_COMPRESSION_NONE:{
		file.read(tiles, level_size);
	}
	break;
	case TILES_COMPRESSION_LEGACY:{
		u32 width = 0, height = 0;
		u32 offset_x = 0, offset_y = 0;

		file.readLegacyStrU32(offset_x);
		file.readLegacyStrU32(offset_y);
		file.readLegacyStrU32(width);
		file.readLegacyStrU32(height);

		for (u32 y = offset_y; y <= offset_y + height; y++) {
			u32 x_start = offset_x + y * level_width;
			/**
			 * @brief 
			 * To prevent a memory leak
			 */
			if(x_start>=0 && x_start<level_size){
				file.read(&tiles[x_start], width + 1);
			}
			else{
				throw std::runtime_error("Malformed level file!");
			}
		}
	}
	break;

	default:
		std::ostringstream os;
		os<<"Tiles compression: "<<compression<<" not supported";
		throw PExcept::PException(os.str());
	}
}

void LevelClass::LoadVersion13(PFile::Path path, bool headerOnly){
	PFile::RW file = path.GetRW2("r");

	memset(this->background_tiles, 255, sizeof(this->background_tiles));
	memset(this->foreground_tiles , 255, sizeof(this->foreground_tiles));
	memset(this->sprite_tiles, 255, sizeof(this->sprite_tiles));


	file.read(version,      sizeof(version));

	file.readLegacyStr13Chars(this->tileset_name);
	file.readLegacyStr13Chars(this->background_name);
	file.readLegacyStr13Chars(this->music_name);


	char name_buffer[40];
	file.read(name_buffer, sizeof(name_buffer));
	name_buffer[39] = '\0';

	/**
	 * @brief 
	 * ???
	 * What's the purpose of this??
	 */
	for (int i = 38; i > 0 && (name[i] == (char)0xCD); i--)
		name_buffer[i] = 0;

	this->name = name_buffer;

	file.readLegacyStr40Chars(author);

	file.readLegacyStrInt(this->level_number);
	file.readLegacyStrInt(this->weather);
	file.readLegacyStrU32(this->button1_time);
	file.readLegacyStrU32(this->button2_time);
	file.readLegacyStrU32(this->button3_time);
	file.readLegacyStrInt(this->map_time);
	file.readLegacyStrInt(this->extra);
	file.readLegacyStrInt(this->background_scrolling);
	file.readLegacyStrInt(this->player_sprite_index);
	file.readLegacyStrInt(this->icon_x);
	file.readLegacyStrInt(this->icon_y);
	file.readLegacyStrInt(this->icon_id);

	/**
	 * @brief
	 * Loading only header for the map screen, it could stop now.
	 * There's no reason to load level tiles in that case.
	 */
	if(headerOnly){
		file.close();
		return;
	}

	// sprite prototypes

	u32 prototypesNumber = 0;
	file.readLegacyStrU32(prototypesNumber);
	if(prototypesNumber>PK2MAP_MAP_MAX_PROTOTYPES){
		std::ostringstream os;
		os<<"Too many level sprite prototypes: "<<prototypesNumber<<std::endl;
		os<<PK2MAP_MAP_MAX_PROTOTYPES<<" is the limit in the \"1.3\" level format";
		file.close();
		throw std::runtime_error(os.str());
	}

	char prototype_names_legacy[PK2MAP_MAP_MAX_PROTOTYPES][13] = {""};
	
	file.read(prototype_names_legacy, sizeof(prototype_names_legacy[0]) * prototypesNumber);

	this->sprite_prototype_names.resize(prototypesNumber);
	for(u32 i=0;i<prototypesNumber;++i){
		char * prototype_name = prototype_names_legacy[i];
		prototype_name[12] = '\0';
		this->sprite_prototype_names[i] = prototype_name;
	}

	// background_tiles

	ReadTiles(file, TILES_COMPRESSION_LEGACY,
		PK2MAP_MAP_WIDTH,
		PK2MAP_MAP_SIZE,
		this->background_tiles);

	// foreground_tiles
	ReadTiles(file, TILES_COMPRESSION_LEGACY,
		PK2MAP_MAP_WIDTH,
		PK2MAP_MAP_SIZE,
		this->foreground_tiles);

	// sprite_tiles
	ReadTiles(file, TILES_COMPRESSION_LEGACY,
		PK2MAP_MAP_WIDTH,
		PK2MAP_MAP_SIZE,
		this->sprite_tiles);
	

	file.close();
}


void LevelClass::LoadVersion15(PFile::Path path, bool headerOnly){

	using namespace PJson;
	PFile::RW file = path.GetRW2("r");

	file.read(version,      sizeof(version));

	u32 regions = 1; //placeholder;

	//Read level header
	{
		const nlohmann::json j = file.readCBOR();
		jsonReadString(j, "name", this->name);
		jsonReadString(j, "author", this->author);
		jsonReadInt(j, "level_number", this->level_number);
		jsonReadInt(j, "icon_x", this->icon_x);
		jsonReadInt(j, "icon_y", this->icon_y);
		jsonReadInt(j, "icon_id", this->icon_id);

		/**
		 * @brief
		 * If loading a level only for the map entry, finish here.
		 */
		if(headerOnly)return;

		jsonReadU32(regions, "regions", regions);

		if(j.contains("sprite_prototypes")){
			this->sprite_prototype_names = j["sprite_prototypes"].get<std::vector<std::string>>();
		}

		jsonReadInt(j, "player_index", this->player_sprite_index);
		jsonReadInt(j, "map_time", this->map_time);
		jsonReadString(j, "lua_script", this->lua_script);
	}

	if(regions!=1){
		throw PExcept::PException("Multiple regions aren't supported yet!"); //placeholder
	}

	u32 width = 0; // placeholder
	u32 height = 0; //placeholder
	u32 compression = 0;
	
	//Read region header
	{
		const nlohmann::json j = file.readCBOR();
		jsonReadU32(j, "width", width);
		jsonReadU32(j, "height", height);
		jsonReadU32(j, "compression", compression);

		jsonReadString(j, "tileset", this->tileset_name);
		jsonReadString(j, "music", this->music_name);

		jsonReadString(j, "background", this->background_name);
		jsonReadInt(j, "scrolling", this->background_scrolling);
		jsonReadInt(j, "weather", this->weather);

		jsonReadInt(j, "splash_color", this->splash_color);
		jsonReadInt(j, "fire_color_1", this->fire_color_1);
		jsonReadInt(j, "fire_color_2", this->fire_color_2);
	}

	if(width!=PK2MAP_MAP_WIDTH || height!=PK2MAP_MAP_HEIGHT){
		throw PExcept::PException("Custom-sized regions not implemented yet!");
	}

	//background tiles
	file.read(this->background_tiles, sizeof(u8)* PK2MAP_MAP_SIZE);

	//foreground tiles
	file.read(this->foreground_tiles, sizeof(u8)* PK2MAP_MAP_SIZE);

	//sprite tiles
	file.read(this->sprite_tiles, sizeof(u8)* PK2MAP_MAP_SIZE);

	file.close();
}


// TO DO
void LevelClass::SaveVersion15(PFile::Path path)const{

	PFile::RW file = path.GetRW2("w");
	char version[5] = "1.5";
	file.write(version, sizeof(version));

	//Write header
	{
		nlohmann::json j;
		j["name"] = this->name;
		j["author"] = this->author;
		j["level_number"] = this->level_number;
		j["icon_id"] = this->icon_id;
		j["icon_x"] = this->icon_x;
		j["icon_y"] = this->icon_y;

		j["sprite_prototypes"] = this->sprite_prototype_names;
		j["player_index"] = this->player_sprite_index;

		j["regions"] = 1;
		j["map_time"] = this->map_time;
		j["lua_script"] = this->lua_script;

		file.writeCBOR(j);
	}

	//Write level data
	{
		nlohmann::json j;

		j["width"] = PK2MAP_MAP_WIDTH;
		j["height"]= PK2MAP_MAP_HEIGHT;
		j["compression"] = TILES_COMPRESSION_NONE;

		j["tileset"] = this->tileset_name;
		j["music"] = this->music_name;
		j["background"] = this->background_name;
		j["scrolling"] = this->background_scrolling;
		j["weather"] = this->weather;

		j["splash_color"] = this->splash_color;
		j["fire_color_1"] = this->fire_color_1;
		j["fire_color_2"] = this->fire_color_2;

		file.writeCBOR(j);
	}

	//background tiles
	file.write(this->background_tiles, sizeof(u8)* PK2MAP_MAP_SIZE);

	//foreground tiles
	file.write(this->foreground_tiles, sizeof(u8)* PK2MAP_MAP_SIZE);

	//sprite tiles
	file.write(this->sprite_tiles, sizeof(u8)* PK2MAP_MAP_SIZE);

	file.close();
}

int LevelClass::Load_BG(PFile::Path path){
	
	if (!FindAsset(&path, "gfx" PE_SEP "scenery" PE_SEP))
		return 1;

	PDraw::image_load(this->background_buffer, path, true, false);
	if (this->background_buffer == -1)
		return -2;

	return 0;
}


void LevelClass::calculateBlockTypes(){
	PK2BLOCK block;

	for (int i=0;i<150;i++){
		block = this->block_types[i];

		block.left  = 0;
		block.right  = 0;//32
		block.top	   = 0;
		block.bottom    = 0;//32

		block.id  = i;

		if ((i < 80 || i > 139) && i != 255){
			block.right_side	= BLOCK_WALL;
			block.left_side	= BLOCK_WALL;
			block.top_side		= BLOCK_WALL;
			block.bottom_side		= BLOCK_WALL;

			// Erikoislattiat

			if (i > 139){
				block.right_side	= BLOCK_BACKGROUND;
				block.left_side	= BLOCK_BACKGROUND;
				block.top_side		= BLOCK_BACKGROUND;
				block.bottom_side		= BLOCK_BACKGROUND;
			}

			// L�pik�velt�v� lattia

			if (i == BLOCK_BARRIER_DOWN){
				block.right_side	= BLOCK_BACKGROUND;
				block.top_side		= BLOCK_BACKGROUND;
				block.bottom_side		= BLOCK_WALL;
				block.left_side	= BLOCK_BACKGROUND;
				block.bottom -= 27;
			}

			// M�et

			if (i > 49 && i < 60){
				block.right_side	= BLOCK_BACKGROUND;
				block.top_side		= BLOCK_WALL;
				block.bottom_side		= BLOCK_WALL;
				block.left_side	= BLOCK_BACKGROUND;
				block.bottom += 1;
			}

			// Kytkimet

			if (i >= BLOCK_BUTTON1 && i <= BLOCK_BUTTON3){
				block.right_side	= BLOCK_WALL;
				block.top_side		= BLOCK_WALL;
				block.bottom_side		= BLOCK_WALL;
				block.left_side	= BLOCK_WALL;
			}
		}
		else{
			block.right_side	= BLOCK_BACKGROUND;
			block.left_side	= BLOCK_BACKGROUND;
			block.top_side		= BLOCK_BACKGROUND;
			block.bottom_side		= BLOCK_BACKGROUND;
		}

		if (i > 131 && i < 140)
			block.water = true;
		else
			block.water = false;

		this->block_types[i] = block;
	}
}

void LevelClass::moveBlocks(u32 button1, u32 button2, u32 button3){
	this->block_types[BLOCK_LIFT_HORI].left = (int)cos_table(degree);
	this->block_types[BLOCK_LIFT_HORI].right = (int)cos_table(degree);

	this->block_types[BLOCK_LIFT_VERT].bottom = (int)sin_table(degree);
	this->block_types[BLOCK_LIFT_VERT].top = (int)sin_table(degree);

	int kytkin1_y = 0,
		kytkin2_y = 0,
		kytkin3_x = 0;

	if (button1 > 0) {
		kytkin1_y = 64;

		if (button1 < 64)
			kytkin1_y = button1;

		if (button1 > this->button1_time - 64)
			kytkin1_y = this->button1_time - button1;
	}

	if (button2 > 0) {
		kytkin2_y = 64;

		if (button2 < 64)
			kytkin2_y = button2;

		if (button2 > this->button2_time - 64)
			kytkin2_y = this->button2_time - button2;
	}

	if (button3 > 0) {
		kytkin3_x = 64;

		if (button3 < 64)
			kytkin3_x = button3;

		if (button3 > this->button3_time - 64)
			kytkin3_x = this->button3_time - button3;
	}

	kytkin1_y /= 2;
	kytkin2_y /= 2;
	kytkin3_x /= 2;

	this->block_types[BLOCK_BUTTON1].bottom = kytkin1_y;
	this->block_types[BLOCK_BUTTON1].top = kytkin1_y;

	this->block_types[BLOCK_BUTTON2_UP].bottom = -kytkin2_y;
	this->block_types[BLOCK_BUTTON2_UP].top = -kytkin2_y;

	this->block_types[BLOCK_BUTTON2_DOWN].bottom = kytkin2_y;
	this->block_types[BLOCK_BUTTON2_DOWN].top = kytkin2_y;

	this->block_types[BLOCK_BUTTON2].bottom = kytkin2_y;
	this->block_types[BLOCK_BUTTON2].top = kytkin2_y;

	this->block_types[BLOCK_BUTTON3_RIGHT].right = kytkin3_x;
	this->block_types[BLOCK_BUTTON3_RIGHT].left = kytkin3_x;
	this->block_types[BLOCK_BUTTON3_RIGHT].id = BLOCK_LIFT_HORI;

	this->block_types[BLOCK_BUTTON3_LEFT].right = -kytkin3_x;
	this->block_types[BLOCK_BUTTON3_LEFT].left = -kytkin3_x;
	this->block_types[BLOCK_BUTTON3_LEFT].id = BLOCK_LIFT_HORI;

	this->block_types[BLOCK_BUTTON3].bottom = kytkin3_x;
	this->block_types[BLOCK_BUTTON3].top = kytkin3_x;
}

void LevelClass::Calculate_Edges(){

	u8 tile1, tile2, tile3;

	memset(this->edges, false, sizeof(this->edges));

	for (u32 x = 1; x < PK2MAP_MAP_WIDTH - 1; x++)
		for (u32 y = 0; y < PK2MAP_MAP_HEIGHT - 1; y++){
			bool edge = false;

			tile1 = this->foreground_tiles[x+y*PK2MAP_MAP_WIDTH];

			if (tile1 > BLOCK_EXIT)
				this->foreground_tiles[x+y*PK2MAP_MAP_WIDTH] = 255;

			tile2 = this->foreground_tiles[x+(y+1)*PK2MAP_MAP_WIDTH];

			if (tile1 > 79 || tile1 == BLOCK_BARRIER_DOWN) tile1 = 1; else tile1 = 0;
			if (tile2 > 79) tile2 = 1; else tile2 = 0;

			if (tile1 == 1 && tile2 == 1){
				tile1 = this->foreground_tiles[x+1+(y+1)*PK2MAP_MAP_WIDTH];
				tile2 = this->foreground_tiles[x-1+(y+1)*PK2MAP_MAP_WIDTH];

				if (tile1 < 80  && !(tile1 < 60 && tile1 > 49)) tile1 = 1; else tile1 = 0;
				if (tile2 < 80  && !(tile2 < 60 && tile2 > 49)) tile2 = 1; else tile2 = 0;

				if (tile1 == 1){
					tile3 = this->foreground_tiles[x+1+y*PK2MAP_MAP_WIDTH];
					if (tile3 > 79 || (tile3 < 60 && tile3 > 49) || tile3 == BLOCK_BARRIER_DOWN)
						edge = true;
				}

				if (tile2 == 1){
					tile3 = this->foreground_tiles[x-1+y*PK2MAP_MAP_WIDTH];
					if (tile3 > 79 || (tile3 < 60 && tile3 > 49) || tile3 == BLOCK_BARRIER_DOWN)
						edge = true;
				}

				if (edge){
					this->edges[x+y*PK2MAP_MAP_WIDTH] = true;
					//this->background_tiles[x+y*PK2MAP_MAP_WIDTH] = 18; //Debug
				}
			}
		}
}


int LevelClass::DrawBackgroundTiles(int kamera_x, int kamera_y){
	
	int kartta_x = kamera_x/32;
	int kartta_y = kamera_y/32;

	int tiles_w = screen_width/32 + 1;
	int tiles_h = screen_height/32 + 1;


	for (int x = 0; x < tiles_w; x++){
		if (x + kartta_x < 0 || uint(x + kartta_x) > PK2MAP_MAP_WIDTH) continue;

		for (int y = 0; y < tiles_h; y++){
			if (y + kartta_y < 0 || uint(y + kartta_y) > PK2MAP_MAP_HEIGHT) continue;

			int i = x + kartta_x + (y + kartta_y) * PK2MAP_MAP_WIDTH;
			if( i < 0 || i >= int(sizeof(background_tiles)) ) continue; //Dont access a not allowed address

			int block = background_tiles[i];

			if (block != 255){
				int px = ((block%10)*32);
				int py = ((block/10)*32);

				if (block == BLOCK_ANIM1 || block == BLOCK_ANIM2 || block == BLOCK_ANIM3 || block == BLOCK_ANIM4)
					px += block_animation_frame * 32;

				PDraw::image_cutclip(this->tileset1.getImage(), x*32-(kamera_x%32), y*32-(kamera_y%32), px, py, px+32, py+32);
			}
		}
	}

	return 0;
}

int LevelClass::DrawForegroundTiles(int kamera_x, int kamera_y){

	int kartta_x = kamera_x / 32;
	int kartta_y = kamera_y / 32;

	int button1_timer_y = 0;
	int button2_timer_y = 0;
	int button3_timer_y = 0;

	int tiles_w = screen_width  / 32 + 1;
	int tiles_h = screen_height / 32 + 1;

	if (button1_timer > 0){
		button1_timer_y = 64;

		if (button1_timer < 64)
			button1_timer_y = button1_timer;

		if (button1_timer > button1_time - 64)
			button1_timer_y = button1_time - button1_timer;
	}

	if (button2_timer > 0){
		button2_timer_y = 64;

		if (button2_timer < 64)
			button2_timer_y = button2_timer;

		if (button2_timer > button2_time - 64)
			button2_timer_y = button2_time - button2_timer;
	}

	if (button3_timer > 0){
		button3_timer_y = 64;

		if (button3_timer < 64)
			button3_timer_y = button3_timer;

		if (button3_timer > button3_time - 64)
			button3_timer_y = button3_time - button3_timer;
	}


	for (int x = -1; x < tiles_w + 1; x++) {
		if (x + kartta_x < 0 || uint(x + kartta_x) > PK2MAP_MAP_WIDTH) continue;

		for (int y = -1; y < tiles_h + 1; y++) {
			if (y + kartta_y < 0 || uint(y + kartta_y) > PK2MAP_MAP_HEIGHT) continue;

			int i = x + kartta_x + (y + kartta_y) * PK2MAP_MAP_WIDTH;
			if( i < 0 || i >= int(sizeof(foreground_tiles)) ) continue; //Dont access a not allowed address

			u8 block = foreground_tiles[i];

			if (block != 255 && block != BLOCK_BARRIER_DOWN){
				
				int px = (block % 10) * 32;
				int py = (block / 10) * 32;
				
				int ay = 0;
				int ax = 0;

				if (block == BLOCK_LIFT_VERT)
					ay = floor(sin_table(arrows_block_degree));

				else if (block == BLOCK_LIFT_HORI)
					ax = floor(cos_table(arrows_block_degree));

				else if (block == BLOCK_BUTTON1)
					ay = button1_timer_y/2;

				else if (block == BLOCK_BUTTON2_UP)
					ay = -button2_timer_y/2;

				else if (block == BLOCK_BUTTON2_DOWN)
					ay = button2_timer_y/2;

				else if (block == BLOCK_BUTTON2)
					ay = button2_timer_y/2;

				else if (block == BLOCK_BUTTON3_RIGHT)
					ax = button3_timer_y/2;

				else if (block == BLOCK_BUTTON3_LEFT)
					ax = -button3_timer_y/2;

				else if (block == BLOCK_BUTTON3)
					ay = button3_timer_y/2;

				else if (block == BLOCK_ANIM1 || block == BLOCK_ANIM2 || block == BLOCK_ANIM3 || block == BLOCK_ANIM4)
					px += block_animation_frame * 32;

				//hide drift tiles
				else if(block == BLOCK_DRIFT_LEFT || block == BLOCK_DRIFT_RIGHT)
					continue;

				PDraw::image_cutclip(this->tileset1.getImage(), x*32-(kamera_x%32)+ax, y*32-(kamera_y%32)+ay, px, py, px+32, py+32);
			}
		}
	}

	if (tiles_animation_timer%2 == 0)
	{

		tileset1.animateFire(this->button1_timer, this->fire_color_1, this->fire_color_2);
		tileset1.animateWaterfall();
		tileset1.animateWaterSurface();
		tileset1.animateRollUp();

		if (tiles_animation_timer%4 == 0)
		{
			tileset1.animateWater(tiles_animation_timer);
			PDraw::rotate_palette(224,239);
		}

	}

	tiles_animation_timer = 1 + tiles_animation_timer % 320;

	return 0;
}
