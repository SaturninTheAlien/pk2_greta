#pragma once
#include "engine/PInput.hpp"

class InputSettingsOld{
public:
    InputSettingsOld(){
        this->setDefault();
    }

    void setDefault();
    void setDefaultJoystick();

    PInput::Key* getKeyByMenuID(unsigned int menuID);

    //bool setKey(const PInput::Key& key, int id);

    PInput::Key left;
    PInput::Key right;
    PInput::Key up;
    PInput::Key down;

    PInput::Key walk_slow;
	PInput::Key attack1;
	PInput::Key attack2;
	PInput::Key open_gift;

    PInput::Key toggleHUD;
    PInput::Key screenshotKey;
    PInput::Key fullscreenModeSwitch;
    PInput::Key pauseGame;
    PInput::Key commitSuicide;
    PInput::Key cycleGifts;

    PInput::Key dev_ghostMode;
    PInput::Key dev_fly;
    PInput::Key dev_heal;
    
    friend void from_json(const nlohmann::json& j, InputSettingsOld& controls);
	friend void to_json(nlohmann::json& j, const InputSettingsOld& controls);
};

class MultiKey{
public:
    MultiKey()=default;
    MultiKey(const PInput::Key& keyboardKey, const PInput::Key& joystickKey):
    keyboardKey(keyboardKey), joystickKey(joystickKey)
    {

    }

    bool isPressed()const{
        return this->keyboardKey.isPressed() || this->joystickKey.isPressed();
    };

    bool wasPressed(const PInput::Key& key)const{
        return key == this->keyboardKey || key==this->joystickKey;
    }

    bool accept(const SDL_Event& event){
        return this->keyboardKey.accept(event) || this->joystickKey.accept(event);
    }
private:
    PInput::Key keyboardKey;
    PInput::Key joystickKey;
};

class InputSettingsMulti{
public:
    InputSettingsMulti(const InputSettingsOld& keyboard, const InputSettingsOld& joystick);

    MultiKey left;
    MultiKey right;
    MultiKey up;
    MultiKey down;

    MultiKey walk_slow;
	MultiKey attack1;
	MultiKey attack2;
	MultiKey open_gift;

    MultiKey toggleHUD;
    MultiKey screenshotKey;
    MultiKey fullscreenModeSwitch;
    MultiKey pauseGame;
    MultiKey commitSuicide;
    MultiKey cycleGifts;
};