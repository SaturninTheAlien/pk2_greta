#pragma once

#include "types.hpp"
#include "PInputKey.hpp"
#include "PString.hpp"

#include <SDL3/SDL.h>
#include <array>
#include <deque>
#include <vector>
#include <functional>


namespace PInput{

/**
 * One step of text editing, queued while text input is active, in the order
 * the user typed. CHAR is a typed character; KEY is any key press, including
 * the OS's key repeats, so editing keys repeat exactly as in other programs.
 *
 * While text input is active the keyboard belongs to the text field: its keys
 * reach only this queue, not the key listeners or isKeyPressed(). Function keys,
 * modifiers and Left Alt combinations (the fullscreen switch) are left alone.
 */
struct TextEvent {
    enum class Type { CHAR, KEY };

    Type type = Type::CHAR;
    PString::UTF8_Char ch;              // CHAR
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;  // KEY
    SDL_Keymod mod = SDL_KMOD_NONE;     // KEY
};


class Touch{
public:
    Touch(float x, float y, int id):
    x(x), y(y), id(id){

    }

    float x = 0;
    float y = 0;
    int id = 0;
};

class InputSystem{
public:
    static InputSystem& instance() {
        static InputSystem instance;
        return instance;
    }


    void addKeyDownListener(std::function<void(const Key&)> f){
        this->keyDownListeners.emplace_back(f);
    }
    void addKeyUpListener(std::function<void(const Key&)> f){
        this->keyUpListeners.emplace_back(f);
    }

    void addPhysicalMouseMotionListener(std::function<void()> f){
        this->physicalMouseMotionListeners.emplace_back(f);
    }


    bool isJoystickAvailable(){
        return this->gController!=nullptr;
    }

    bool isTextInputActive()const{
        return this->textInput;
    }

    const Point2D& getMousePos(){
        return this->mousePos;
    }


    void searchForInputDevices();
    void closeInputDevices();


    void handleEvent(const SDL_Event& event);
    float getAxis(int axis)const;
    void vibrate(int duration, float strength)const;

    bool isKeyPressed(const Key& key);


    void startTextInput();
    void stopTextInput();
    bool pollTextEvent(TextEvent& event);
    
    
    void updateMouse();
    void updateTouch();

    bool mouseKeysEnabled = false;

    const std::vector<Touch>& getTouches()const{
        return this->mTouchlist;
    }
private:
    std::vector<Touch> mTouchlist;

    InputSystem() = default;

    Point2D mousePos;

    bool takeTypedKey(const SDL_KeyboardEvent& key);

    bool textInput = false;
    std::deque<TextEvent> textEvents;

    // Keys pressed while typing; they stay the text field's until released
    std::array<bool, SDL_SCANCODE_COUNT> typedKeys{};

    SDL_Gamepad *gController = nullptr;
    SDL_Haptic * gHaptic = nullptr;

    std::vector<std::function<void(const Key& key)>> keyDownListeners;
    std::vector<std::function<void(const Key& key)>> keyUpListeners;
    std::vector<std::function<void()>> physicalMouseMotionListeners;
};
    
}