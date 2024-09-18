//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include <string>
#include <vector>
#include <optional>



namespace PZip{


class PZipException:public std::exception{
public:
    PZipException(const std::string& message):message(message){}
    const char* what() const noexcept{
        return message.c_str();
    }
private:
    std::string message;
};

class PZipEntry{
public:
    PZipEntry()=default;
    PZipEntry(const std::string& name, int index, int size):
    name(name), index(index), size(size){

    }

    std::string name;
    int index=0;
    int size=0;
};


class PZip{
public:
    PZip() = default;

    PZip(const std::string& path){
        this->open(path);
    }


    PZip(const PZip& pzip)=delete;
    PZip(PZip&& pzip);

    PZip& operator=(const PZip& pzip)=delete;

    ~PZip(){
        this->close();
    }

    const std::string& getName()const{
        return this->name;
    }
    
    void open(const std::string& path);
    void close();

    std::optional<PZipEntry> getEntry(const std::string& cAsE_path);


    /*int getIndex(const std::string& filename, int& size, std::string& name);*/
    void* readFile(const std::string& name, int&size);

    std::string name;
    void * zip = nullptr;
    void * src = nullptr;
};

}