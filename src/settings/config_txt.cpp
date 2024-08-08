//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "config_txt.hpp"
#include "engine/PLang.hpp"

//TODO Remove this dependency
#include "system.hpp"

static const char default_config[] = 
"\r\n -- Silent suicide"
"\r\n -- If set to \"yes\" the player has no destruction effect"
"\r\n -- after committing suicide by pressing delete key"
"\r\n -- Some players requested it for \"psychological reasons\"."
"\r\n -- disabled by default"
"\r\n*silent_suicide:  no"
"\r\n"
"\r\n"
"\r\n-- Audio Buffer Size"
"\r\n-- low value = low audio latency; high value = less cpu usage"
"\r\n-- Default is 1024"
"\r\n-- Prefer a power of 2: 512 1024 2048 4096 default"
"\r\n---------------"
"\r\n*audio_buffer_size:    default"
"\r\n"
"\r\n"
"\r\n"
"\r\n"
"\r\n-- For the compatibility with some older episodes"
"\r\n-- Don't turn it on without a good reason!"
"\r\n-- On the legacy PK2, the player used to move a bit upwards after using a transformation potion."
"\r\n-- If you are a mapmaker, please don't use it intentionally."
"\r\n--"
"\r\n-- This feature may be removed completely in the future."
"\r\n---------------"
"\r\n*potion_transformation_offset:    no"
"\r\n"
"\r\n"
"\r\n";

void Config_txt::readFile(){
    PLang conf = PLang();
	PFile::Path path = PFile::Path(data_path + "config.txt");
	
	bool ok = conf.Read_File(path);
	if (!ok) {
		
		PFile::RW rw = path.GetRW2("w");
		rw.write(default_config, sizeof(default_config) - 1);
		rw.close();
		return;
	}

	//PLog::Write(PLog::DEBUG, "PK2", "Found config file");

	int idx = conf.Search_Id("audio_buffer_size");
	if (idx != -1) {
		const char* txt = conf.Get_Text(idx);
		int val = atoi(txt);

		if (val > 0) {
			this->audio_buffer_size = val;
			

		}
	}
	//PLog::Write(PLog::DEBUG, "PK2", "Audio buffer size set to %i", configuration.audio_buffer_size);

	idx = conf.Search_Id("audio_multi_thread_x");
	if (idx != -1) {
		const char* txt = conf.Get_Text(idx);

		if (strcmp(txt, "default") == 0)
			this->audio_multi_thread = false;
		else if (strcmp(txt, "yes") == 0)
			this->audio_multi_thread = true;
		else if (strcmp(txt, "no") == 0)
			this->audio_multi_thread = false;

		
	}
	idx = conf.Search_Id("potion_transformation_offset");
	if (idx != -1) {
		const char* txt = conf.Get_Text(idx);

		if (strcmp(txt, "default") == 0)
			this->transformation_offset = false;
		else if (strcmp(txt, "yes") == 0)
			this->transformation_offset = true;
		else if (strcmp(txt, "no") == 0)
			this->transformation_offset = false;	
	}
	idx = conf.Search_Id("silent_suicide");
	if(idx != -1){
		const char* txt = conf.Get_Text(idx);

		if (strcmp(txt, "default") == 0)
			this->silent_suicide = false;
		else if (strcmp(txt, "yes") == 0)
			this->silent_suicide = true;
		else if (strcmp(txt, "no") == 0)
			this->silent_suicide = false;	
	}

	//PLog::Write(PLog::DEBUG, "PK2", "Audio multi thread is %s", configuration.audio_multi_thread? "ON" : "OFF");
}


Config_txt config_txt;