//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "PSound.hpp"

#include "PLog.hpp"

#include <SDL3/SDL.h>
#include <libopenmpt/libopenmpt.h>

#include <string>
#include <vector>

namespace PSound {

/**
 * Music is decoded by libopenmpt and fed to SDL through an audio stream.
 * SDL calls music_callback from its audio thread whenever the device needs
 * more data; the callback renders that much PCM from the current module.
 * SDL converts it to the device's format and sample rate.
 */
static const int MUSIC_FREQ = 48000;
static const int MUSIC_CHANNELS = 2;

static SDL_AudioStream* music_stream = nullptr;
static openmpt_module* music_module = nullptr;
static PFile::File playingMusic = PFile::File("");

static float mus_volume = 100;
static float mus_volume_now = 100;

// Called by SDL with the stream locked, so music_module can't change under it.
static void SDLCALL music_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {

	if (music_module == nullptr)
		return; // nothing queued, SDL plays silence

	const int CHUNK_FRAMES = 1024;
	float buffer[CHUNK_FRAMES * MUSIC_CHANNELS];

	int frames = additional_amount / (int)(MUSIC_CHANNELS * sizeof(float));

	while (frames > 0) {   // robust music handling for 1-n frames if necessary. TODO: test by lagging on limited hardware. 
		int chunk = frames < CHUNK_FRAMES ? frames : CHUNK_FRAMES;
		size_t rendered = openmpt_module_read_interleaved_float_stereo(music_module, MUSIC_FREQ, chunk, buffer);
		if (rendered == 0)
			break;

		SDL_PutAudioStreamData(stream, buffer, (int)(rendered * MUSIC_CHANNELS * sizeof(float)));
		frames -= (int)rendered;
	}
}

// Abdullah: plans to allow libopenmpt's powerful music controls to be exposed via PSound? (like flexible repeats, seeking based on demand, etc.)

static void set_music_gain(float volume) {
	if (music_stream != nullptr)
		SDL_SetAudioStreamGain(music_stream, volume / 100.f);
}

static void free_music() {
	if (music_stream != nullptr)
		SDL_LockAudioStream(music_stream);

	if (music_module != nullptr) {
		openmpt_module_destroy(music_module);
		music_module = nullptr;
	}

	if (music_stream != nullptr) {
		SDL_ClearAudioStream(music_stream);
		SDL_UnlockAudioStream(music_stream);
	}
}

int init(int buffer_size) {

	SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, std::to_string(buffer_size).c_str());

	SDL_AudioSpec spec;
	spec.format = SDL_AUDIO_F32;
	spec.channels = MUSIC_CHANNELS;
	spec.freq = MUSIC_FREQ;

	music_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, music_callback, nullptr);
	if (music_stream == nullptr) {
		PLog::Write(PLog::FATAL, "PSound", "Unable to open audio device: %s", SDL_GetError());
		return -1;
	}

	set_music_gain(mus_volume_now);
	SDL_ResumeAudioStreamDevice(music_stream);

	PLog::Write(PLog::DEBUG, "PSound", "buffer size: %i", buffer_size);
	PLog::Write(PLog::DEBUG, "PSound", "Audio driver: %s", SDL_GetCurrentAudioDriver());

	u32 v = openmpt_get_library_version();
	PLog::Write(PLog::DEBUG, "PSound", "libopenmpt %u.%u.%u", v >> 24, (v >> 16) & 0xFF, v & 0xFFFF);

	return 0;
}

int update() {

	const float VOLUME_DELAY = 19.f;

	mus_volume_now = (VOLUME_DELAY * mus_volume_now + mus_volume) / (VOLUME_DELAY + 1);
	set_music_gain(mus_volume_now);

	return 0;
}

int terminate() {

	free_music();

	if (music_stream != nullptr) {
		SDL_DestroyAudioStream(music_stream);
		music_stream = nullptr;
	}

	return 0;
}

// TODO: sound effects are not implemented yet on SDL3.

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

bool is_playing(int channel) {
	return false;
}

int set_channel(int channel, int panoramic, int volume){
	return 0;
}

int start_music(const PFile::File& file) {

	if (playingMusic == file)
		return 1;

	free_music();
	playingMusic = file;

	std::vector<u8> data;
	try {
		PFile::RW rw = file.getRW("r");
		data.resize(rw.size());          // Abdullah Long Term: what happens if we run out of memory?
		if (rw.read(data.data(), data.size()) != data.size()) {
			PLog::Write(PLog::ERR, "PSound", "Unable to read music \"%s\"", file.c_str());
			return -1;
		}
	}
	catch (const PFile::PFileException& e) {
		PLog::Write(PLog::ERR, "PSound", e.what());
		PLog::Write(PLog::ERR, "PSound", "Unable to load music \"%s\"", file.c_str());
		return -1;
	}

	int error = OPENMPT_ERROR_OK;
	const char* error_message = nullptr;

	// libopenmpt copies what it needs, so data can go out of scope afterwards.
	openmpt_module* module = openmpt_module_create_from_memory2(
		data.data(), data.size(),
		openmpt_log_func_silent, nullptr,
		openmpt_error_func_store, nullptr,
		&error, &error_message, nullptr);

	if (module == nullptr) {
		PLog::Write(PLog::ERR, "PSound", "Unable to load music \"%s\": %s",
			file.c_str(), error_message != nullptr ? error_message : "unknown error");
		openmpt_free_string(error_message);
		return -1;
	}

	openmpt_module_set_repeat_count(module, -1); // loop forever

	SDL_LockAudioStream(music_stream);
	music_module = module;
	SDL_UnlockAudioStream(music_stream);

	set_music_gain(mus_volume_now);

	PLog::Write(PLog::DEBUG, "PSound", "Loaded %s", file.c_str());
	return 0;
}

int resume_music() {

	if (music_module == nullptr) {
		static bool warned = false;
		if (!warned) {
			PLog::Write(PLog::WARN, "PSound", "No music loaded");
			warned = true;
		}
		return -1;
	}

	return 0; // the module loops forever, so it never needs restarting
}

void stop_music() {
	free_music();
	playingMusic = PFile::File("");
}

void set_musicvolume(u8 volume) {
	mus_volume = volume;
}

void set_musicvolume_now(u8 volume) {
	mus_volume = volume;
	mus_volume_now = volume;
	set_music_gain(volume);
}

}
