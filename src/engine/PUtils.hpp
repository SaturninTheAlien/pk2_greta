//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/platform.hpp"

#include <vector>
#include <string>

namespace PUtils {

void Setcwd();
void Lower(char* string);
void RemoveSpace(char* string);

void Show_Error(const char* txt);

void GetLanguage(char* lang);

void Force_Mobile();
bool Is_Mobile();
int Alphabetical_Compare(const char *a, const char *b);
bool NoCaseCompare(const char* a, const char* b);

#ifdef __ANDROID__
    bool ExternalWriteable();
#endif

}