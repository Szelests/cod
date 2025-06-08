#ifndef BUTTON_MANAGER_HPP
#define BUTTON_MANAGER_HPP

#include <Arduino.h>

const uint8_t MAX_BUTTONS = 12;

enum class ButtonAction {
    NONE = 0,
    PLAY_SOUND_1, PLAY_SOUND_2, PLAY_SOUND_3, PLAY_SOUND_4,
    PLAY_SOUND_5, PLAY_SOUND_6, PLAY_SOUND_7, PLAY_SOUND_8,
    VOLUME_UP, VOLUME_DOWN,
    CAPTURE_COLOR
};

typedef void (*ButtonActionCallback)(ButtonAction action);

class ButtonManager {
public:
    ButtonManager();

    // Método para botões normais (via polling)
    bool addButton(uint8_t pin, ButtonAction action, bool activeLow = true);
    
    // NOVO: Método para o botão especial de interrupção
    void addInterruptButton(uint8_t pin, ButtonAction action);

    void onButtonPressed(ButtonActionCallback callback);

    // Método de atualização para os botões de polling
    void update();

    // NOVO: Método público para ser chamado pela ISR global
    // Deve ser muito rápido!
    void handleInterrupt();

private:
    struct Button {
        uint8_t pin;
        ButtonAction action;
        bool activeLow;
        int lastState;
        unsigned long lastDebounceTime;
        bool waitingForRelease;
    };

    Button _buttons[MAX_BUTTONS];
    uint8_t _buttonCount;
    unsigned long _debounceDelay;
    ButtonActionCallback _actionCallback;

    // NOVAS variáveis para o botão de interrupção
    volatile bool _interruptFlag;
    ButtonAction _interruptAction;
    unsigned long _lastInterruptTime;
};

#endif // BUTTON_MANAGER_HPP