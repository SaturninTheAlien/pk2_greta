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
/**
 * @brief 
 * Convert a string to lowercase
 */
std::string lowercase(const std::string& src);
/**
 * @brief 
 * Remove white characters at the end of a string
 */
std::string rtrim(const std::string & src);

/**
 * @brief 
 * Replace '\\' with '/'
 */
std::string unwindowsPath(const std::string& path);

}