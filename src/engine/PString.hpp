//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
/**
 * @brief
 * String utils by SaturninTheAlien
 */

#pragma once
#include <string>

namespace PString{

std::string lowercase(const std::string& src);
std::string rtrim(const std::string & src);
std::string unwindowsPath(const std::string& path);

}