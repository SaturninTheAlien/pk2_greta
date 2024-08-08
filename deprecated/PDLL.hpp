//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#ifdef _WIN32
#ifdef PK2_EXPORTS_DLL
#define PK2_EXPORT __declspec(dllexport)
#else
#define PK2_EXPORT __declspec(dllimport)
#endif

#else
#define PK2_EXPORT
#endif
