//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "widgets.hpp"

#include <sstream>

#include "engine/PDraw.hpp"
#include "engine/PInput.hpp"

#include "system.hpp"
#include "settings/config_txt.hpp"
#include "settings/settings.hpp"
#include "sfx.hpp"
#include "utils/open_browser.hpp"

namespace PK2gui{


bool Draw_BoolBox(int x, int y, bool value, bool active){
		PDraw::RECT img_src, img_dst = {x , y, 0, 0};

	if(value) img_src = {504,124,31,31};
	else img_src = {473,124,31,31};

	if(active){
		PDraw::image_cutclip(global_gfx_texture,img_src,img_dst);
	} else{
		PDraw::image_cutcliptransparent(global_gfx_texture,img_src,img_dst,50, 255);
		return false;
	}

	if (PInput::mouse_x > x && PInput::mouse_x < x+30 && PInput::mouse_y > y && PInput::mouse_y < y+31){
		if (Clicked()){

			Play_MenuSFX(sfx_global.menu_sound, 100);
			key_delay = 20;
			return true;
		}
	}

	return false;
}

TextInput::TextInput(std::size_t max_chars, const std::string& rejected){
	const char * text = rejected.c_str();
	this->buffer.resize(max_chars);
	PString::UTF8_Char u8c;
	while(*text!='\0'){
		text = u8c.read(text);
		this->rejectedChars.emplace_back(u8c);
	}

	this->selectedIndex = 0;
}

bool TextInput::acceptInputChar(PString::UTF8_Char u8c)const{
    if(u8c.isNull())return false;

    for(const PString::UTF8_Char& rejected : this->rejectedChars){
        if(u8c==rejected)return false;
    }

	//To accept space
	if(u8c==PString::UTF8_Char(' '))return true;

    return PDraw::font_accept_char(fontti2, u8c);
}

void TextInput::startInput(){
    PInput::StartKeyboard();
	this->editing = true;
}

void TextInput::endInput(){
    this->editing = false;
	PInput::EndKeyboard();
}


int TextInput::getTextSize()const{
	int n = (int)this->buffer.size();

	while (n>0){
		--n;
		if(this->buffer[n]!=PString::UTF8_Char(' ')){
			return n + 1;
		}
	}
	return 0;
}

bool TextInput::draw(const std::string& title, int tx_start, int ty_start){

	int tx_end = tx_start + 15 *  (int)this->buffer.size();
	int ty_end = ty_start + 18;

	bool mouse_on_text = PInput::mouse_x >= tx_start && PInput::mouse_x <= tx_end && PInput::mouse_y >= ty_start && PInput::mouse_y <= ty_end;

	int nameLength = this->getTextSize();

	if (mouse_on_text && PInput::MouseLeft() && key_delay == 0) {
		this->startInput();
		this->selectedIndex = (PInput::mouse_x - tx_start) / 15; //Set text cursor with the mouse
		key_delay = 10;
		if(this->selectedIndex>nameLength){
			this->selectedIndex = nameLength;
		}

	}

	if(this->selectedIndex >= (int)this->buffer.size()){
		this->selectedIndex = (int)this->buffer.size() - 1;
	}

	if(this->selectedIndex < 0){
		this->selectedIndex = 0;
	}

    PDraw::font_write_line(fontti2, title, tx_start,ty_start - 30);

	PDraw::screen_fill(tx_start-2, ty_start-2, tx_end+4, ty_end+4, 0);
	PDraw::screen_fill(tx_start, ty_start, tx_end, ty_end, mouse_on_text? 54:50);

	if (editing) { //Draw text cursor
		int mx = this->selectedIndex *15 + tx_start + rand() % 2;
		PDraw::screen_fill(mx-2, ty_start, mx+6+3, ty_end+3, 0);
		PDraw::screen_fill(mx-1, ty_start, mx+6, ty_end, 96+16);
		PDraw::screen_fill(mx+4, ty_start, mx+6, ty_end, 96+8);
	}

	int i = 0;

    int x = tx_start;
    int y = ty_start - 1;

	for(const PString::UTF8_Char& u8c: this->buffer){
		int ys = (int)(sin_table((i+degree)*4))/9;
		int xs = (int)(cos_table((i+degree)*4))/11;

		if (Settings.transparent_text) {
		
			x += PDraw::font_writealpha_s(fontti2 ,u8c.c_str(),x-xs,y+ys,75).first;
		
		} else {
			PDraw::font_write_line(fontti4,u8c.c_str(),x-xs+1,y+ys+1);
			x += PDraw::font_write_line(fontti2 ,u8c.c_str(),x-xs,y+ys);
		}
		i+=1;
	}

    x = tx_start;

	for(const PString::UTF8_Char& u8c: this->buffer){
		x += PDraw::font_writealpha_s(fontti3, u8c.c_str(), x, y, 15).first;
	}

    if (this->editing) {

		PString::UTF8_Char u8c = PInput::ReadKeyboardInput();
        if(this->acceptInputChar(u8c)){

			/**
			 * @brief
			 * TODO
			 * Support the "insert" key
			 */
			for(int j = (int)this->buffer.size() - 1; j > this->selectedIndex; j--){
				this->buffer[j] = this->buffer[j-1];
			}

            this->buffer[this->selectedIndex] = u8c;
            ++this->selectedIndex;
        }


		int keyNav = PInput::ReadKeyboardNav();
		if (PInput::Keydown(PInput::JOY_START) && key_delay == 0) {
			this->endInput();
            return true;
		}

		if (keyNav == PInput::DEL) {
			for (int c=this->selectedIndex + 1; c < (int)this->buffer.size(); c++){
                this->buffer[c - 1] = this->buffer[c];
            }
            this->buffer[(int)this->buffer.size() - 1] = ' ';
		}

		if (keyNav == PInput::BACK && this->selectedIndex > 0) {
			for (int c= this->selectedIndex; c < (int)this->buffer.size(); c++){
                this->buffer[c - 1] = this->buffer[c];
            }
            --this->selectedIndex;
		}

		if (keyNav == PInput::RETURN) {
            this->endInput();
            return true;
		}

		if (keyNav == PInput::LEFT) {
            --this->selectedIndex;
			key_delay = 8;
		}

		if (keyNav == PInput::RIGHT) {
			++this->selectedIndex;
			if(this->selectedIndex>nameLength){
				this->selectedIndex = nameLength;
			}
			key_delay = 8;
		}
	}

    return false;
}

void TextInput::clear(){
	//Clear the buffer
	for(PString::UTF8_Char& c : this->buffer){
		c = ' ';
	}
	this->selectedIndex = 0;
}

void TextInput::setText(const std::string& text){
	this->clear();
	const char* text_ptr = text.c_str();
	std::size_t i = 0;
	PString::UTF8_Char u8c;
	while(*text_ptr!='\0' && i < this->buffer.size()){
		text_ptr = u8c.read(text_ptr);
		this->buffer[i] = u8c;
		++i;
	}
    
    this->selectedIndex = i;
}

std::string TextInput::getText()const{
	std::ostringstream builder;
	for(const PString::UTF8_Char& u8c : this->buffer){
		builder << u8c.c_str();
	}
	return PString::lowercase(PString::rtrim(builder.str()));
}

bool LinksMenu::drawButton(int x, int y, const PDraw::RECT& rect, const std::string& label){
	int mouse_x = (int)PInput::mouse_x;
	int mouse_y = (int)PInput::mouse_y;

	bool res = false;

	if(mouse_x > x && mouse_x < x + rect.w &&
	mouse_y > y && mouse_y < y + rect.h){

		int label_x = x - PDraw::font_get_text_size(fontti1, label).first - 5;
		int label_y = y + 10;

		PDraw::font_write_line(fontti1, label, label_x, label_y);

		x += rand()%3 - rand()%3;
		y += rand()%3 - rand()%3;

		if(Clicked()){
			if(this->btnKeyDelay==0){
				res = true;
			}
			this->btnKeyDelay = 10;
		}		
	}

	PDraw::image_cutclip(global_gfx_texture, rect, PDraw::RECT(x, y, 0, 0));
	return res;
}


void LinksMenu::draw(){
	int x = 604;
	int y = 443;

	if(this->btnKeyDelay>0){
		--this->btnKeyDelay;
	}

	if(drawButton(x, y, PDraw::RECT(600, 124, 30, 31), "Links")){
		this->expanded = !this->expanded;
	}

	if(this->expanded){

		y -= 47;
		if(drawButton(x, y, PDraw::RECT(600, 157, 30, 31), "Documentation")){
			OpenBrowser(URL_MAKYUNI);
		}

		y -= 47;
		if(drawButton(x, y, PDraw::RECT(569, 157, 30, 31), "PisteGamez")){
			OpenBrowser(URL_PISTEGAMEZ);
		}

		y -= 47;
		if(drawButton(x, y, PDraw::RECT(537, 157, 30, 31), "Proboards")){
			OpenBrowser(URL_PROBOARDS);
		}

		y -= 47;
		if(drawButton(x, y, PDraw::RECT(505, 157, 30, 31), "GitHub")){
			OpenBrowser(URL_GITHUB);
		}

		y -= 47;
		if(drawButton(x, y, PDraw::RECT(473, 157, 30, 31), "Discord")){
			OpenBrowser(URL_DISCORD);
		}
	}
}

}