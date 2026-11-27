//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "Piste.hpp"
#include <SDL3/SDL.h>
#include <functional>
#include <stdexcept>

#define UPDATE_FPS 30 //Update FPS each 30 frames

namespace Piste {

static int desired_fps = 60;

static bool ready = false;
static bool running = false;

static float avrg_fps = 0;

static bool debug = false;
static bool draw = true;

static void wait_frame() {

	static u64 last_time = SDL_GetPerformanceCounter();

	u64 c_frec = SDL_GetPerformanceFrequency();

	u64 one_frame = c_frec / desired_fps;
	u64 exit_time = last_time + one_frame;
	
	u64 curr_time = SDL_GetPerformanceCounter();
	
	if (curr_time > exit_time) {
		last_time = curr_time;
		return;
	}
	
	u64 wait_time = exit_time - curr_time;
	u32 ms = wait_time * 1000 / c_frec;

	SDL_Delay(ms);

	while(1) {

		u64 c = SDL_GetPerformanceCounter();
		
		if (c >= exit_time || c < last_time)
			break;

	}

	last_time = exit_time;

}

static void logic() {
	
	SDL_Event event;

	while( SDL_PollEvent(&event) ) {

		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			running = false;
			break;

		case SDL_EVENT_WINDOW_RESIZED:
			PRender::adjust_screen();
			break;
		
		case SDL_EVENT_WINDOW_RESTORED:
			PRender::set_pixelperfect();
			break;
		
		default:
			PInput::InputSystem::instance().handleEvent(event);
		}
	}

	PInput::InputSystem::instance().updateMouse();
	PInput::InputSystem::instance().updateTouch();

	// Pass PDraw informations do PRender
	if (draw) {
		//PGui::update();
		void* buffer8;
		PDraw::get_buffer_data(&buffer8);
		PRender::update(buffer8);
	}

	// Clear PDraw buffer
	PDraw::update();

	if (!PRender::is_vsync() && (desired_fps > 0) && draw)
		wait_frame();

	PSound::update();
	
	if (debug) {
		fflush(stdout);
	}

}

static void sdl_show_version() {
    const int compiled = SDL_VERSION;
    const int linked = SDL_GetVersion();

    PLog::Write(PLog::DEBUG, "Piste",
        "We compiled against SDL version %d.%d.%d ...",
        SDL_VERSIONNUM_MAJOR(compiled),
        SDL_VERSIONNUM_MINOR(compiled),
        SDL_VERSIONNUM_MICRO(compiled));

    PLog::Write(PLog::DEBUG, "Piste",
        "But we are linking against SDL version %d.%d.%d.",
        SDL_VERSIONNUM_MAJOR(linked),
        SDL_VERSIONNUM_MINOR(linked),
        SDL_VERSIONNUM_MICRO(linked));
}


static std::function<void()> game_logic;

/**
 * While the user drags or resizes the window, some platforms (e.g. Windows)
 * block inside SDL_PollEvent until the drag ends, so loop() stops running.
 * SDL keeps sending live-resize expose events (data1 == 1) to event watchers
 * during the drag, so keep the game running from there, at the normal frame
 * rate. Events are left queued and handled by logic() once the drag ends.
 */
static bool SDLCALL live_resize_watch(void* userdata, SDL_Event* event) {
	if (event->type != SDL_EVENT_WINDOW_EXPOSED || event->window.data1 != 1 || !game_logic)
		return true;

	static u64 last_frame_ns = 0;
	u64 frame_ns = SDL_NS_PER_SECOND / (desired_fps > 0 ? desired_fps : 60);
	u64 now = SDL_GetTicksNS();

	PRender::adjust_screen();

	void* buffer8;
	PDraw::get_buffer_data(&buffer8);

	if (now - last_frame_ns >= frame_ns) {
		last_frame_ns = now;

		// The buffer holds the frame drawn before the drag started (or by the
		// previous live frame); clear it and draw the next one.
		PDraw::update();
		game_logic();

		PInput::InputSystem::instance().updateMouse();
		PInput::InputSystem::instance().updateTouch();
		PSound::update();
	}

	if (draw)
		PRender::update(buffer8);
	draw = true;

	return true;
}

void init(int width, int height, const char* name, const char* icon, int audio_buffer_size) {
	
	u32 flags =
        SDL_INIT_AUDIO |
        SDL_INIT_VIDEO |
        SDL_INIT_EVENTS |
        SDL_INIT_JOYSTICK |
        SDL_INIT_HAPTIC |
        SDL_INIT_GAMEPAD;
	
	if (!SDL_Init(flags)) {
		PLog::Write(PLog::FATAL, "Piste", "Unable to init SDL: %s", SDL_GetError());
		throw std::runtime_error(std::string("Unable to init SDL: ") +SDL_GetError());
	}

	sdl_show_version();
	
	PDraw::init(width, height);
	PRender::init(width, height, name, icon);
	if (!SDL_AddEventWatch(live_resize_watch, nullptr)) {
		PLog::Write(PLog::WARN, "Piste", "Cannot add live resize watch: %s", SDL_GetError());
	}
	PInput::InputSystem::instance().searchForInputDevices();

	PSound::init(audio_buffer_size);

	ready = true;

}

void terminate() {
	
	PDraw::terminate();
	PRender::terminate();
	PSound::terminate();

	PInput::InputSystem::instance().closeInputDevices();

	SDL_Quit();

	ready = false;

}

void loop(std::function<void()> GameLogic) {
	
	static int frame_counter = 0;
	static u32 last_time = 0;

	running = true;
	game_logic = GameLogic;

	while(running) {
		
		GameLogic();
		logic();

		frame_counter++;
		if (frame_counter >= UPDATE_FPS) {

			frame_counter = 0;
			u32 elapsed_time = (SDL_GetTicks() - last_time);

			avrg_fps = 1000.f * UPDATE_FPS / elapsed_time;	

			last_time += elapsed_time;
		
		}
		
		draw = true;
	
	}

}

void stop() {
	
	running = false;

}

int get_fps() {
	
	return (int)avrg_fps;

}

void set_debug(bool set) {

	debug = set;

}

int set_fps(int fps) {

	desired_fps = fps;

	PLog::Write(PLog::DEBUG, "Piste", "FPS set to %i", fps);
	
	// Vsync true
	if (fps == -1)
		return PRender::set_vsync(true);
	
	return PRender::set_vsync(false);

}

void ignore_frame() {

	draw = false;

}

bool is_ready() {

	return ready;

}

}
