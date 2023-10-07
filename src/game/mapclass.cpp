//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "game/mapclass.hpp"
#include "exceptions.hpp"
#include <sstream>

#include "system.hpp"

#include "engine/PDraw.hpp"
#include "engine/PUtils.hpp"
#include "engine/PInput.hpp"
#include "engine/PLog.hpp"

#include <cinttypes>
#include <cstring>
#include <cmath>

struct PK2KARTTA{ // Vanha version 0.1
	char version[8];
	char name[40];
	u8   taustakuva;
	u8   musiikki;
	u8   kartta [640*224];
	u8   palikat[320*256];
	u8   extrat [640*480];
};

void MapClass::SetTilesAnimations(int degree, int anim, u32 aika1, u32 aika2, u32 aika3) {

	arrows_block_degree = degree;
	block_animation_frame = anim;
	button1_timer = aika1;
	button2_timer = aika2;
	button3_timer = aika3;

}

MapClass::MapClass(){}

MapClass::~MapClass(){
	PDraw::image_delete(this->tiles_buffer);
	PDraw::image_delete(this->bg_tiles_buffer);
	PDraw::image_delete(this->background_buffer);
	PDraw::image_delete(this->water_buffer);
	PDraw::image_delete(this->bg_water_buffer);
}

int MapClass::Load(PFile::Path path){
	
	char version[8];

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr){
		return 1;
	}

	file->read(version, sizeof(version));
	file->close();

	int ok = 2;

	PLog::Write(PLog::DEBUG, "PK2", "Loading %s, version %s", path.c_str(), version);

	if (strcmp(version,"1.3")==0) {
		this->LoadVersion13(path);
		ok = 0;
	}
	if (strcmp(version,"1.2")==0) {
		this->LoadVersion12(path);
		ok = 0;
	}
	if (strcmp(version,"1.1")==0) {
		this->LoadVersion11(path);
		ok = 0;
	}
	if (strcmp(version,"1.0")==0) {
		this->LoadVersion10(path);
		ok = 0;
	}
	if (strcmp(version,"0.1")==0) {
		this->LoadVersion01(path);
		ok = 0;
	}
	
	path.SetFile(this->tileset_filename);
	Load_TilesImage(path);

	path.SetFile(this->background_filename);
	Load_BG(path);

	Calculate_Edges();

	return ok;
}

int MapClass::Load_Plain_Data(PFile::Path path) {
	
	char version[8];

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr){
		return 1;
	}

	file->read(version, sizeof(version));
	file->close();

	if (strcmp(version,"1.3")==0)
		this->LoadVersion13(path);

	if (strcmp(version,"1.2")==0)
		this->LoadVersion12(path);

	if (strcmp(version,"1.1")==0)
		this->LoadVersion11(path);

	if (strcmp(version,"1.0")==0)
		this->LoadVersion10(path);

	if (strcmp(version,"0.1")==0)
		this->LoadVersion01(path);

	return(0);
}

int MapClass::LoadVersion01(PFile::Path path){

	PK2KARTTA kartta;

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {
		return 1;
	}

	file->read(&kartta, sizeof(PK2KARTTA));
	file->close();

	strcpy(this->version, PK2MAP_LAST_VERSION);
	strcpy(this->tileset_filename,"blox.bmp");
	strcpy(this->background_filename, "default.bmp");
	strcpy(this->music_filename,   "default.xm");

	strcpy(this->name,  "v01");
	strcpy(this->author,"unknown");

	this->map_time		= 0;
	this->extra		= 0;
	this->background_scrolling	= kartta.taustakuva;

	for (u32 i=0;i<PK2MAP_MAP_SIZE;i++)
		this->foreground_tiles[i] = kartta.kartta[i%PK2MAP_MAP_WIDTH + (i/PK2MAP_MAP_WIDTH) * 640];

	memset(this->background_tiles,255, sizeof(background_tiles));

	memset(this->sprite_tiles,255, sizeof(sprite_tiles));

	return(0);
}
int MapClass::LoadVersion10(PFile::Path path){
	
	MapClass kartta;

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {
		return 1;
	}

	file->read(&kartta, sizeof(PK2KARTTA));
	file->close();

	strcpy(this->version,		kartta.version);
	strcpy(this->tileset_filename,	kartta.tileset_filename);
	strcpy(this->background_filename,	kartta.background_filename);
	strcpy(this->music_filename,		kartta.music_filename);

	strcpy(this->name,			kartta.name);
	strcpy(this->author,		kartta.author);

	this->map_time			= kartta.map_time;
	this->extra			= kartta.extra;
	this->background_scrolling		= kartta.background_scrolling;

	for (u32 i=0; i<PK2MAP_MAP_SIZE; i++)
		this->background_tiles[i] = kartta.background_tiles[i];

	for (u32 i=0; i<PK2MAP_MAP_SIZE;i++)
		this->foreground_tiles[i] = kartta.foreground_tiles[i];

	for (u32 i=0; i<PK2MAP_MAP_SIZE; i++)
		this->sprite_tiles[i] = kartta.sprite_tiles[i];

	return 0;
}
int MapClass::LoadVersion11(PFile::Path path){
	int virhe = 0;

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {
		return 1;
	}

	memset(this->background_tiles, 255, sizeof(this->background_tiles));
	memset(this->foreground_tiles , 255, sizeof(this->foreground_tiles));
	memset(this->sprite_tiles, 255, sizeof(this->sprite_tiles));

	file->read(this->version,      sizeof(char) * 5);
	file->read(this->tileset_filename, sizeof(char) * 13);
	file->read(this->background_filename,  sizeof(char) * 13);
	file->read(this->music_filename,    sizeof(char) * 13);
	file->read(this->name,        sizeof(char) * 40);
	file->read(this->author,      sizeof(char) * 40);
	file->read(&this->map_time,       sizeof(int));
	file->read(&this->extra,      sizeof(u8));
	file->read(&this->background_scrolling,     sizeof(u8));
	file->read(this->background_tiles,     sizeof(background_tiles));
	if (file->read(this->foreground_tiles,  sizeof(foreground_tiles)) != PK2MAP_MAP_SIZE)
		virhe = 2;
	file->read(this->sprite_tiles,     sizeof(sprite_tiles));

	file->close();

	for (u32 i=0;i<PK2MAP_MAP_SIZE;i++)
		if (foreground_tiles[i] != 255)
			foreground_tiles[i] -= 50;

	for (u32 i=0;i<PK2MAP_MAP_SIZE;i++)
		if (background_tiles[i] != 255)
			background_tiles[i] -= 50;

	for (u32 i=0;i<PK2MAP_MAP_SIZE;i++)
		if (sprite_tiles[i] != 255)
			sprite_tiles[i] -= 50;

	return (virhe);
}
int MapClass::LoadVersion12(PFile::Path path){

	char luku[8];
	
	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {
		return 1;
	}

	memset(this->background_tiles, 255, sizeof(this->background_tiles));
	memset(this->foreground_tiles , 255, sizeof(this->foreground_tiles));
	memset(this->sprite_tiles, 255, sizeof(this->sprite_tiles));

	for (u32 i=0; i<PK2MAP_MAP_MAX_PROTOTYPES; i++)
		strcpy(this->sprite_filenames[i],"");

	//tiedosto->read ((char *)this, sizeof (*this));
	file->read(version,      sizeof(version));
	file->read(tileset_filename, sizeof(tileset_filename));
	file->read(background_filename,  sizeof(background_filename));
	file->read(music_filename,    sizeof(music_filename));
	file->read(name,        sizeof(name));
	file->read(author,      sizeof(author));

	file->read(luku, sizeof(luku));
	this->level_number = atoi(luku);

	file->read(luku, sizeof(luku));
	this->weather = atoi(luku);

	file->read(luku, sizeof(luku));
	this->button1_time = atoi(luku);

	file->read(luku, sizeof(luku));
	this->button2_time = atoi(luku);

	file->read(luku, sizeof(luku));
	this->button3_time = atoi(luku);

	file->read(luku, sizeof(luku));
	this->map_time = atoi(luku);

	file->read(luku, sizeof(luku));
	this->extra = atoi(luku);

	file->read(luku, sizeof(luku));
	this->background_scrolling = atoi(luku);

	file->read(luku, sizeof(luku));
	this->player_sprite_index = atoi(luku);

	file->read(background_tiles, sizeof(background_tiles));
	file->read(foreground_tiles,  sizeof(foreground_tiles));
	file->read(sprite_tiles, sizeof(sprite_tiles));

	file->read(sprite_filenames, sizeof(sprite_filenames[0]) * PK2MAP_MAP_MAX_PROTOTYPES);

	file->close();

	return 0;
}
int MapClass::LoadVersion13(PFile::Path path){

	char luku[8];
	u32 i;

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {
		return 1;
	}

	memset(this->background_tiles, 255, sizeof(this->background_tiles));
	memset(this->foreground_tiles , 255, sizeof(this->foreground_tiles));
	memset(this->sprite_tiles, 255, sizeof(this->sprite_tiles));

	for (i=0;i<PK2MAP_MAP_MAX_PROTOTYPES;i++)
		strcpy(this->sprite_filenames[i],"");

	file->read(version,      sizeof(version));
	file->read(tileset_filename, sizeof(tileset_filename));
	file->read(background_filename,  sizeof(background_filename));
	file->read(music_filename,    sizeof(music_filename));
	file->read(name,        sizeof(name));

	for (int i = 38; i > 0 && (name[i] == (char)0xCD); i--)
		name[i] = 0;
	
	file->read(author,      sizeof(author));

	file->read(luku, sizeof(luku));
	this->level_number = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->weather = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->button1_time = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->button2_time = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->button3_time = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->map_time = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->extra = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->background_scrolling = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->player_sprite_index = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->x = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->y = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->icon = atoi(luku);
	memset(luku, 0, sizeof(luku));

	u32 lkm;
	file->read(luku, sizeof(luku));
	lkm = (int)atoi(luku);

	file->read(sprite_filenames, sizeof(sprite_filenames[0]) * lkm);

	u32 width, height;
	u32 offset_x, offset_y;

	// background_tiles
	file->read(luku, sizeof(luku)); offset_x = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); offset_y = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); width   = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); height  = atol(luku); memset(luku, 0, sizeof(luku));
	for (u32 y = offset_y; y <= offset_y + height; y++) {
		u32 x_start = offset_x + y * PK2MAP_MAP_WIDTH;
		file->read(&background_tiles[x_start], width + 1);
	}

	// foreground_tiles
	file->read(luku, sizeof(luku)); offset_x = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); offset_y = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); width   = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); height  = atol(luku); memset(luku, 0, sizeof(luku));
	for (u32 y = offset_y; y <= offset_y + height; y++) {
		u32 x_start = offset_x + y * PK2MAP_MAP_WIDTH;
		file->read(&foreground_tiles[x_start], width + 1);
	}

	//sprite_tiles
	file->read(luku, sizeof(luku)); offset_x = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); offset_y = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); width   = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); height  = atol(luku); memset(luku, 0, sizeof(luku));
	for (u32 y = offset_y; y <= offset_y + height; y++) {
		u32 x_start = offset_x + y * PK2MAP_MAP_WIDTH;
		file->read(&sprite_tiles[x_start], width + 1);
	}

	file->close();

	return 0;
}

int MapClass::Load_BG(PFile::Path path){
	
	if (!FindAsset(&path, "gfx" PE_SEP "scenery" PE_SEP))
		return 1;

	PDraw::image_load(this->background_buffer, path, true);
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

int MapClass::Load_TilesImage(PFile::Path path){
	
	PFile::Path bkp = path;

	if (!FindAsset(&path, "gfx" PE_SEP "tiles" PE_SEP)){
		throw PExcept::FileNotFoundException(path.c_str(), PExcept::MISSING_TILESET);
	}

	PDraw::image_load(this->tiles_buffer, path, false);
	if (this->tiles_buffer == -1){
		throw PExcept::FileNotFoundException(path.c_str(), PExcept::MISSING_TILESET);
	}

	PDraw::image_delete(this->water_buffer); //Delete last water buffer
	this->water_buffer = PDraw::image_cut(this->tiles_buffer,0,416,320,32);

	// load bg buffer
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
	}

	return 0;

}

int MapClass::Load_BGSfx(PFile::Path path){

	return 0;
}

void MapClass::Calculate_Edges(){

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
					//this->background_tiles[x+y*PK2MAP_MAP_WIDTH] = 49; //Debug
				}
			}
		}
}

/* Kartanpiirtorutiineja ----------------------------------------------------------------*/

void MapClass::Animate_Fire(int tiles){
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

void MapClass::Animate_Waterfall(int tiles){
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

void MapClass::Animate_WaterSurface(int tiles){
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

void MapClass::Animate_Water(int tiles, int water_tiles){
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

void MapClass::Animate_RollUp(int tiles){
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

int MapClass::DrawBackgroundTiles(int kamera_x, int kamera_y){
	
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

int MapClass::DrawForegroundTiles(int kamera_x, int kamera_y){

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
