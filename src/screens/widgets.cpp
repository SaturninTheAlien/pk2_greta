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


bool Draw_BoolBox(int x, int y, bool value, bool active, bool mouseClicked){
		PDraw::RECT img_src, img_dst = {x , y, 0, 0};

	if(value) img_src = {504,124,31,31};
	else img_src = {473,124,31,31};

	if(active){
		PDraw::image_cutclip(global_gfx_texture,img_src,img_dst);
	} else{
		PDraw::image_cutcliptransparent(global_gfx_texture,img_src,img_dst,50, 255);
		return false;
	}


	const Point2D& mousePos = PInput::InputSystem::instance().getMousePos();

	if (mouseClicked && mousePos.x > x && mousePos.x < x+30 && mousePos.y > y && mousePos.y < y+31){
		Play_MenuSFX(sfx_global.menu_sound, 100);
		return true;
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
	PInput::InputSystem::instance().startTextInput();
	this->editing = true;
}

void TextInput::endInput(){
    this->editing = false;
	PInput::InputSystem::instance().stopTextInput();
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

bool TextInput::updateNavigation(const KeyNav::Nav& nav){
	if(!this->isEditing()) return false;

	switch (nav)
	{
	case KeyNav::ACCEPT:
		this->endInput();
		return true;

	case KeyNav::DELETE:{
		this->eraseAt(this->selectedIndex);
		return true;
	}

	case KeyNav::BACKSPACE:{

		if(this->selectedIndex > 0){
			this->eraseAt(this->selectedIndex - 1);
			--this->selectedIndex;
		}
		return true;
	}

	case KeyNav::LEFT:{
		if(this->selectedIndex > 0){
			--this->selectedIndex;
		}
		return true;
	}
	case KeyNav::RIGHT:{
		if(this->selectedIndex < this->getTextSize()){
			++this->selectedIndex;
		}

		return true;
	}
	default:
		break;
	}

	return false;
}

/**
 * Removes the character at index and shifts the rest left.
 */
void TextInput::eraseAt(int index) {
	int size = static_cast<int>(this->buffer.size());
	if (index < 0 || index >= size) {
		return;
	}

	for (int c = index + 1; c < size; ++c) {
		this->buffer[c - 1] = this->buffer[c];
	}
	this->buffer[size - 1] = ' ';
}

/**
 * Types one character at the cursor. In insert mode the rest of the text moves
 * right (nothing is typed once the field is full, so no character is pushed
 * off the end); in overwrite mode it replaces the character under the cursor.
 */
void TextInput::typeChar(PString::UTF8_Char c) {
	if (!this->acceptInputChar(c)) {
		return;
	}

	int capacity = static_cast<int>(this->buffer.size());
	if (this->selectedIndex >= capacity) {
		return;
	}

	if (!this->overwrite) {
		if (this->getTextSize() >= capacity) {
			return;
		}

		for (int j = capacity - 1; j > this->selectedIndex; --j) {
			this->buffer[j] = this->buffer[j - 1];
		}
	}

	this->buffer[this->selectedIndex] = c;
	++this->selectedIndex;
}

// Ctrl+Left / Ctrl+Backspace: start of the word before index
int TextInput::previousWordStart(int index) const {
	while (index > 0 && this->buffer[index - 1] == PString::UTF8_Char(' ')) {
		--index;
	}
	while (index > 0 && this->buffer[index - 1] != PString::UTF8_Char(' ')) {
		--index;
	}
	return index;
}

// Ctrl+Right / Ctrl+Delete: start of the next word after index
int TextInput::nextWordEnd(int index) const {
	int size = this->getTextSize();
	while (index < size && this->buffer[index] != PString::UTF8_Char(' ')) {
		++index;
	}
	while (index < size && this->buffer[index] == PString::UTF8_Char(' ')) {
		++index;
	}
	return index;
}

/**
 * Applies everything typed since the last frame, in order, so fast typing and
 * held keys behave as in any other program (OS key repeat included).
 */
void TextInput::processTextEvents() {
	PInput::InputSystem& input = PInput::InputSystem::instance();
	PInput::TextEvent event;

	while (this->editing && input.pollTextEvent(event)) {

		if (event.type == PInput::TextEvent::Type::CHAR) {
			this->typeChar(event.ch);
			continue;
		}

		bool ctrl = (event.mod & SDL_KMOD_CTRL) != 0;

		switch (event.scancode) {
		case SDL_SCANCODE_BACKSPACE: {
			int from = ctrl ? this->previousWordStart(this->selectedIndex) : this->selectedIndex - 1;
			if (from < 0) {
				break;
			}
			for (int n = this->selectedIndex - from; n > 0; --n) {
				this->eraseAt(from);
			}
			this->selectedIndex = from;
			break;
		}

		case SDL_SCANCODE_DELETE: {
			int to = ctrl ? this->nextWordEnd(this->selectedIndex) : this->selectedIndex + 1;
			for (int n = to - this->selectedIndex; n > 0; --n) {
				this->eraseAt(this->selectedIndex);
			}
			break;
		}

		case SDL_SCANCODE_LEFT:
			if (ctrl) {
				this->selectedIndex = this->previousWordStart(this->selectedIndex);
			} else if (this->selectedIndex > 0) {
				--this->selectedIndex;
			}
			break;

		case SDL_SCANCODE_RIGHT:
			if (ctrl) {
				this->selectedIndex = this->nextWordEnd(this->selectedIndex);
			} else if (this->selectedIndex < this->getTextSize()) {
				++this->selectedIndex;
			}
			break;

		case SDL_SCANCODE_HOME:
			this->selectedIndex = 0;
			break;

		case SDL_SCANCODE_END:
			this->selectedIndex = this->getTextSize();
			break;

		case SDL_SCANCODE_INSERT:
			if ((event.mod & (SDL_KMOD_CTRL | SDL_KMOD_SHIFT | SDL_KMOD_ALT)) == 0) {
				this->overwrite = !this->overwrite;
			}
			break;

		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_KP_ENTER:
			this->endInput();
			break;

		default:
			break;
		}
	}
}

bool TextInput::draw(const std::string& title, int tx_start, int ty_start, bool mouseClicked){

	int tx_end = tx_start + 15 *  (int)this->buffer.size();
	int ty_end = ty_start + 18;


	const Point2D& mousePos = PInput::InputSystem::instance().getMousePos();

	bool mouse_on_text = mousePos.x >= tx_start && mousePos.x <= tx_end && mousePos.y >= ty_start && mousePos.y <= ty_end;

	int nameLength = this->getTextSize();

	if (mouse_on_text && mouseClicked) {
		this->startInput();
		// Set the text cursor with the mouse, at the nearest gap between characters
		this->selectedIndex = (static_cast<int>(mousePos.x) - tx_start + 7) / 15;
		if(this->selectedIndex>nameLength){
			this->selectedIndex = nameLength;
		}

	}

	this->processTextEvents();

	// The cursor may sit after the last character, even when the field is full
	int capacity = static_cast<int>(this->buffer.size());
	if (this->selectedIndex > capacity) {
		this->selectedIndex = capacity;
	}

	if(this->selectedIndex < 0){
		this->selectedIndex = 0;
	}

    PDraw::font_write_line(fontti2, title, tx_start,ty_start - 30);

	PDraw::screen_fill(tx_start-2, ty_start-2, tx_end+4, ty_end+4, 0);
	PDraw::screen_fill(tx_start, ty_start, tx_end, ty_end, mouse_on_text? 54:50);

	int cursorX = this->selectedIndex * 15 + tx_start + rand() % 2;
	bool overwriteCell = this->overwrite && this->selectedIndex < capacity;

	if (this->editing && overwriteCell) { // Overwrite mode: a block behind the character
		PDraw::screen_fill(cursorX - 2, ty_start, cursorX + 15 + 2, ty_end + 3, 0);
		PDraw::screen_fill(cursorX - 1, ty_start, cursorX + 15, ty_end, 96 + 16);
		PDraw::screen_fill(cursorX + 12, ty_start, cursorX + 15, ty_end, 96 + 8);
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

	// Text cursor, drawn over the text so it stays visible between characters
	if (this->editing) {
		if (overwriteCell) { // underline the character that will be replaced
			PDraw::screen_fill(cursorX - 1, ty_end - 3, cursorX + 15, ty_end + 1, 0);
			PDraw::screen_fill(cursorX, ty_end - 2, cursorX + 14, ty_end, 96 + 16);
		} else { // insert mode: a bar in the gap before the character
			PDraw::screen_fill(cursorX - 3, ty_start - 2, cursorX + 2, ty_end + 3, 0);
			PDraw::screen_fill(cursorX - 2, ty_start - 1, cursorX + 1, ty_end + 2, 96 + 16);
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

/**
 * True for characters that show nothing: whitespace, control characters and
 * the invisible Unicode ones (no-break and wide spaces, zero-width characters,
 * soft hyphen, BOM...). Trailing ones are trimmed from the typed text.
 */
static bool isBlankChar(const PString::UTF8_Char& u8c) {
	std::optional<char32_t> codePoint = u8c.codePoint();
	if (!codePoint) {
		return true; // empty or not valid UTF-8
	}

	char32_t cp = *codePoint;
	return cp <= 0x20                       // null, controls, space
		|| (cp >= 0x7F && cp <= 0xA0)       // DEL, C1 controls, no-break space
		|| cp == 0xAD                       // soft hyphen
		|| cp == 0x34F                      // combining grapheme joiner
		|| cp == 0x115F || cp == 0x1160     // Hangul fillers
		|| cp == 0x1680 || cp == 0x180E     // Ogham space, Mongolian vowel separator
		|| (cp >= 0x2000 && cp <= 0x200F)   // wide spaces, zero-width chars, direction marks
		|| (cp >= 0x2028 && cp <= 0x202F)   // line/paragraph separators, embeddings, narrow nbsp
		|| (cp >= 0x205F && cp <= 0x206F)   // math space, word joiner, invisible operators
		|| cp == 0x3000 || cp == 0x3164     // ideographic space, Hangul filler
		|| cp == 0xFEFF || cp == 0xFFA0;    // BOM / zero-width nbsp, halfwidth Hangul filler
}

std::string TextInput::getText() const {
	std::size_t end = this->buffer.size();
	while (end > 0 && isBlankChar(this->buffer[end - 1])) {
		--end;
	}

	std::ostringstream builder;
	for (std::size_t i = 0; i < end; ++i) {
		builder << this->buffer[i].c_str();
	}
	return PString::lowercase(builder.str());
}

bool LinksMenu::drawButton(int x, int y, const PDraw::RECT& rect, const std::string& label, bool mouseClicked){


	const Point2D& mousePos = PInput::InputSystem::instance().getMousePos();
	int mouse_x = (int)mousePos.x;
	int mouse_y = (int)mousePos.y;


	if(mouseClicked && mouse_x > x && mouse_x < x + rect.w &&
	mouse_y > y && mouse_y < y + rect.h){

		int label_x = x - PDraw::font_get_text_size(fontti1, label).first - 5;
		int label_y = y + 10;

		PDraw::font_write_line(fontti1, label, label_x, label_y);

		x += rand()%3 - rand()%3;
		y += rand()%3 - rand()%3;
		return true;
	}

	PDraw::image_cutclip(global_gfx_texture, rect, PDraw::RECT(x, y, 0, 0));
	return false;
}


void LinksMenu::draw(bool mouseClicked){
	int x = 604;
	int y = 443;

	if(drawButton(x, y, PDraw::RECT(597, 124, 30, 31), "Links", mouseClicked)){
		this->expanded = !this->expanded;
	}

	if(this->expanded){

		y -= 47;
		if(drawButton(x, y, PDraw::RECT(597, 157, 30, 31), "PK2 Fanpage", mouseClicked)){
			OpenBrowser(URL_MAKYUNI);
		}

		y -= 47;
		if(drawButton(x, y, PDraw::RECT(566, 157, 30, 31), "PisteGamez", mouseClicked)){
			OpenBrowser(URL_PISTEGAMEZ);
		}

		y -= 47;
		if(drawButton(x, y, PDraw::RECT(535, 157, 30, 31), "Proboards", mouseClicked)){
			OpenBrowser(URL_PROBOARDS);
		}

		y -= 47;
		if(drawButton(x, y, PDraw::RECT(504, 157, 30, 31), "GitHub", mouseClicked)){
			OpenBrowser(URL_GITHUB);
		}

		y -= 47;
		if(drawButton(x, y, PDraw::RECT(473, 157, 30, 31), "Discord", mouseClicked)){
			OpenBrowser(URL_DISCORD);
		}
	}
}

}