#ifndef BUTTON_MANAGER_HPP
#define BUTTON_MANAGER_HPP

#include <Arduino.h>
// #include <functional> // GARANTA QUE ESTA LINHA ESTEJA REMOVIDA OU COMENTADA

const uint8_t MAX_BUTTONS = 10;
enum class ButtonAction { NONE=0, PLAY_SOUND_1, PLAY_SOUND_2, PLAY_SOUND_3, PLAY_SOUND_4, VOLUME_UP, VOLUME_DOWN, CAPTURE_COLOR };

// Usando o ponteiro de função C padrão, que é compatível com o Uno
typedef void (*ButtonActionCallback)(ButtonAction action);

class ButtonManager {
public:
    ButtonManager();
    bool addButton(uint8_t pin, ButtonAction action, bool activeLow = true);
    void onButtonPressed(ButtonActionCallback callback); // Espera o tipo simples
    void update();

private:
    struct Button { uint8_t pin; ButtonAction action; bool activeLow; int lastState; unsigned long lastDebounceTime; bool waitingForRelease; };
    Button _buttons[MAX_BUTTONS];
    uint8_t _buttonCount;
    unsigned long _debounceDelay;
    ButtonActionCallback _actionCallback; // Agora é um ponteiro de função simples
};

#endif