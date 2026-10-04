//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "types.hpp"
#include "3rd_party/json.hpp"
#include "PZip.hpp"

#include <vector>
#include <string>
#include <stdexcept>
#include <filesystem>

namespace PFile {

class PFileException:public std::exception{
public:
    PFileException(const std::string& message):message(message){}
    const char* what() const noexcept{
        return message.c_str();
    }
private:
    std::string message;
};


class RW {
public:
    RW(void* rwops, void*mem_buffer=nullptr):
    _rwops(rwops), _mem_buffer(mem_buffer){
    }

    RW(const RW& source)=delete;
    RW& operator=(const RW& source)=delete;

    RW(RW&& source);
    ~RW(){
        this->close();
    }

    size_t size();
    //size_t to_buffer(void** buffer);

    int read(void* val, size_t size);

    // Read the value always in little endian
    void read(bool& val);
    void read(u8& val);
    void read(s8& val);
    void read(u16& val);
    void read(s16& val);
    void read(u32& val);
    void read(s32& val);
    void read(u64& val);
    void read(s64& val);

    void readLegacyStrInt(int& val);
    void readLegacyStrU32(u32& val);
    void readLegacyStr13Chars(std::string& val);
    void readLegacyStr40Chars(std::string& val);

    int write(const void* val, size_t size);
    
    // Write the value always in little endian    
    void write(bool val);
    void write(u8 val);
    void write(s8 val);
    void write(u16 val);
    void write(s16 val);
    void write(u32 val);
    void write(s32 val);
    void write(u64 val);
    void write(s64 val);

    nlohmann::json readCBOR();
    void writeCBOR(const nlohmann::json& j);

    void close();

    void * _rwops;
private:
    void * _mem_buffer;

};


class File{
public:
    File(const std::filesystem::path& path):path(path){

    }

    File(PZip::PZip* zip_file, const PZip::PZipEntry&e)
    :path( std::filesystem::u8path(e.name)), zip_file(zip_file), zip_entry(e){
    }
    ~File()=default;

    bool operator ==(const File& other)const;

    const char* c_str()const{
        return this->path.c_str();
    }

    RW getRW(std::string mode)const;
    nlohmann::json readJSON()const;
    std::string readString()const;
    
    std::string extension()const{
        return this->path.extension().string();
    }

#ifdef __ANDROID__
    bool insideAndroidAPK = false;
#endif

private:
    std::filesystem::path path;
    PZip::PZip* zip_file = nullptr;
    PZip::PZipEntry zip_entry;

};

}