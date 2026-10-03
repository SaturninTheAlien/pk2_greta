//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include <sstream>

#include <algorithm>
#include <filesystem>
#include <cstring>
#include <fstream>
#include <stdexcept>

#ifdef __ANDROID__
#include <jni.h>
#include <functional>
#endif

#include "PFile.hpp"

#include "PLog.hpp"
#include "types.hpp"
#include "PString.hpp"



namespace fs = std::filesystem;

namespace PFile {


#ifdef __ANDROID__

static void getAndroidAsset(const std::string& name, const std::function<void(jbyte*, int)>& func){
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();

    jclass clazz(env->GetObjectClass(activity));
    jmethodID method_id = env->GetMethodID(clazz, "getPK2Asset", "(Ljava/lang/String;)[B");
    /*if(method_id==nullptr){
        throw std::runtime_error("JNI: Method \"getPK2Asset\" not found!");
    }*/

    jstring param = env->NewStringUTF(name.c_str());
    jbyteArray byteArray = (jbyteArray)env->CallObjectMethod(activity, method_id, param);
    
    /**
     * No longer needed
     */
    env->DeleteLocalRef(param);
    //env->DeleteLocalRef(clazz);
    //env->DeleteLocalRef(activity);
    

    if(byteArray==nullptr){
        std::ostringstream os;
        os<<"APK asset \""<<name<<"\" not found!";
        throw std::runtime_error(os.str());
    }

    int len = env->GetArrayLength(byteArray);
    jbyte* data = env->GetByteArrayElements(byteArray, NULL);
    if(data==nullptr){
        env->DeleteLocalRef(byteArray);
        throw std::runtime_error("JNI: env->GetByteArrayElements failed!");
    }

    func(data, len);
    env->ReleaseByteArrayElements(byteArray, data, JNI_ABORT);
    env->DeleteLocalRef(byteArray);
}

#endif


bool File::operator==(const File& second)const {

#ifdef __ANDROID__
	if(this->insideAndroidAPK!=second.insideAndroidAPK){
		return false;
	}
#endif

    if(this->zip_file!=nullptr || second.zip_file!=nullptr){
		return this->zip_file == second.zip_file && this->zip_entry == second.zip_entry;
	}
	else{
		return this->path == second.path;
	}
}

RW File::getRW(std::string mode)const {

	SDL_IOStream* ret = nullptr;

#ifdef __ANDROID__
	if(this->insideAndroidAPK){
		void * buffer = nullptr;
		int size = 0;

		getAndroidAsset(this->path.string(), [&](jbyte* data_j, int size_j){
			buffer = SDL_malloc(size_j);
			memcpy(buffer, data_j, size_j);
			size = size_j;
		});

		ret = SDL_IOFromConstMem(buffer, size);
		return RW(ret, buffer);
	}
#endif

	if (this->zip_file != nullptr && this->zip_entry.good()) {
		void * buffer = SDL_malloc(this->zip_entry.size);
		this->zip_file->read(this->zip_entry, buffer);
		ret = SDL_IOFromConstMem(buffer, this->zip_entry.size);
		return RW(ret, buffer);
	}
	else{
		if(!PString::endsWith(mode, "b")){
			mode+="b";
		}

		ret = SDL_IOFromFile(this->path.c_str(), mode.c_str());
		if (!ret) {

			std::ostringstream os;
			os<<"Can't get RW from the file: "<<this->path;
			std::string s = os.str();
			throw PFileException(s);
		}

		return RW(ret, nullptr);
	}
}


nlohmann::json File::readJSON()const{

#ifdef __ANDROID__
	if(this->insideAndroidAPK){
		char * buffer = nullptr;

		getAndroidAsset(this->path, [&](jbyte* data_j, int size){
			buffer = new char[size + 1];
			memcpy(buffer, data_j, size);
			buffer[size] = '\0';
		});

		nlohmann::json res = nlohmann::json::parse(buffer);
		delete[] buffer;
		return res;
	}
#endif
	
	if(this->zip_file!=nullptr && this->zip_entry.good()){

		char * buffer = new char[this->zip_entry.size + 1];
		buffer[this->zip_entry.size] = '\0';

		this->zip_file->read(this->zip_entry, buffer);

		nlohmann::json res = nlohmann::json::parse(buffer);
		delete[] buffer;
		return res;

	}else{
		std::ifstream in(this->path.c_str());
		nlohmann::json res = nlohmann::json::parse(in);
		return res;
	}
}


std::string File::readString()const{
	
#ifdef __ANDROID__
	if(this->insideAndroidAPK){
		char * buffer = nullptr;

		getAndroidAsset(this->path, [&](jbyte* data_j, int size){
			buffer = new char[size + 1];
			memcpy(buffer, data_j, size);
			buffer[size] = '\0';
		});

		std::string res = buffer;
		delete[] buffer;
		return res;
	}
#endif
	if(this->zip_file!=nullptr){
		char * buffer = new char[this->zip_entry.size + 1];
		buffer[this->zip_entry.size] = '\0';

		this->zip_file->read(this->zip_entry, buffer);

		std::string res = buffer;
		delete[] buffer;
		return res;
	}
	else{
		std::ifstream in(this->path.c_str());
		return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	}
}



RW::RW(RW&& source)
    : io(source.io),
      _mem_buffer(source._mem_buffer)
{
    source.io = nullptr;
    source._mem_buffer = nullptr;
}

std::size_t RW::read(void* val, size_t size) {

	return SDL_ReadIO(this->io, val, size);

}


static void ioFailed(){

	std::ostringstream os;
	os<<"SDL_Read error: "<<SDL_GetError();
	throw PFile::PFileException(os.str());
}

void RW::read(bool& val) {

	u8 v = 0;
	if(!SDL_ReadU8(this->io, &v)){
		ioFailed();
	}
	
	if (v == 0) val = false;
	else val = true;
}
void RW::read(u8& val) {

	if(!SDL_ReadU8(this->io, &val)){
		ioFailed();
	}
}
void RW::read(s8& val) {
	if(!SDL_ReadS8(this->io, &val)){
		ioFailed();
	}
}
void RW::read(u16& val) {
	if(!SDL_ReadU16LE(this->io, &val)){
		ioFailed();
	}
}
void RW::read(s16& val) {
	if(!SDL_ReadS16LE(this->io, &val)){
		ioFailed();
	}
}
void RW::read(u32& val) {
	if(!SDL_ReadU32LE(this->io, &val)){
		ioFailed();
	}
}
void RW::read(s32& val) {
	if(!SDL_ReadS32LE(this->io, &val)){
		ioFailed();
	}
}
void RW::read(u64& val) {
	if(!SDL_ReadU64LE(this->io, &val)){
		ioFailed();
	}
}
void RW::read(s64& val) {
	if(!SDL_ReadS64LE(this->io, &val)){
		ioFailed();
	}
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

	return SDL_RWwrite((SDL_IOStream*)(this->_rwops), str.c_str(), 1, str.size() + 1);

}*/

std::size_t RW::write(const void* val, size_t size) {
	return SDL_WriteIO(this->io, val, size);
}
void RW::write(bool val) {
	if(!SDL_WriteU8(this->io, val)){
		ioFailed();
	}
}
void RW::write(u8 val) {
	if(!SDL_WriteU8(this->io, val)){
		ioFailed();
	}
}
void RW::write(s8 val) {
	if(!SDL_WriteS8(this->io, val)){
		ioFailed();
	}

}
void RW::write(u16 val) {
	if(!SDL_WriteU16LE(this->io, val)){
		ioFailed();
	}
}
void RW::write(s16 val) {
	if(!SDL_WriteS16LE(this->io, val)){
		ioFailed();
	}
}
void RW::write(u32 val) {
	if(!SDL_WriteU32LE(this->io, val)){
		ioFailed();
	}
}
void RW::write(s32 val) {
	if(!SDL_WriteS32LE(this->io, val)){
		ioFailed();
	}
}
void RW::write(u64 val) {
	if(!SDL_WriteU64LE(this->io, val)){
		ioFailed();
	}
}
void RW::write(s64 val) {
	if(!SDL_WriteS64LE(this->io, val)){
		ioFailed();
	}
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
	return SDL_GetIOSize(this->io);

}

void RW::close() {

	if(this->io!=nullptr){

		if(!SDL_CloseIO(this->io)){
			PLog::Write(PLog::ERR, "PFile", "Error freeing rw");
		}
		this->io = nullptr;
	}

	if(this->_mem_buffer!=nullptr){
		SDL_free(this->_mem_buffer);
		this->_mem_buffer = nullptr;
	}

}


};