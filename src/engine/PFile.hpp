//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "types.hpp"
#include "3rd_party/json.hpp"

#include <vector>
#include <string>
#include <stdexcept>


namespace PFile {

void SetAssetsPath(const std::string& _assetsPath);
void SetDefaultAssetsPath();
void CreateDirectory(const std::string& path);

class PFileException:public std::exception{
public:
    PFileException(const std::string& message):message(message){}
    const char* what() const noexcept{
        return message.c_str();
    }
private:
    std::string message;
};

struct Zip;

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
    int read(bool& val);

    int read(std::string& str);

    // Read the value always in little endian
    int read(u8& val);
    int read(s8& val);
    int read(u16& val);
    int read(s16& val);
    int read(u32& val);
    int read(s32& val);
    int read(u64& val);
    int read(s64& val);

    int write(const void* val, size_t size);
    int write(bool val);

    int write(std::string& str);

    // Write the value always in little endian
    int write(u8 val);
    int write(s8 val);
    int write(u16 val);
    int write(s16 val);
    int write(u32 val);
    int write(s32 val);
    int write(u64 val);
    int write(s64 val);

    void close();

    void * _rwops;
private:
    void * _mem_buffer;

};

class Path {

    public: 

    Path(std::string path);
    Path(Zip* zip_file, std::string path);
    Path(Path path, std::string file);
    ~Path();

    bool operator ==(Path path);
    const char* c_str()const{
        return this->path.c_str();
    }

    const std::string& str()const{
        return this->path;
    }

    //type:
    // ""  - all files and directories
    // "/" - directory
    // ".exe" - *.exe
    std::vector<std::string> scandir(const char* type);

    bool NoCaseFind();
    bool Find();

    bool Is_Zip()const{
        return this->zip_file!=nullptr;
    };
    bool Is_Absolute()const;

    void SetFile(std::string file);
    void SetPath(std::string path);

    void SetSubpath(std::string sub_path);

    void FixSep();

    std::string GetDirectory()const;
    std::string GetFileName()const;

    std::string GetContentAsString()const;

    RW GetRW2(const char* mode)const;
    nlohmann::json GetJSON()const;

    void getBuffer(std::vector<char>& bytes)const;

private:   
    std::string path;
    Zip* zip_file;

};

Zip* OpenZip(std::string path);
void CloseZip(Zip* zp);

}