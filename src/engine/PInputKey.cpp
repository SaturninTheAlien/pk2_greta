#include "PInputKey.hpp"
#include "PInput.hpp"


namespace PInput{

const Key Key::ESCAPE = Key(SDL_SCANCODE_ESCAPE, INPUT_KEYBOARD);
const Key Key::RETURN = Key(SDL_SCANCODE_RETURN, INPUT_KEYBOARD);
const Key Key::DELETE = Key(SDL_SCANCODE_DELETE, INPUT_KEYBOARD);
const Key Key::BACKSPACE = Key(SDL_SCANCODE_BACKSPACE, INPUT_KEYBOARD);
const Key Key::SPACE = Key(SDL_SCANCODE_SPACE, INPUT_KEYBOARD);

const Key Key::LEFT = Key(SDL_SCANCODE_LEFT, INPUT_KEYBOARD);
const Key Key::RIGHT = Key(SDL_SCANCODE_RIGHT, INPUT_KEYBOARD);
const Key Key::UP = Key(SDL_SCANCODE_UP, INPUT_KEYBOARD);
const Key Key::DOWN = Key(SDL_SCANCODE_DOWN, INPUT_KEYBOARD);
const Key Key::LALT = Key(SDL_SCANCODE_LALT, INPUT_KEYBOARD);


const Key Key::MOUSE_LEFT = Key(SDL_BUTTON_LEFT, INPUT_MOUSE_BUTTON);
const Key Key::MOUSE_RIGHT = Key(SDL_BUTTON_RIGHT, INPUT_MOUSE_BUTTON);
 
const Key Key::JOY_A = Key(SDL_GAMEPAD_BUTTON_SOUTH, INPUT_GAME_CONTROLLER);
const Key Key::JOY_B = Key(SDL_GAMEPAD_BUTTON_EAST, INPUT_GAME_CONTROLLER);
const Key Key::JOY_X = Key(SDL_GAMEPAD_BUTTON_WEST, INPUT_GAME_CONTROLLER);
const Key Key::JOY_Y = Key(SDL_GAMEPAD_BUTTON_NORTH, INPUT_GAME_CONTROLLER);

const Key Key::JOY_UP = Key(SDL_GAMEPAD_BUTTON_DPAD_UP, INPUT_GAME_CONTROLLER);
const Key Key::JOY_DOWN = Key(SDL_GAMEPAD_BUTTON_DPAD_DOWN, INPUT_GAME_CONTROLLER);
const Key Key::JOY_LEFT = Key(SDL_GAMEPAD_BUTTON_DPAD_LEFT, INPUT_GAME_CONTROLLER);
const Key Key::JOY_RIGHT = Key(SDL_GAMEPAD_BUTTON_DPAD_RIGHT, INPUT_GAME_CONTROLLER);

const Key Key::JOY_START = Key(SDL_GAMEPAD_BUTTON_START, INPUT_GAME_CONTROLLER);

const Key Key::JOY_STICK_LEFT = Key(SDL_GAMEPAD_BUTTON_LEFT_STICK, INPUT_GAME_CONTROLLER);
const Key Key::JOY_STICK_RIGHT = Key(SDL_GAMEPAD_BUTTON_RIGHT_STICK, INPUT_GAME_CONTROLLER);
const Key Key::JOY_GUIDE = Key(SDL_GAMEPAD_BUTTON_GUIDE, INPUT_GAME_CONTROLLER);

const Key Key::JOY_LEFT_SHOULDER = Key(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, INPUT_GAME_CONTROLLER);
const Key Key::JOY_RIGHT_SHOULDER = Key(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, INPUT_GAME_CONTROLLER);

Key::Key(const SDL_Event& event) {
    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        this->type = INPUT_KEYBOARD;
        this->code = event.key.scancode;
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        this->type = INPUT_MOUSE_BUTTON;
        this->code = event.button.button;
        break;

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        this->type = INPUT_GAME_CONTROLLER;
        this->code = event.gbutton.button;
        break;

    default:
        this->type = INPUT_UNKNOWN;
        break;
    }
}


bool Key::isPressed()const{
    return PInput::InputSystem::instance().isKeyPressed(*this);

}

bool Key::accept(const SDL_Event& event) const {

    switch (event.type) {

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        return this->type == INPUT_KEYBOARD &&
               this->code == event.key.scancode;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        return this->type == INPUT_MOUSE_BUTTON &&
               this->code == event.button.button;

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        return this->type == INPUT_GAME_CONTROLLER &&
               this->code == event.gbutton.button;

    default:
        break;
    }

    return false;
}

std::string Key::getName() const {

    switch (this->type)
    {
    case INPUT_KEYBOARD:
    {
        SDL_Keycode keycode = SDL_GetKeyFromScancode(
            (SDL_Scancode)this->code,
            SDL_KMOD_NONE,
            false
        );

        const char* name = SDL_GetKeyName(keycode);

        if (name && name[0] != '\0')
            return name;

        return "Unknown Key";
    }

    case INPUT_MOUSE_BUTTON:
    {
        switch (this->code)
        {
        case SDL_BUTTON_LEFT:   return "Mouse Left";
        case SDL_BUTTON_RIGHT:  return "Mouse Right";
        case SDL_BUTTON_MIDDLE: return "Mouse Middle";
        case SDL_BUTTON_X1:     return "Mouse X1";
        case SDL_BUTTON_X2:     return "Mouse X2";
        default:                return "Mouse Button";
        }
    }

    case INPUT_GAME_CONTROLLER:
    {
        switch (this->code)
        {
        case SDL_GAMEPAD_BUTTON_SOUTH:
            return "Joy A";

        case SDL_GAMEPAD_BUTTON_EAST:
            return "Joy B";

        case SDL_GAMEPAD_BUTTON_WEST:
            return "Joy X";

        case SDL_GAMEPAD_BUTTON_NORTH:
            return "Joy Y";

        case SDL_GAMEPAD_BUTTON_BACK:
            return "Joy Back";

        case SDL_GAMEPAD_BUTTON_GUIDE:
            return "Joy Guide";

        case SDL_GAMEPAD_BUTTON_START:
            return "Joy Start";

        default:
            break;
        }

        const char* name = SDL_GetGamepadStringForButton(
            (SDL_GamepadButton)this->code
        );

        if (name && name[0] != '\0')
            return name;

        return "Controller Button";
    }

    default:
        break;
    }

    return "Unknown Input!";
}

std::optional<int> Key::getNumericValue()const{
    if(this->type==INPUT_KEYBOARD){
        if (this->code == SDL_SCANCODE_0 || this->code == SDL_SCANCODE_KP_0) return 0;
        if (this->code == SDL_SCANCODE_1 || this->code == SDL_SCANCODE_KP_1) return 1;
        if (this->code == SDL_SCANCODE_2 || this->code == SDL_SCANCODE_KP_2) return 2;
        if (this->code == SDL_SCANCODE_3 || this->code == SDL_SCANCODE_KP_3) return 3;
        if (this->code == SDL_SCANCODE_4 || this->code == SDL_SCANCODE_KP_4) return 4;
        if (this->code == SDL_SCANCODE_5 || this->code == SDL_SCANCODE_KP_5) return 5;
        if (this->code == SDL_SCANCODE_6 || this->code == SDL_SCANCODE_KP_6) return 6;
        if (this->code == SDL_SCANCODE_7 || this->code == SDL_SCANCODE_KP_7) return 7;
        if (this->code == SDL_SCANCODE_8 || this->code == SDL_SCANCODE_KP_8) return 8;
        if (this->code == SDL_SCANCODE_9 || this->code == SDL_SCANCODE_KP_9) return 9;
    }

    return {};
}

void to_json(nlohmann::json& j, const Key& key){
    j["type"] = key.type;
    j["code"] = key.code;

}

void from_json(const nlohmann::json& j, Key& key) {
    j.at("type").get_to(key.type);
    j.at("code").get_to(key.code);
}
}

