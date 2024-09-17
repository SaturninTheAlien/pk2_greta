//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include <string>
#include <vector>

namespace PFile{

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
    
    void* readFile(const std::string& name, int&size);
    //std::vector<std::string> scan(const std::string& path, const std::string& type);
    
//private:
    int getIndex(const std::string& filename, int& size);

    std::string name;
    void * zip = nullptr;
    void * src = nullptr;
};

}