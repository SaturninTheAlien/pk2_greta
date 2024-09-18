//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include "engine/PFile.hpp"

#include "engine/PLog.hpp"
#include "engine/PUtils.hpp"
#include "engine/platform.hpp"
#include "engine/PString.hpp"

#include <cstring>
#include <sys/stat.h>

#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

#include <SDL.h>
#include <stdexcept>

#include "PZip.hpp"

#ifdef PK2_USE_ZIP
#include <zip.h>
#endif

#ifdef __ANDROID__
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

#define PE_SEP "/"

namespace fs = std::filesystem;

namespace PFile {

bool Path::exists()const{
	if(this->zip_file!=nullptr){
		throw std::runtime_error("Unimplemented Path::exists (zip)");
	}
	else{
		return fs::exists(this->path);
	}
}

Path::Path(std::string path) {

	path = path.substr(0, path.find_last_not_of(" ") + 1);
    this->path = path;
	
	this->zip_file = nullptr;
}

Path::Path(PZip* zip_file, std::string path) {

	if(zip_file == nullptr) {

		PLog::Write(PLog::ERR, "PFile", "No zip file for %s\n", path.c_str());

	}

	path = path.substr(0, path.find_last_not_of(" ") + 1);
    this->path = path;
	
    this->zip_file = zip_file;
}

Path::Path(Path path, std::string file) {

	*this = path;
	
	file = file.substr(0, file.find_last_not_of(" ") + 1);
	this->path += file;
}

Path::~Path() {

}

bool Path::operator==(Path path) { //needed?

	bool a = this->zip_file == path.zip_file; //compare with zip_file name?
	bool b = this->path == path.path;

	return a && b;

}

const char* Get_Extension(const char* string) {

	int len = strlen(string);
	const char* end = string + len;
	
	for( int i = 0; i < len; i++ ) {

		if (*(end - i) == '.' 
			|| *(end - i) == '/'
			|| *(end - i) == '\\') {

			return end - i;

		}

	}

	return string;

}

bool is_type(const char* file, const char* type) {

    if(type[0] == '\0') {

        return true;

    } else if(type[0] == '/' && strstr(file, ".") == NULL) {

        return true;

    } else {

        const char* ext = Get_Extension(file);
        if(strcmp(ext, type) == 0) {

            return true;

        }
    }

    return false;

}

#ifdef PK2_USE_ZIP
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

}

std::vector<std::string> scan_zip(PZip* zip_file, const char* path, const char* type) {

	std::cout<<"Scanning zip: "<<zip_file->getName()<<std::endl;	

    std::vector<std::string> result;

    /*int path_size = strlen(path);

    struct zip_stat st;
    zip_stat_init(&st);
    
    int sz = zip_get_num_entries((zip_t*)zip_file->zip, 0);
    for (int i = 0; i < sz; i++) {
        
        zip_stat_index((zip_t*)zip_file->zip, i, 0, &st);

		std::cout<<i<<" "<<st.crc<<":->"<<st.name<<std::endl;

		if( pathcomp(path, st.name) ) {

			std::string filename(st.name + path_size + 1);
			filename = filename.substr(0, filename.find("/")); //PE_SEP?

			if(filename.size() == 0)
				continue;
			
            if(is_type(filename.c_str(), type)) {

				//needed?
				bool repeated = false;
				for (std::string st : result)
					if (st == filename) {
						repeated = true;
						break;
					}
				
				if (!repeated)
                	result.push_back(filename);

            }

        }

    }

	exit(10);*/

    //PLog::Write(PLog::DEBUG, "PFile", "Scanned zip \"%s\" on \"%s\" for \"%s\". Found %i matches", zip_file->name.c_str(), path, type, (int)result.size());
    return result;

}

#endif

//Scans directory to find file based on case
bool Path::NoCaseFind() {

	std::string filename = this->GetFileName();
	this->SetFile("");

	std::vector<std::string> list = this->scandir("");

	int sz = list.size();
	for(int i = 0; i < sz; i++) {
		
		std::string name = list[i];
		
		if(PUtils::NoCaseCompare(name.c_str(), filename.c_str())) {

			this->SetFile(name);
			PLog::Write(PLog::DEBUG, "PFile", "Found on %s", this->path.c_str());

			return true;
		}

	}

	this->SetFile(filename);
	//PLog::Write(PLog::INFO, "PFile", "%s not found", this->path.c_str());

	return false;

}

#ifdef __ANDROID__
/**
 * Is it necessary?
 * Why can't apk be handled as a regular zip.
 * Perhaps we don't need Java to do it.
 */
std::vector<std::string> scan_apk(const char* dir, const char* type) {

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));
	jmethodID method_id = env->GetMethodID(clazz, "listDir", "(Ljava/lang/String;)[Ljava/lang/String;");

	jstring param = env->NewStringUTF(dir);
	jobjectArray array = (jobjectArray)env->CallObjectMethod(activity, method_id, param);

	jsize size = env->GetArrayLength(array);
	
	std::vector<std::string> result;

	for (int i = 0; i < size; i++) {

		jstring filename = (jstring)env->GetObjectArrayElement(array, i);
		jboolean isCopy;
		const char* file = env->GetStringUTFChars(filename, &isCopy);

		if( file[0] != '.' ) {

			if(type[0] == '/' && strstr(file, ".") == NULL) { //provisory way - consider file without '.' a directory

				result.push_back(file);
				continue;

			} else if(type[0] == '\0') {
			
				result.push_back(file);
				continue;
			
			} else {

				const char* ext = Get_Extension(file);
				if(strcmp(ext, type) == 0) {

					result.push_back(file);
					continue;

				}
			}
		}

		if (isCopy == JNI_TRUE) {
    		env->ReleaseStringUTFChars(filename, file);
		}
	}


	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

    PLog::Write(PLog::DEBUG, "PFile", "Scanned APK on \"%s\" for \"%s\". Found %i matches", dir, type, (int)result.size());

	return result;

}

#endif



std::vector<std::string> scan_file(const char* dir, const char* type){
	
	std::vector<std::string> result;
	for (const auto & entry : fs::directory_iterator(dir)){
		std::string filename = entry.path().filename().string();

		//std::cout<<filename<<std::endl;
		if(filename[0]!='.'){

			if(type[0] == '/' && entry.is_directory()) {

				result.push_back(filename);
				continue;

			} else if(type[0] == '\0') {
			
				result.push_back(filename);
				continue;
			
			} else {

				const char* ext = Get_Extension(filename.c_str());
				if(strcmp(ext, type) == 0) {

					result.push_back(filename);
					continue;

				}

			}

		}
	}
	std::sort(result.begin(), result.end());
	//PLog::Write(PLog::DEBUG, "PFile", "Scanned on \"%s\" for \"%s\". Found %i matches", dir, type, (int)result.size());

	return result;
}


void Path::SetFile(std::string file) {
	this->path = this->GetDirectory() + PE_SEP  + file;
}

std::string Path::GetDirectory()const {
	std::filesystem::path p(this->path);
	return p.parent_path().u8string();

}

std::string Path::GetFileName()const {
	std::filesystem::path p(this->path);
	return p.filename().u8string();
}


void Path::getBuffer(std::vector<char>& bytes)const{
	if(this->zip_file != nullptr){
		#ifdef PK2_USE_ZIP

		int size = 0;
		zip_file_t* zfile = (zip_file_t*)this->zip_file->readFile(this->path, size);
		bytes.resize(size);
		zip_fread(zfile, bytes.data(), size);
		zip_fclose(zfile);	

		#else
		throw PFileException("Zip is not supported in this PK2 version!");
		#endif
	}
	else{
		std::ifstream file(this->path.c_str(), std::ios::binary | std::ios::ate);
		bool success = file.good();
		if(success){
			std::streamsize size = file.tellg();
			file.seekg(0, std::ios::beg);

			bytes.resize(size);
			if (!file.read(bytes.data(), size)){
				success = false;
				bytes.clear();
			}

		}

		if(!success){
			std::ostringstream os;
			os<<"Cannot get raw buffer from the file: \""<<this->path<<"\"";
			std::string s = os.str();
			throw PFileException(s);
		}
	}

}

RW Path::GetRW2(const char* mode)const {
	SDL_RWops* ret;
	if (this->zip_file != nullptr) {

		#ifdef PK2_USE_ZIP
		
		int size = 0;
		zip_file_t* zfile = (zip_file_t*)this->zip_file->readFile(this->path, size);
		void* buffer = SDL_malloc(size);
		zip_fread(zfile, buffer, size);
		zip_fclose(zfile);

		ret = SDL_RWFromConstMem(buffer, size);
		return RW(ret, buffer);

		#else

		throw PFileException("Zip is not supported in this PK2 version!");
		
		#endif
		
	}
	else{
		ret = SDL_RWFromFile(this->path.c_str(), mode);
		if (!ret) {

			std::ostringstream os;
			os<<"Can't get RW from the file: \""<<this->path<<"\"";
			std::string s = os.str();
			throw PFileException(s);
		}

		return RW(ret, nullptr);
	}
}


nlohmann::json Path::GetJSON()const{
	
	if(this->zip_file!=nullptr){

		#ifdef PK2_USE_ZIP

		int size = 0;
		zip_file_t* zfile = (zip_file_t*)this->zip_file->readFile(this->path, size);
		char* buffer = new char[size+1];
		buffer[size] = '\0';
		
		zip_fread(zfile, buffer, size);
		zip_fclose(zfile);

		nlohmann::json res = nlohmann::json::parse(buffer);
		delete[] buffer;
		return res;

		#else

		throw PFileException("Zip is not supported in this PK2 version!");
		
		#endif

	}else{
		std::ifstream in(this->path.c_str());
		nlohmann::json res = nlohmann::json::parse(in);
		return res;
	}
}

std::string Path::GetContentAsString()const{
	if(this->zip_file!=nullptr){
		#ifdef PK2_USE_ZIP

		int size = 0;
		zip_file_t* zfile = (zip_file_t*)this->zip_file->readFile(this->path, size);
		char* buffer = new char[size+1];
		buffer[size] = '\0';
		
		zip_fread(zfile, buffer, size);
		zip_fclose(zfile);

		std::string res = buffer;

		delete[] buffer;
		return res;

		#else

		throw PFileException("Zip is not supported in this PK2 version!");
		
		#endif
	}
	else{
		std::ifstream in(this->path.c_str());
		return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	}
}

RW::RW(RW&& source){
	this->_rwops = source._rwops;
	this->_mem_buffer = source._mem_buffer;

	source._rwops = nullptr;
	source._mem_buffer = nullptr;
}

/*
int RW::read(std::string& str) {

	str.clear();

	while(1) {
		u8 c = SDL_ReadU8((SDL_RWops*)(this->_rwops));

		if (c == '\0')
			return str.size();
		
		str += c;
	
	}

}*/

int RW::read(void* val, size_t size) {

	return SDL_RWread((SDL_RWops*)(this->_rwops), val, 1, size);

}
void RW::read(bool& val) {

	u8 v = SDL_ReadU8((SDL_RWops*)(this->_rwops));
	
	if (v == 0) val = false;
	else val = true;
}
void RW::read(u8& val) {

	val = SDL_ReadU8((SDL_RWops*)(this->_rwops));

}
void RW::read(s8& val) {

	val = SDL_ReadU8((SDL_RWops*)(this->_rwops));
}
void RW::read(u16& val) {

	val = SDL_ReadLE16((SDL_RWops*)(this->_rwops));
}
void RW::read(s16& val) {

	val = SDL_ReadLE16((SDL_RWops*)(this->_rwops));
}
void RW::read(u32& val) {

	val = SDL_ReadLE32((SDL_RWops*)(this->_rwops));

}
void RW::read(s32& val) {

	val = SDL_ReadLE32((SDL_RWops*)(this->_rwops));
}
void RW::read(u64& val) {
	val = SDL_ReadLE64((SDL_RWops*)(this->_rwops));
}
void RW::read(s64& val) {
	val = SDL_ReadLE64((SDL_RWops*)(this->_rwops));
}

void RW::readLegacyStrInt(int&val){
	char buffer[8];
	this->read(buffer, sizeof(buffer));
	buffer[7] = '\0';

	val = atoi(buffer);
}

void RW::readLegacyStrU32(u32& val){
	char buffer[8];
	this->read(buffer, sizeof(buffer));
	buffer[7] = '\0';

	val = (u32)atol(buffer);
}

void RW::readLegacyStr13Chars(std::string & val){
	char buffer[13];
	this->read(buffer, sizeof(buffer));
	buffer[12] = '\0';
	val = buffer;
}

void RW::readLegacyStr40Chars(std::string & val){
	char buffer[40];
	this->read(buffer, sizeof(buffer));
	buffer[39] = '\0';
	val = buffer;
}

/*
int RW::write(std::string& str) {

	return SDL_RWwrite((SDL_RWops*)(this->_rwops), str.c_str(), 1, str.size() + 1);

}*/

int RW::write(const void* val, size_t size) {

	return SDL_RWwrite((SDL_RWops*)(this->_rwops), val, size, 1);

}
void RW::write(bool val) {
	SDL_WriteU8((SDL_RWops*)(this->_rwops), val);

}
void RW::write(u8 val) {
	SDL_WriteU8((SDL_RWops*)(this->_rwops), val);

}
void RW::write(s8 val) {
	SDL_WriteU8((SDL_RWops*)(this->_rwops), val);

}
void RW::write(u16 val) {
	SDL_WriteLE16((SDL_RWops*)(this->_rwops), val);

}
void RW::write(s16 val) {
	SDL_WriteLE16((SDL_RWops*)(this->_rwops), val);

}
void RW::write(u32 val) {
	SDL_WriteLE32((SDL_RWops*)(this->_rwops), val);

}
void RW::write(s32 val) {
	SDL_WriteLE32((SDL_RWops*)(this->_rwops), val);

}
void RW::write(u64 val) {
	SDL_WriteLE64((SDL_RWops*)(this->_rwops), val);

}
void RW::write(s64 val) {
	SDL_WriteLE64((SDL_RWops*)(this->_rwops), val);

}

void RW::writeCBOR(const nlohmann::json& j){
	std::vector<std::uint8_t> v_cbor = nlohmann::json::to_cbor(j);
	u32 size = (u32)v_cbor.size();
	this->write(size);
	this->write(v_cbor.data(), size);
}

nlohmann::json RW::readCBOR(){
	u32 size;
	this->read(size);

	std::vector<std::uint8_t> v_cbor;
	v_cbor.resize(size);

	this->read(v_cbor.data(), size);

	return nlohmann::json::from_cbor(v_cbor);
}

size_t RW::size() {

	SDL_RWops* rwops = (SDL_RWops*)(this->_rwops);

	return SDL_RWsize(rwops);

}
/*
size_t RW::to_buffer(void** buffer) {

	SDL_RWops* rwops = (SDL_RWops*)(this->_rwops);
	
	size_t size = SDL_RWsize(rwops);
	
	if (size == 0) {

		PLog::Write(PLog::ERR, "PFile", "RW size = 0");
		return size;

	}

	*buffer = SDL_malloc(size);

	if (*buffer) {

		this->read(*buffer, size);
		return size;

	} else {

		PLog::Write(PLog::ERR, "PFile", "Could not alloc memory");
		return 0;

	}

}*/

void RW::close() {

	if(this->_rwops!=nullptr){
		SDL_RWops* rwops = (SDL_RWops*)(this->_rwops);
	
		int ret = SDL_RWclose(rwops);
		if (ret != 0) {
		
			PLog::Write(PLog::ERR, "PFile", "Error freeing rw");
		}

		this->_rwops = nullptr;
	}

	if(this->_mem_buffer!=nullptr){
		SDL_free(this->_mem_buffer);
		this->_mem_buffer = nullptr;
	}

}

std::unordered_map<std::string, std::vector<std::string>> scan_cache;
//std::unordered_map<std::string, std::vector<std::string>> scan_cache_zip; //TODO

std::vector<std::string> Path::scandir(const char* type) {
    
	std::vector<std::string> ret;

	std::string dir = this->GetDirectory(); //this->path.substr(0, this->path.find_last_of(PE_SEP));

	if(!this->Is_Zip()){
		if(!fs::exists(dir) || !fs::is_directory(dir)){
			return ret;
		}
	}

	if(dir.empty()){
		dir = ".";
	}

	std::string cache_entry(dir + type);
	
	const char* cstr = dir.c_str();
	
	if (this->zip_file != nullptr) {

    	#ifdef PK2_USE_ZIP
        
		return scan_zip(this->zip_file, cstr, type);
		
		#else
		
		return ret;
		
		#endif

	}
	
	// Look the cache	
	auto it = scan_cache.find(cache_entry);
	if (it != scan_cache.end()) {

		//PLog::Write(PLog::DEBUG, "PFile", "Got cache on \"%s\" for \"%s\"", cstr, type);
		return it->second;

	}

    #ifdef __ANDROID__

    if (cstr[0] != '/') {

		ret = scan_apk(cstr, type);
		scan_cache[cache_entry] = ret;
        return ret;

	}

    #endif
	
	ret = scan_file(cstr, type);
	scan_cache[cache_entry] = ret;
	return ret;

}

};