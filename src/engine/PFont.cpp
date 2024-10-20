//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include <iostream>

#include "engine/PFont.hpp"

#include "engine/PDraw.hpp"
#include "engine/PLang.hpp"
#include "engine/platform.hpp"
#include "engine/PFilesystem.hpp"

#include <cmath>
#include <cstring>


static int getBytesNumber(char c){

    //ASCII ch
    if((c & 0x80)==0){
        return 1;
    }
    // 11110000

    //3 following bytes
    else if((c & 0xF0)==0xF0){
        return 4;
    }

    //2 following bytes
    else if((c & 0xE0)==0xE0){
        return 3;
    }

    //1 following bytes
    else if((c & 0xC0)==0xC0){
        return 2;
    }


    return 0;
}

const char* PFont::UTF8_Char::read(const char *str){
	for(int i=0;i<4;++i){
		this->data[i] = '\0';
	}

	int bytes = getBytesNumber(*str);
	for(int i=0;i<bytes;++i){
		if(*str=='\0')return str;

		this->data[i] = *str;
		++str;
	}

	return str;
}

PFont::UTF8_Char PFont::lowercase(UTF8_Char src){
	if(getBytesNumber(*src.data)==1){
		*src.data = std::tolower(*src.data);
	}
	return src;
}

void PFont::initCharlist() {

	const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "###" "0123456789.!?:-.+=()/#\\_%🌜";
	this->initCharlist(chars);
}


void PFont::initCharlist(const char* letters){

	for ( uint i = 0; i < 256; i++ )
		charlist[i] = -1;
	
	for ( uint i = 0; i < strlen(letters); i++ ){
		charlist[std::tolower(letters[i])] = i * char_w;
		charlist[std::toupper(letters[i])] = i * char_w;
	}

	this->utf8_charlist.clear();

	int index_counter = 0;
	const char * str = letters;
	UTF8_Char u8c;

	while(*str!='\0'){
		str = u8c.read(str);
		this->utf8_charlist.emplace_back(std::make_pair(index_counter, u8c));

		UTF8_Char lower = lowercase(u8c);
		if(u8c!=lower){
			//std::cout<<"Lowering \" "<<u8c.data<<" to "<<lower.data<<std::endl;

			this->utf8_charlist.emplace_back(std::make_pair(index_counter,  lower));
		}

		++index_counter;
	}
}

int PFont::get_image(int x, int y, int img_source) {

	image_index = PDraw::image_cut(img_source, x, y, char_w * char_count, char_h * char_count);
	return 0;

}

int PFont::load(PFile::Path path) {

	int i = 0;

	PLang param_file = PLang();

	if (!param_file.Read_File(path))
		return -1;

	//i = param_file.Search_Id("image width");
	//int buf_width = atoi(param_file.Get_Text(i));

	i = param_file.Search_Id("image x");
	int buf_x = atoi(param_file.Get_Text(i));

	i = param_file.Search_Id("image y");
	int buf_y = atoi(param_file.Get_Text(i));

	/*i = param_file.Search_Id("letters");
	this->char_count = strlen(param_file.Get_Text(i));*/

	i = param_file.Search_Id("letter width");
	this->char_w = atoi(param_file.Get_Text(i));

	i = param_file.Search_Id("letter height");
	this->char_h = atoi(param_file.Get_Text(i));

	/*i = param_file.Search_Id("letters");
	this->initCharlist(param_file.Get_Text(i));*/

	this->initCharlist();

	i = param_file.Search_Id("image");

	std::optional<PFile::Path> imagePath = PFilesystem::FindVanillaAsset(param_file.Get_Text(i), PFilesystem::FONTS_DIR);
	if (!imagePath.has_value()){
		//TODO Add exception here
		return -1;
	}
		

	int temp_image = PDraw::image_load(*imagePath, true);
	if (temp_image == -1) return -1;

	this->get_image(buf_x, buf_y, temp_image);
	PDraw::image_delete(temp_image);

	

	return 0;

}

int PFont::write(int posx, int posy, const char *text) {
	
	int ox = posx;
	
	PDraw::RECT srcrect, dstrect;
	srcrect.y = 0;
	srcrect.w = char_w;
	srcrect.h = char_h;
	dstrect.y = posy;

	dstrect.w = char_w;
	dstrect.h = char_h;

	int charsNumber = 0;
	const char* curr_char = text;
	UTF8_Char u8c;
	while (*curr_char!='\0'){
		curr_char = u8c.read(curr_char);

		for(const std::pair<int, UTF8_Char>& p: this->utf8_charlist){
			if(p.second==u8c){
				srcrect.x = p.first * this->char_w;
				dstrect.x = ox;

				PDraw::image_cutclip(image_index,srcrect,dstrect);
				break;
			}
		}

		ox += char_w;
		charsNumber+=1;
	}
	return char_w * charsNumber;
}

int PFont::write_trasparent(int posx, int posy, const char* text, int alpha) {

	u8 *back_buffer, *txt_buffer;
	u32 back_w, txt_w;

	uint i = 0;
	u8 color1, color2, color3;
	char curr_char;

	int w, h;
	PDraw::get_buffer_size(&w, &h);

	if (alpha > 100) alpha = 100;
	if (alpha < 0) alpha = 0;
	int a1 = alpha;
	int a2 = 100 - alpha;

	PDraw::drawscreen_start(back_buffer, back_w);
	PDraw::drawimage_start(image_index, txt_buffer, txt_w);

	do {
		curr_char = text[i];
		int ix = charlist[(u8)(curr_char)];
		if (ix > -1){
			for (uint x = 0; x < char_w; x++) {
				
				int fx = posx + x + i * char_w;
				if(fx < 0 || fx >= w) break;

				for (uint y = 0; y < char_h; y++) {
					
					int fy = posy + y;
					if (fy < 0 || fy >= h) break;


					color1 = txt_buffer[ix + x + y * txt_w];

					if (color1 != 255) {
					
						// Mix colors
						color1 &= (u8)0b00011111;
						color2 = back_buffer[fx + fy * back_w];
						color3 = color2 & (u8)0b11100000;
						color2-= color3;
						color1 = (color1 * a1 + color2 * a2)/100;

						back_buffer[fx + fy * back_w] = color1 + color3;

					}
				}
			}
		}
		i++;
	
	} while(curr_char != '\0');

	PDraw::drawscreen_end();
	PDraw::drawimage_end(image_index);

	return( (i-1) * char_w );
}

PFont::PFont(int img_source, int x, int y, int width, int height, int count) {

	char_w = width;
	char_h = height;
	char_count = count;

	this->get_image(x, y, img_source);
	this->initCharlist();

}

PFont::PFont() {

	char_w = 0;
	char_h = 0;
	char_count = 0;

}

PFont::~PFont() {

	if(image_index != -1)
		PDraw::image_delete(image_index);

}
