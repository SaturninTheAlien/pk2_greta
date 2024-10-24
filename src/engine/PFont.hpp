//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "PFile.hpp"
#include "platform.hpp"
#include "PString.hpp"


#include <vector>
#include <utility>

class PFont{
private:

	int getCharacterPos(const PString::UTF8_Char& c)const;

	std::vector<std::pair<int, PString::UTF8_Char>> utf8_charlist;


	//int charlist[256];
	uint char_w, char_h, char_count;
	int image_index = -1;

	void initCharlist();
	void initCharlist(const char* letters);
	int get_image(int x,int y,int img_source);

public:
	int write(int posx, int posy, const char *text);
	int write_trasparent(int posx, int posy, const char* text, int alpha);
	
	PFont(int img_source, int x, int y, int width, int height, int count);
	PFont();
	~PFont();

	int load(PFile::Path path);

	bool acceptChar(PString::UTF8_Char u8c)const;
};