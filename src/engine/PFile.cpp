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

Path::Path(PZip::PZip* zip_file, std::string path) {

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


};