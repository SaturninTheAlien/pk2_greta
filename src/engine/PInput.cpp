#include "PInput.hpp"
#include "PInputKey.hpp"

#include "PLog.hpp"
#include "PRender.hpp"
#include "PDraw.hpp"

namespace PInput{


void InputSystem::searchForInputDevices(){

	int start = 0;

	char* cont = getenv("PK2_CONTROLLER");
	if (cont) {
		start = std::max(0, atoi(cont));
	}


	this->closeInputDevices();

	int num = 0;
	SDL_JoystickID* joysticks = SDL_GetJoysticks(&num);

	for (int i = start; i < num; ++i) {
		SDL_JoystickID id = joysticks[i];

		if (SDL_IsGamepad(id)) {
			this->gController = SDL_OpenGamepad(id);
			if (this->gController) {
				PLog::Write(
					PLog::INFO,
					"PInput",
					"Controller found: %s",
					SDL_GetGamepadName(this->gController)
				);
				break;
			}
		}
	}

	SDL_free(joysticks);

	if(this->gController == nullptr) {
		PLog::Write(PLog::INFO, "PInput", "No Controller found");
	}

	if (this->gController) {
		SDL_Joystick* joy = SDL_GetGamepadJoystick(this->gController);
		this->gHaptic = SDL_OpenHapticFromJoystick(joy);

		if (this->gHaptic) {
			if (SDL_HapticRumbleSupported(this->gHaptic) &&
				SDL_InitHapticRumble(this->gHaptic)) {
				// success
			} else {
				SDL_CloseHaptic(this->gHaptic);
				this->gHaptic = nullptr;
			}
		}
	}
	else {
		int numHaptics = 0;
		SDL_HapticID* haptics = SDL_GetHaptics(&numHaptics);

		for (int i = 0; i < numHaptics; ++i) {

			this->gHaptic = SDL_OpenHaptic(haptics[i]);

			if (!this->gHaptic)
				continue;

			if (SDL_HapticRumbleSupported(this->gHaptic) &&
				SDL_InitHapticRumble(this->gHaptic)) {
				break; // success
			}

			SDL_CloseHaptic(this->gHaptic);
			this->gHaptic = nullptr;
		}

		SDL_free(haptics);
	}

	if (this->gHaptic == nullptr) {
		PLog::Write(PLog::INFO, "PInput", "No haptic found");
	}
	
}


void InputSystem::closeInputDevices() {

    if (this->gController != nullptr) {
        SDL_CloseGamepad(this->gController);
        this->gController = nullptr;
    }

    if (this->gHaptic != nullptr) {
        SDL_CloseHaptic(this->gHaptic);
        this->gHaptic = nullptr;
    }
}

void InputSystem::handleEvent(const SDL_Event& event){
	if( (event.type == SDL_EVENT_KEY_DOWN && event.key.repeat == 0) ||
		event.type ==  SDL_EVENT_MOUSE_BUTTON_DOWN ||
		event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN){

		Key eventKey(event);
		for(const std::function<void(const Key&)> &f: this->keyDownListeners){
			f(eventKey);
		}
	}

	else if(event.type == SDL_EVENT_KEY_UP ||
		event.type == SDL_EVENT_MOUSE_BUTTON_UP ||
		event.type == SDL_EVENT_GAMEPAD_BUTTON_UP ){

		Key eventKey(event);
		for(const std::function<void(const Key&)> &f: this->keyUpListeners){
			f(eventKey);
		}
	}
	else if(event.type == SDL_EVENT_TEXT_INPUT){
		if(this->textInput){
			this->lastUTF8Char.read(event.text.text);
		}
	}
	else if(event.type == SDL_EVENT_MOUSE_MOTION){
		for(const std::function<void()>& f: this->physicalMouseMotionListeners){
			f();
		}
	}

	else if(event.type ==  SDL_EVENT_GAMEPAD_ADDED || event.type == SDL_EVENT_GAMEPAD_REMOVED){
		this->searchForInputDevices();
	}
}

bool InputSystem::isKeyPressed(const Key& key) {

    switch (key.type)
    {
    case INPUT_KEYBOARD: {
        const bool* keymap = SDL_GetKeyboardState(nullptr);
        return keymap[(SDL_Scancode)key.code];
    }

    case INPUT_MOUSE_BUTTON: {
        float x, y;
        SDL_MouseButtonFlags buttons = SDL_GetMouseState(&x, &y);
        return (buttons & SDL_BUTTON_MASK(key.code)) != 0;
    }

    case INPUT_GAME_CONTROLLER: {
        if (!gController)
            return false;

        return SDL_GetGamepadButton(
            gController,
            (SDL_GamepadButton)key.code
        );
    }

    default:
        break;
    }

    return false;
}

void InputSystem::startTextInput(){

	if(!this->textInput){
		this->textInput = true;
		SDL_StartTextInput(PRender::window);

		this->lastUTF8Char = PString::UTF8_Char();
	}
}

void InputSystem::stopTextInput(){
	if(this->textInput){
		this->textInput = false;
		SDL_StopTextInput(PRender::window);
	}
}

PString::UTF8_Char InputSystem::getLastUTF8(){
	PString::UTF8_Char res = this->lastUTF8Char;
	if(!res.isNull()){
		this->lastUTF8Char = PString::UTF8_Char();
	}

	return res;
}



float InputSystem::getAxis(int axis) const {

    if (this->gController == nullptr)
        return 0.f;

    float fac = 1.f / 32768.f;

    if (axis == 0)
        fac *= SDL_GetGamepadAxis(gController, SDL_GAMEPAD_AXIS_LEFTX);
    else if (axis == 1)
        fac *= SDL_GetGamepadAxis(gController, SDL_GAMEPAD_AXIS_LEFTY);
    else if (axis == 2)
        fac *= SDL_GetGamepadAxis(gController, SDL_GAMEPAD_AXIS_RIGHTX);
    else if (axis == 3)
        fac *= SDL_GetGamepadAxis(gController, SDL_GAMEPAD_AXIS_RIGHTY);
    else if (axis == 4)
        fac *= SDL_GetGamepadAxis(gController, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
    else if (axis == 5)
        fac *= SDL_GetGamepadAxis(gController, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
    else
        return 0.f;

    if (std::abs(fac) < 0.15f)
        fac = 0.f;

    return fac;
}

void InputSystem::vibrate(int duration, float strength)const{

	u32 duration_ms = (u32)(duration * 5 / 3); //convert game ticks to ms

	if(this->gController!=nullptr){
		u16 rumble = (u16)(0xFFFF * strength);
		SDL_RumbleGamepad(this->gController, rumble, rumble, duration_ms);
	}

	if(this->gHaptic!=nullptr){
		SDL_PlayHapticRumble(this->gHaptic, strength, duration_ms);
	}
}


void InputSystem::updateMouse() {
    static float last_x = 0.f, last_y = 0.f;
    static bool ignore_mouse = false;

    int sw, sh;
    PRender::get_window_size(&sw, &sh);

    int bw, bh;
    PDraw::get_buffer_size(&bw, &bh);

    int off_x, off_y;
    PDraw::get_offset(&off_x, &off_y);

    if (!PRender::is_fullscreen()) {
        SDL_SetWindowRelativeMouseMode(PRender::window, false);

        float tmpx, tmpy;
        SDL_GetMouseState(&tmpx, &tmpy);

        // Problem with fitScreen
        tmpx *= float(bw) / sw;
        tmpy *= float(bh) / sh;

        tmpx -= off_x;
        tmpy -= off_y;

        // Mouse moved
        if (std::abs(last_x - tmpx) > 0.f ||
            std::abs(last_y - tmpy) > 0.f)
            ignore_mouse = false;

        last_x = tmpx;
        last_y = tmpy;

        if (!ignore_mouse) {
            mousePos.x = tmpx;
            mousePos.y = tmpy;
        }

    } else {
		SDL_SetWindowRelativeMouseMode(PRender::window, true);

        ignore_mouse = false;

        float delta_x, delta_y;
        SDL_GetRelativeMouseState(&delta_x, &delta_y);

        mousePos.x += 0.4f * delta_x;
        mousePos.y += 0.4f * delta_y;
    }

    if (this->mouseKeysEnabled) {

        float delta_x = 0.f;
        float delta_y = 0.f;

        delta_x += getAxis(0) * 3.f;
        delta_y += getAxis(1) * 3.f;

        if (isKeyPressed(Key::LEFT) || isKeyPressed(Key::JOY_LEFT))
            delta_x -= 3.f;

        if (isKeyPressed(Key::RIGHT) || isKeyPressed(Key::JOY_RIGHT))
            delta_x += 3.f;

        if (isKeyPressed(Key::UP) || isKeyPressed(Key::JOY_UP))
            delta_y -= 3.f;

        if (isKeyPressed(Key::DOWN) || isKeyPressed(Key::JOY_DOWN))
            delta_y += 3.f;

        if (delta_x > 0.1f || delta_x < -0.1f ||
            delta_y > 0.1f || delta_y < -0.1f)
            ignore_mouse = true;

        mousePos.x += delta_x;
        mousePos.y += delta_y;
    }

    // set limits
    if (mousePos.x < -off_x) mousePos.x = -off_x;
    if (mousePos.x > bw - off_x - 19) mousePos.x = bw - off_x - 19;
    if (mousePos.y < -off_y) mousePos.y = -off_y;
    if (mousePos.y > bh - off_y - 19) mousePos.y = bh - off_y - 19;
}


void InputSystem::updateTouch() {
    this->mTouchlist.clear();

    int bw, bh;
    PDraw::get_buffer_size(&bw, &bh);

    int off_x, off_y;
    PDraw::get_offset(&off_x, &off_y);

    int devicesNumber = 0;
    SDL_TouchID* devices = SDL_GetTouchDevices(&devicesNumber);

    bool mouseSet = false;

    for (int i = 0; i < devicesNumber; i++) {

        SDL_TouchID id = devices[i];

        int fingersNumber = 0;
        SDL_Finger** fingers = SDL_GetTouchFingers(id, &fingersNumber);

        for (int j = 0; j < fingersNumber; j++) {

            SDL_Finger* finger = fingers[j];

            if (finger != nullptr) {

                Touch touch(finger->x, finger->y, finger->id);
                this->mTouchlist.emplace_back(touch);

                int tmpx = finger->x * bw - off_x;
                int tmpy = finger->y * bh - off_y;

                if (!mouseSet) {
                    mouseSet = true;
                    this->mousePos.x = tmpx;
                    this->mousePos.y = tmpy;
                }
            }
        }

        SDL_free(fingers);
    }

    SDL_free(devices);
}


}