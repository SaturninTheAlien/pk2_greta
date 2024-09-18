//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
/**
 * @brief
 * String utils by SaturninTheAlien
 */
#include "PString.hpp"
#include <algorithm>

namespace PString{

std::string lowercase(const std::string& src){

    std::string str = src;

    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return str;
}

std::string rtrim(const std::string & src){
    std::size_t i = src.size();
    while(i>0){
        --i;
        if(!std::isspace(src[i]))break;
    }

    ++i;
    return src.substr(0, i);
}

std::string unwindowsPath(const std::string& path){
    std::string res = path;
    std::size_t n = path.size();
    for(std::size_t i=0;i<n;++i){
        if(res[i]=='\\'){
            res[i]='/';
        }
    }
    return res;
}

}