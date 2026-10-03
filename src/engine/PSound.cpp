//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "PSound.hpp"

#include "PLog.hpp"

#include <cstring>
#include <queue>

#define PS_FREQ        MIX_DEFAULT_FREQUENCY
#define PS_FORMAT      MIX_DEFAULT_FORMAT
#define PS_CHANNELS    MIX_DEFAULT_CHANNELS

namespace PSound {

int init(int buffer_size) {
    return 0;
}

int update() {
    return 0;
}

int terminate() {
    return 0;
}

int load_sfx(const PFile::File& file) {
    return 0;
}

int play_sfx(int index, int volume, int panoramic, int freq) {
    return 0;
}

int free_sfx(int index) {
    return 0;
}

void reset_sfx() {
}

void clear_channels() {
}

int start_music(const PFile::File& file) {
    return 0;
}

int resume_music() {
    return 0;
}

void stop_music() {
}

void set_musicvolume(u8 volume) {
}

void set_musicvolume_now(u8 volume) {
}

bool is_playing(int channel) {
    return false;
}

int set_channel(int channel, int panoramic, int volume){
    return 0;
}

}