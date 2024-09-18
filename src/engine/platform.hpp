//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/types.hpp"

#ifdef _WIN32

#else
	#include <unistd.h>
	#include <limits.h>
#endif


#ifdef _WIN32
    #define PE_SEP "\\"
    #define PE_NOSEP "/"
#else
    #define PE_SEP "/"
    #define PE_NOSEP "\\"
#endif

#define PE_PATH_SIZE 128

// Win32 redefinitions and stuff to make MSVC happy
#if defined (_WIN32) && defined (_MSC_VER)

// silence macro definition warnings (C4005)
#pragma warning(disable: 4005)

// require #include <direct.h>
#ifndef getcwd
#define getcwd _getcwd
#endif

#ifndef chdir
#define chdir _chdir
#endif

#endif // _WIN32 && _MSC_VER
