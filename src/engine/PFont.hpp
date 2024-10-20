//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/PFile.hpp"
#include "engine/platform.hpp"

#include <vector>
#include <utility>

class PFont{
private:
	class UTF8_Char{
	public:
		char data[5] = {'\0'};
		const char* read(const char*str);

		bool operator==(const UTF8_Char& other)const{
			return *reinterpret_cast<const u32*>(this->data) == *reinterpret_cast<const u32*>(other.data);
		}

		bool operator!=(const UTF8_Char& other)const{
			return *reinterpret_cast<const u32*>(this->data) != *reinterpret_cast<const u32*>(other.data);
		}
	};

	static UTF8_Char lowercase(UTF8_Char src);

	std::vector<std::pair<int, UTF8_Char>> utf8_charlist;


	int charlist[256];
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
};