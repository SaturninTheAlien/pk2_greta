//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "version.hpp"

#define PK2_NAME "Pekka Kana 2"
#define PK2_VERSION_NAME "Greta Engine"


#ifndef PK2_VERSION
#define PK2_VERSION "(Unknown version)"
#endif


#ifdef PK2_USE_ZIP
#define PK2_ZIP_STR "(zip)"
#else
#define PK2_ZIP_STR "(no-zip)"
#endif

const char* PK2_NAME_STR = PK2_NAME;
const char* PK2_VERSION_STR = PK2_NAME " " PK2_VERSION_NAME " " PK2_VERSION " " PK2_ZIP_STR;
const char* PK2_VERSION_STR_MENU = "Greta " PK2_VERSION;