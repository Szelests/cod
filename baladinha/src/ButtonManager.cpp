#include "ButtonManager.hpp"

ButtonManager::ButtonManager()
    : _buttonCount(0), _debounceDelay(50), _actionCallback(nullptr),
      _interruptFlag(false), _interruptAction(ButtonAction::NONE), _lastInterruptTime(0) {
}

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

void ButtonManager::addInterruptButton(uint8_t pin, ButtonAction action) {
    _interruptAction = action;
    pinMode(pin, INPUT_PULLUP); // Configura o pino da interrupção
}

void ButtonManager::onButtonPressed(ButtonActionCallback callback) {
    _actionCallback = callback;
}

void ButtonManager::handleInterrupt() {
    // Debounce simples para a interrupção para evitar ruídos
    unsigned long currentTime = millis();
    if (currentTime - _lastInterruptTime > 200) { // Debounce de 200ms
        _interruptFlag = true;
        _lastInterruptTime = currentTime;
    }
}

void ButtonManager::update() {
    // 1. Verifica a flag da interrupção PRIMEIRO
    if (_interruptFlag) {
        Serial.println("INTERRUPÇÃO DETECTADA!");
        if (_actionCallback != nullptr) {
            _actionCallback(_interruptAction);
        }
        _interruptFlag = false; // Reseta a flag após tratar o evento
    }

    // 2. Continua com a lógica de polling para os outros botões
    unsigned long currentTime = millis();
    for (uint8_t i = 0; i < _buttonCount; ++i) {
        int reading = digitalRead(_buttons[i].pin);
        int pressedState = _buttons[i].activeLow ? LOW : HIGH;
        
        if (reading != _buttons[i].lastState) {
            Serial.print("! Mudanca de estado no pino "); Serial.print(_buttons[i].pin);
            Serial.print("! Novo estado: "); Serial.println(reading == LOW ? "LOW" : "HIGH");
            _buttons[i].lastDebounceTime = currentTime;
        }

        if ((currentTime - _buttons[i].lastDebounceTime) > _debounceDelay) {
            if (reading == pressedState && !_buttons[i].waitingForRelease) {
                Serial.print(">>> DEBOUNCE OK! PINO "); Serial.print(_buttons[i].pin);
                Serial.println(" PRESSIONADO. CHAMANDO CALLBACK... <<<");
                if (_actionCallback != nullptr) {
                    _actionCallback(_buttons[i].action);
                }
                _buttons[i].waitingForRelease = true;
            } else if (reading != pressedState) {
                _buttons[i].waitingForRelease = false;
            }
        }
        _buttons[i].lastState = reading;
    }
}