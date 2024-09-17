//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "PZip.hpp"
#include "PFile.hpp"
#include "PString.hpp"

#include <SDL.h>
#include <zip.h>
#include <sstream>
#include <filesystem>

namespace PFile{

PZip::PZip(PZip&& pzip)
:name(std::move(pzip.name)), zip(pzip.zip), src(pzip.src){
    pzip.zip = nullptr;
    pzip.src = nullptr;
}

void PZip::open(const std::string& path){
	SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "r");
	if (rw == NULL) {
        std::ostringstream os;
        os<<"Can't open \""<<path<<"\"";       
        throw PFileException(os.str());
    }

	int size = SDL_RWsize(rw);

	void* buffer = malloc(size);

	SDL_RWread(rw, buffer, size, 1);
	SDL_RWclose(rw);

	zip_error err;

    /**
     * @brief 
     * There's no need to free(buffer) because zip_source_buffer_create does it.
     */
	zip_source_t* src = zip_source_buffer_create(buffer, size, 1, &err);

    zip_t* zip = zip_open_from_source(src, ZIP_RDONLY, &err);

    this->name = std::filesystem::path(path).filename().string();


    this->src = src;
    this->zip = zip;
}

void PZip::close(){
    if(this->src!=nullptr){
        zip_source_close((zip_source_t*)this->src);
        this->src = nullptr;
    }

	if(this->zip!=nullptr){
        zip_discard((zip_t*)this->zip);
        this->zip = nullptr;
    }

    this->name = "";
}

int PZip::getIndex(const std::string& filename, int& size) {

    std::string filename_lowercase = PString::rtrim(PString::lowercase(filename));
   

	struct zip_stat st;
    zip_stat_init(&st);

	int sz = zip_get_num_entries((zip_t*)this->zip , 0);

    for (int i = 0; i < sz; ++i) {

		zip_stat_index((zip_t*)this->zip, i, 0, &st);
        if(filename_lowercase == PString::lowercase(st.name)){
            size = st.size;
			return i;
        }
	}

    size = 0;
	return -1;
}

void* PZip::readFile(const std::string& path, int&size){

    int index = this->getIndex(path, size);;
	if (index < 0 || size<=0) {

		std::ostringstream os;
		os<<"Can't find the file: \""<<path<<"\" in zip \""<<this->name<<"\"";
		throw PFileException(os.str());
	}

	zip_file_t* zfile = zip_fopen_index((zip_t*)this->zip, index, 0);

	if (!zfile) {

		std::ostringstream os;
		os<<"Zfile from zip \""<<this->name<<"\", file \""<<
		path<<"\" is NULL";
		throw PFileException(os.str());
	}
	return zfile;
}


bool PZip::findFile(const std::string& dir,
const std::string& name_cAsE,
std::string& res,
const std::string& alt_extension){

	
	
	return false;
}

/*
static bool pathcomp(const char* path, const char* entry) {

	while(*path != '\0') {

		char a = *path;
		char b = *entry;

		if (a == '\\') a = '/';
		if (b == '\\') b = '/';

		if ( (a | ' ') != (b | ' ') )
			return false;
		
		path++;
		entry++;

	}

	return true;

}*/

}