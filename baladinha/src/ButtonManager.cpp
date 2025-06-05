#include "ButtonManager.hpp"

ButtonManager::ButtonManager()
    : _buttonCount(0), _debounceDelay(50), _actionCallback(nullptr) {
    // Construtor: inicializa contagem de botões, delay de debounce e callback.
}

bool ButtonManager::addButton(uint8_t pin, ButtonAction action, bool activeLow) {
    if (_buttonCount >= MAX_BUTTONS) {
        return false; // Limite de botões atingido
    }

    _buttons[_buttonCount].pin = pin;
    _buttons[_buttonCount].action = action;
    _buttons[_buttonCount].activeLow = activeLow;
    _buttons[_buttonCount].lastState = activeLow ? HIGH : LOW; // Estado inicial "não pressionado"
    _buttons[_buttonCount].lastDebounceTime = 0;
    _buttons[_buttonCount].waitingForRelease = false;

    if (activeLow) {
        pinMode(pin, INPUT_PULLUP); // Configura o pino com pull-up interno se ativo em LOW
    } else {
        pinMode(pin, INPUT); // Requer resistor de pull-down externo se ativo em HIGH
    }

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
        int releasedState = _buttons[i].activeLow ? HIGH : LOW;

        // Se o estado do botão mudou, resetar o temporizador de debounce
        if (reading != _buttons[i].lastState) {
            _buttons[i].lastDebounceTime = currentTime;
        }

        if ((currentTime - _buttons[i].lastDebounceTime) > _debounceDelay) {
            // Se o estado estabilizou (passou o tempo de debounce)
            if (reading == pressedState && _buttons[i].lastState == releasedState && !_buttons[i].waitingForRelease) {
                // Botão foi pressionado (transição de solto para pressionado)
                if (_actionCallback != nullptr) {
                    _actionCallback(_buttons[i].action);
                }
                _buttons[i].waitingForRelease = true; // Aguarda o botão ser solto
            } else if (reading == releasedState && _buttons[i].waitingForRelease) {
                // Botão foi solto
                _buttons[i].waitingForRelease = false;
            }
        }
        _buttons[i].lastState = reading; // Salva o estado atual para a próxima iteração
    }
}