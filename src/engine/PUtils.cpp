//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "PUtils.hpp"

#include "PLog.hpp"
#include "PFile.hpp"

#include <SDL3/SDL.h>
#include <filesystem>
#include <cstring>
#include <string>
#include <locale>
#include <sys/stat.h>

#ifdef __ANDROID__
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

namespace PUtils {

void GetLanguage(char* lang) {
	
	char locale[5] = "en";

	int count = 0;
	SDL_Locale** locales = SDL_GetPreferredLocales(&count);

	if (locales && count > 0 && locales[0] && locales[0]->language)
	{
		locale[0] = locales[0]->language[0];
		locale[1] = locales[0]->language[1];
	}

	lang[0] = SDL_tolower(locale[0]);
	lang[1] = SDL_tolower(locale[1]);     // there should be a better way to do this but meh,
	lang[2] = '\0';
}

}