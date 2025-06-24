#include "ButtonManager.hpp"

ButtonManager::ButtonManager() : _buttonCount(0), _debounceDelay(50), _actionCallback(nullptr) {}

bool ButtonManager::addButton(uint8_t pin, ButtonAction action, bool activeLow) {
    if (_buttonCount >= MAX_BUTTONS) return false;
    _buttons[_buttonCount].pin = pin;
    _buttons[_buttonCount].action = action;
    _buttons[_buttonCount].activeLow = activeLow;
    _buttons[_buttonCount].lastState = activeLow ? HIGH : LOW;
    _buttons[_buttonCount].lastDebounceTime = 0;
    _buttons[_buttonCount].waitingForRelease = false;
    if (activeLow) pinMode(pin, INPUT_PULLUP);
    else pinMode(pin, INPUT);
    _buttonCount++;
    return true;
}

void ButtonManager::onButtonPressed(ButtonActionCallback callback) {
    _actionCallback = callback;
}

void ButtonManager::update() {
    unsigned long currentTime = millis();
    for (uint8_t i = 0; i < _buttonCount; ++i) {
        int reading = digitalRead(_buttons[i].pin);
        int pressedState = _buttons[i].activeLow ? LOW : HIGH;
        
        if (reading != _buttons[i].lastState) {
            _buttons[i].lastDebounceTime = currentTime;
        }

        if ((currentTime - _buttons[i].lastDebounceTime) > _debounceDelay) {
            if (reading == pressedState && !_buttons[i].waitingForRelease) {
                if (_actionCallback != nullptr) _actionCallback(_buttons[i].action);
                _buttons[i].waitingForRelease = true;
            } else if (reading != pressedState) {
                _buttons[i].waitingForRelease = false;
            }
        }
        _buttons[i].lastState = reading;
    }
}