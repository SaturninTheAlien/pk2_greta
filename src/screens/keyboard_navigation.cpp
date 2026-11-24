#include "keyboard_navigation.hpp"
#include "engine/PInput.hpp"

#include <vector>

namespace PK2gui{

namespace KeyNav{

static Nav mNav = NONE;

static int keyDelayCounter = 0;

struct KeyMapEntry {
    PInput::Key key;
    Nav nav = NONE;
};

/**
 * Built on first use, not at static initialization: the PInput::Key constants
 * live in another translation unit, and a namespace-scope array copying them may
 * be initialized first (MinGW runs constructors in that order), which leaves
 * every entry INPUT_UNKNOWN and breaks keyboard and gamepad menu navigation.
 */
static const std::vector<KeyMapEntry>& keyMap() {
    static const std::vector<KeyMapEntry> map = {
        {PInput::Key::ESCAPE, ESCAPE},
        {PInput::Key::LEFT, LEFT},
        {PInput::Key::JOY_LEFT, LEFT},
        {PInput::Key::RIGHT, RIGHT},
        {PInput::Key::JOY_RIGHT, RIGHT},
        {PInput::Key::UP, UP},
        {PInput::Key::JOY_UP, UP},
        {PInput::Key::DOWN, DOWN},
        {PInput::Key::JOY_DOWN, DOWN},
        {PInput::Key::BACKSPACE, BACKSPACE},
        {PInput::Key::DELETE, DELETE},
        {PInput::Key::RETURN, ACCEPT},
        {PInput::Key(SDL_SCANCODE_KP_ENTER), ACCEPT},
        {PInput::Key::JOY_START, ACCEPT},
    };
    return map;
}

static Nav mapKey(const PInput::Key& k) {
    for (const KeyMapEntry& e : keyMap())
        if (k == e.key)
            return e.nav;

    return NONE;
}


Nav readKeyNav(){

    if (mNav != NONE) {
        Nav nav = mNav;
        mNav = NONE;
        return nav;
    }

    if (keyDelayCounter > 0) {
        --keyDelayCounter;
        return NONE;
    }

    // While a text field is being edited, the keyboard belongs to it; only
    // gamepad keys navigate.
    bool typing = PInput::InputSystem::instance().isTextInputActive();

    for (const auto& e : keyMap()) {
        if (typing && e.key.getInputType() == PInput::INPUT_KEYBOARD)
            continue;

        if (e.key.isPressed()) {
            keyDelayCounter = 9;
            return e.nav;
        }
    }
    return NONE;
}


void injectKey(const PInput::Key& k){
    if(mNav != NONE)return;
    mNav = mapKey(k);
    keyDelayCounter = 9;
}


}

}