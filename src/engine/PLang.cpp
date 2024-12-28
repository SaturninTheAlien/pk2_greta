//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "engine/PLang.hpp"
#include "engine/PLog.hpp"
#include "engine/PString.hpp"

#include <SDL.h>
#include <cstring>
#include <stdlib.h>

enum {
	READ_SKIP,
	READ_TITLE,
	READ_TEXT,
	READ_SPACE
};

const char MARKER_1 = '*',
           MARKER_2 = ':';

const std::string PLang::PLACEHOLDER = ".....";

PLang::PLang() {

}

PLang::PLang(PFile::Path path) {

	Read_File(path);

}

PLang::~PLang(){}

bool PLang::Read_File(PFile::Path path){

	try{
		PFile::RW io = path.GetRW2("r");
		tekstit.clear();
		titles.clear();

		u8 marker;
		int table_index = -1;
		int read = READ_SKIP;

		while(io.read(&marker, 1)) {

			if (marker == '\r' || marker == '\n') {
				read = READ_SKIP;
				continue;
			}

			switch (read) {
				case READ_SKIP:
					if (marker == MARKER_1) {
						read = READ_TITLE;
						tekstit.push_back("");
						titles.push_back("");
						table_index++;
					} 
					break;
				
				case READ_TITLE:
					if (marker == MARKER_2)
						read = READ_SPACE;
					else
						titles[table_index] += marker;
					break;
				
				case READ_SPACE:
					if (marker == ' ' || marker == '\t')
						break;
					read = READ_TEXT;
					
				case READ_TEXT:
					tekstit[table_index] += marker;
					break;

			}
		}
		io.close();
	}
	catch(const PFile::PFileException& e){
		PLog::Write(PLog::ERR, "Plang", e.what());
		return false;
	}	

	loaded = true;
	return true;
}

int PLang::Search_Id(const std::string& title)const {

	if (!loaded)
		return -1;

	size_t i;
	for (i = 0; i < titles.size(); i++)
		if (titles[i] == title) break;

	if (i >= titles.size())
		return -1;

	return i;

}



const std::string& PLang::Get_Text(int index)const {

	if (!loaded)
		return PLACEHOLDER;

	if (index>=0 && index < (int)tekstit.size())
		return tekstit[index];
	else
		return PLACEHOLDER;
}

int PLang::Set_Text(const std::string& title, const char* text) {

	if (!loaded)
		return -1;

	int idx = this->Search_Id(title);
	if (idx != -1) {
	
		tekstit[idx] = text;
		return idx;
	
	}

	titles.push_back(title);
	tekstit.push_back(text);
	return titles.size() - 1;

}

const std::string& PLang::getString(int id, const std::string& def)const{
	if(id>=0 && id < (int)this->tekstit.size()){
		return this->tekstit[id];
	}

	return def;
}

bool PLang::getBoolean(int id, bool def)const{
	if(id >= 0 && id < (int)this->tekstit.size()){
		const std::string& text = PString::lowercase(this->tekstit[id]);
		if(text=="default")return def;
		else if(text=="true")return true;
		else if(text=="false")return false;
		else if(text=="yes")return true;
		else if(text=="no")return false;
	}

	return def;
}

int PLang::getInteger(int id, int def)const{
	if(id >= 0 && id < (int)this->tekstit.size()){
		int tmp = 0;
		const char* src = this->tekstit[id].c_str();
		if(sscanf(src, "%i", &tmp)==1){
			return tmp;
		}
	}
	return def;
}