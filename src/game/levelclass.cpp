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
	PDraw::image_delete(this->tiles_buffer);
	PDraw::image_delete(this->bg_tiles_buffer);
	PDraw::image_delete(this->background_buffer);
	PDraw::image_delete(this->water_buffer);
	PDraw::image_delete(this->bg_water_buffer);
}

void LevelClass::Load(PFile::Path path){
	this->Load_Plain_Data(path, false);
	
	path.SetFile(this->tileset_name);
	Load_TilesImage(path);

	path.SetFile(this->background_name);
	Load_BG(path);

	Calculate_Edges();
}

void LevelClass::Load_Plain_Data(PFile::Path path, bool headerOnly) {
	try{
		char version[5];
		PFile::RW file = path.GetRW2("r");

		file.read(version, 4);
		version[4] = '\0';

		file.close();

		PLog::Write(PLog::DEBUG, "PK2", "Loading %s, version %s", path.c_str(), version);

		if (strcmp(version,"1.3")==0) {
			this->LoadVersion13(path, headerOnly);
		}
		/**
		 * @brief 
		 * Testing the new level format
		 */
		else if(strcmp(version, "2.p")==0){
			this->LoadVersion20(path, headerOnly);
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


void LevelClass::ReadVersion13Tiles(PFile::RW& file, u8* tiles){
	u32 width = 0, height = 0;
	u32 offset_x = 0, offset_y = 0;

	// background_tiles

	file.readLegacyStrU32(offset_x);
	file.readLegacyStrU32(offset_y);
	file.readLegacyStrU32(width);
	file.readLegacyStrU32(height);

	for (u32 y = offset_y; y <= offset_y + height; y++) {
		u32 x_start = offset_x + y * PK2MAP_MAP_WIDTH;

		/**
		 * @brief 
		 * To prevent a memory leak
		 */
		if(x_start>=0 && x_start<PK2MAP_MAP_SIZE){
			file.read(&tiles[x_start], width + 1);
		}
		else{
			throw std::runtime_error("Malformed level file!");
		}
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

	ReadVersion13Tiles(file, this->background_tiles);

	// foreground_tiles
	ReadVersion13Tiles(file, this->foreground_tiles);

	// sprite_tiles
	ReadVersion13Tiles(file, this->sprite_tiles);
	

	file.close();
}
// TO DO
void LevelClass::SaveVersion20(const std::string& filename){
	PFile::Path path(filename);

	PFile::RW file = path.GetRW2("w");
	char version[5] = "2.p";
	file.write(version, sizeof(version));

	//Write header
	{
		nlohmann::json header;
		header["name"] = this->name;
		header["author"] = this->author;
		header["level_number"] = this->level_number;
		header["icon_id"] = this->icon_id;
		header["icon_x"] = this->icon_x;
		header["icon_y"] = this->icon_y;

		file.writeCBOR(header);
	}

	//Write level data
	{
		nlohmann::json j;
		j["background"] = this->background_name;
		j["tileset"] = this->tileset_name;
		j["music"] = this->music_name;
		j["scrolling"] = this->background_scrolling;
		j["weather"] = this->weather;
		j["map_time"] = this->map_time;
		j["player_index"] = this->player_sprite_index;

		j["button1_time"] = this->button1_time;
		j["button2_time"] = this->button2_time;
		j["button3_time"] = this->button3_time;

		j["lua_script"] = this->lua_script;
		j["sprite_prototypes"] = this->sprite_prototype_names;

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
void LevelClass::LoadVersion20(PFile::Path path, bool headerOnly){

	using namespace PJson;
	PFile::RW file = path.GetRW2("r");

	file.read(version,      sizeof(version));

	//Read header
	{
		const nlohmann::json header = file.readCBOR();
		jsonReadString(header, "name", this->name);
		jsonReadString(header, "author", this->author);
		jsonReadInt(header, "level_number", this->level_number);
		jsonReadInt(header, "icon_x", this->icon_x);
		jsonReadInt(header, "icon_y", this->icon_y);
		jsonReadInt(header, "icon_id", this->icon_id);
	}

	if(headerOnly){
		return;
	}

	//Read level data
	{
		const nlohmann::json j = file.readCBOR();
		jsonReadString(j, "background", this->background_name);
		jsonReadString(j, "tileset", this->tileset_name);
		jsonReadString(j, "music", this->music_name);

		jsonReadInt(j, "scrolling", this->background_scrolling);
		jsonReadInt(j, "weather", this->weather);
		jsonReadInt(j, "map_time", this->map_time);
		jsonReadInt(j, "player_index", this->player_sprite_index);

		jsonReadString(j, "lua_script", this->lua_script);

		if(j.contains("sprite_prototypes")){
			this->sprite_prototype_names = j["sprite_prototypes"].get<std::vector<std::string>>();
		}

	}

	//background tiles
	file.read(this->background_tiles, sizeof(u8)* PK2MAP_MAP_SIZE);

	//foreground tiles
	file.read(this->foreground_tiles, sizeof(u8)* PK2MAP_MAP_SIZE);

	//sprite tiles
	file.read(this->sprite_tiles, sizeof(u8)* PK2MAP_MAP_SIZE);

	file.close();
}

int LevelClass::Load_BG(PFile::Path path){
	
	if (!FindAsset(&path, "gfx" PE_SEP "scenery" PE_SEP))
		return 1;

	PDraw::image_load(this->background_buffer, path, true, false);
	if (this->background_buffer == -1)
		return -2;

	//strcpy(this->taustakuva, filename.c_str());

	/*
	u8 *buffer = NULL;
	u32 width;

	PDraw::drawimage_start(background_buffer, buffer, width);

	for ( int x = 0; x < 640; x++)
		for ( int y = 0; y < 480; y++) {

			if (buffer[x+y*width] == 255)
				buffer[x+y*width] = 254;
			
		}

	PDraw::drawimage_end(background_buffer);
	*/

	return 0;
}

void LevelClass::Load_TilesImage(PFile::Path path){
	
	PFile::Path bkp = path;

	if (!FindAsset(&path, "gfx" PE_SEP "tiles" PE_SEP)){
		throw PExcept::FileNotFoundException(path.c_str(), PExcept::MISSING_TILESET);
	}

	PDraw::image_load(this->tiles_buffer, path, false);
	if (this->tiles_buffer == -1){
		throw PExcept::FileNotFoundException(path.c_str(), PExcept::MISSING_TILESET);
	}

	this->average_water_color = CalculateSplashColor(this->tiles_buffer);


	PDraw::image_delete(this->water_buffer); //Delete last water buffer
	this->water_buffer = PDraw::image_cut(this->tiles_buffer,0,416,320,32);

	/*// load bg buffer
	{
		// transform tiles01.bmp to tiles01_bg.bmp
		path = bkp;
		std::string filename = path.GetFileName();
		size_t i = filename.find_last_of('.');
		filename = filename.substr(0, i) + "_bg" + filename.substr(i, std::string::npos);
		path.SetFile(filename);
		if (FindAsset(&path, "gfx" PE_SEP "tiles" PE_SEP)) {
			PDraw::image_load(this->bg_tiles_buffer, path, false);
			if (this->bg_tiles_buffer >= 0) {
				PDraw::image_delete(this->bg_water_buffer); //Delete last water buffer
				this->bg_water_buffer = PDraw::image_cut(this->bg_tiles_buffer,0,416,320,32);
			}
		}
	}*/
}
/*
int LevelClass::Load_BGSfx(PFile::Path path){

	return 0;
}*/

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


int LevelClass::CalculateSplashColor(int tiles){
	u8* buffer = nullptr;
	u32 width = 0;

	PDraw::drawimage_start(tiles, buffer, width);

	/**
	 * @brief
	 * gray 	 - 0 
	 * blue 	 - 1
	 * red 		 - 2
	 * green	 - 3
	 * orange	 - 4
	 * violet	 - 5
	 * turquoise - 6
	 * unknown	 - 7
	 */

	std::array<int, 8> colorCounters = {0};
	for(int y = 416; y < 448; ++y){
		for(int x = 64; x < 320; ++x){

			int color = buffer[x + y*width];
			color /= 32;
			if(color < 7){
				colorCounters[color] += 1;
			}
			else{
				colorCounters[7] += 1; //Unknown colors
			}
		}
	}

	PDraw::drawimage_end(tiles);

	int max_val = colorCounters[0];
	int max_index = 0;

	for(int i=1; i < 8; ++i){
		int val = colorCounters[i];

		if(val > max_val){
			max_val = val;
			max_index = i;
		}
	}

	if(max_index==7){
		/**
		 * @brief 
		 * Fallback to the blue if unknown colors predominate.
		 */
		max_index = 1;
	}

	return 32* max_index;
}


/* Tileset animations ----------------------------------------------------------------*/

void LevelClass::Animate_Fire(int tiles){
	u8 *buffer = NULL;
	u32 width;
	int x,y;
	int color;

	PDraw::drawimage_start(tiles, buffer, width);

	for (x=128;x<160;x++)
		for (y=448;y<479;y++)
		{
			color = buffer[x+(y+1)*width];

			if (color != 255)
			{
				color %= 32;
				color = color - rand()%4;

				if (color < 0)
					color = 255;
				else
				{
					if (color > 21)
						color += 128;
					else
						color += 64;
				}
			}

			buffer[x+y*width] = color;
		}

	if (button1_timer < 20)
	{
		for (x=128;x<160;x++)
			buffer[x+479*width] = rand()%15+144;
	}
	else
		for (x=128;x<160;x++)
			buffer[x+479*width] = 255;

	PDraw::drawimage_end(tiles);
}

void LevelClass::Animate_Waterfall(int tiles){
	u8 *buffer = NULL;
	u32 width;
	int x,y,plus;
	int color,color2;

	u8 temp[32*32];

	PDraw::drawimage_start(tiles, buffer, width);

	for (x=32;x<64;x++)
		for (y=416;y<448;y++)
			temp[x-32+(y-416)*32] = buffer[x+y*width];

	color2 = (temp[0]/32)*32;	// mahdollistaa erivriset vesiputoukset

	for (x=32;x<64;x++)
	{
		plus = rand()%2+2;//...+1
		for (y=416;y<448;y++)
		{
			color = temp[x-32+(y-416)*32];

			if (color != 255)	// mahdollistaa eri leveyksiset vesiputoukset
			{
				color %= 32;
				if (color > 10)//20
					color--;
				if (rand()%40 == 1)
					color = 11+rand()%11;//15+rand()%8;//9+rand()%5;
				if (rand()%160 == 1)
					color = 30;
				buffer[x + (416+(y+plus)%32)*width] = color+color2;
			}
			else
				buffer[x + (416+(y+plus)%32)*width] = color;
		}
	}

	PDraw::drawimage_end(tiles);
}

void LevelClass::Animate_WaterSurface(int tiles){
	u8 *buffer = NULL;
	u32 width;
	int x,y;

	u8 temp[32];

	PDraw::drawimage_start(tiles, buffer, width);

	for (y=416;y<448;y++)
		temp[y-416] = buffer[y*width];

	for (y=416;y<448;y++)
	{
		for (x=0;x<31;x++)
		{
			buffer[x+y*width] = buffer[x+1+y*width];
		}
	}

	for (y=416;y<448;y++)
		buffer[31+y*width] = temp[y-416];

	PDraw::drawimage_end(tiles);
}

void LevelClass::Animate_Water(int tiles, int water_tiles){
	u8 *buffer_lahde = NULL, *buffer_kohde = NULL;
	u32 leveys_lahde, leveys_kohde;
	int x, y, color1, color2;
	int d1 = tiles_animation_timer / 2, d2;
	int sini, cosi;
	int vx,vy;
	int i;


	PDraw::drawimage_start(tiles, buffer_kohde, leveys_kohde);
	PDraw::drawimage_start(water_tiles, buffer_lahde, leveys_lahde);

	for (y=0;y<32;y++){
		d2 = d1;

		for (x=0;x<32;x++){
			sini = (y+d2/2) * 11.25;
			cosi = (x+d1/2) * 11.25;
			sini = (int)sin_table(sini);
			cosi = (int)cos_table(cosi);

			vy = (y+sini/11)%32;
			vx = (x+cosi/11)%32;

			if (vy < 0){
				vy = -vy;
				vy = 31-(vy%32);
			}

			if (vx < 0){
				vx= -vx;
				vx = 31-(vx%32);
			}

			color1 = buffer_lahde[64+vx+vy*leveys_lahde];
			buffer_lahde[32+x+y*leveys_lahde] = color1;
			d2 = 1 + d2 % 360;
		}

		d1 = 1 + d1 % 360;
	}

	int vy2;

	for (int p=2;p<5;p++){
		i = p*32;

		for (y=0;y<32;y++){
			//d2 = d1;
			vy = y*leveys_lahde;
			vy2 = (y+416)*leveys_kohde;

			for (x=0;x<32;x++){
				vx = x+vy;
				color1 = buffer_lahde[32+vx];
				color2 = buffer_lahde[ i+vx];
				buffer_kohde[i+x+vy2] = (color1 + color2*2) / 3;
			}
		}
	}
	PDraw::drawimage_end(tiles);
	PDraw::drawimage_end(water_tiles);
}

void LevelClass::Animate_RollUp(int tiles){
	u8 *buffer = NULL;
	u32 width;
	int y;

	u8 temp[32];

	PDraw::drawimage_start(tiles, buffer, width);

	memcpy(temp, 64 + buffer + 448*width, 32);

	for (y=448; y<479; y++)
		memcpy(buffer + 64 + y*width, buffer + 64 + (y+1)*width, 32);

	memcpy(buffer + 64 + 479*width, temp, 32);

	PDraw::drawimage_end(tiles);
}

int LevelClass::DrawBackgroundTiles(int kamera_x, int kamera_y){
	
	int kartta_x = kamera_x/32;
	int kartta_y = kamera_y/32;

	int tiles_w = screen_width/32 + 1;
	int tiles_h = screen_height/32 + 1;

	int buffer = bg_tiles_buffer;
	if (buffer < 0)
		buffer = tiles_buffer;

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

				PDraw::image_cutclip(buffer, x*32-(kamera_x%32), y*32-(kamera_y%32), px, py, px+32, py+32);
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

				PDraw::image_cutclip(tiles_buffer, x*32-(kamera_x%32)+ax, y*32-(kamera_y%32)+ay, px, py, px+32, py+32);
			}
		}
	}

	if (tiles_animation_timer%2 == 0)
	{
		Animate_Fire(this->tiles_buffer);
		Animate_Waterfall(this->tiles_buffer);
		Animate_RollUp(this->tiles_buffer);
		Animate_WaterSurface(this->tiles_buffer);

		if (this->bg_tiles_buffer >= 0) {
			Animate_Fire(this->bg_tiles_buffer);
			Animate_Waterfall(this->bg_tiles_buffer);
			Animate_RollUp(this->bg_tiles_buffer);
			Animate_WaterSurface(this->bg_tiles_buffer);
		}
	}

	if (tiles_animation_timer%4 == 0)
	{
		Animate_Water(this->tiles_buffer, this->water_buffer);
		if (this->bg_tiles_buffer >= 0)
			Animate_Water(this->bg_tiles_buffer, this->bg_water_buffer);

		PDraw::rotate_palette(224,239);
	}

	tiles_animation_timer = 1 + tiles_animation_timer % 320;

	return 0;
}
