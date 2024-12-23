//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

class Config_txt{
public:

    void readFile();

    bool silent_suicide = false;
	//bool audio_multi_thread = true;
	int audio_buffer_size = 1024;

    /**
     * @brief 
     * Deprecated features, they can be removed
     */
    // bool transformation_offset = false;
};

extern Config_txt config_txt;
