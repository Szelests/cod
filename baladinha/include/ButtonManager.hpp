#ifndef BUTTON_MANAGER_HPP
#define BUTTON_MANAGER_HPP

#include <Arduino.h>

// Número máximo de botões que esta classe pode gerenciar
#define MAX_BUTTONS 12 // 8 sons + 2 volume + 1 captura + um pouco de folga

// Enum para identificar as ações dos botões de forma clara
enum class ButtonAction {
    NONE = 0,
    PLAY_SOUND_1,
    PLAY_SOUND_2,
    PLAY_SOUND_3,
    PLAY_SOUND_4,
    PLAY_SOUND_5,
    PLAY_SOUND_6,
    PLAY_SOUND_7,
    PLAY_SOUND_8,
    VOLUME_UP,
    VOLUME_DOWN,
    CAPTURE_COLOR
    // Adicione mais ações se necessário
};

// Assinatura da função de callback que será chamada quando uma ação de botão ocorrer
typedef void (*ButtonActionCallback)(ButtonAction action);

class ButtonManager {
public:
    ButtonManager();

    /**
     * @brief Adiciona um botão para ser gerenciado. Chame no setup() para cada botão.
     * @param pin O pino do Arduino ao qual o botão está conectado.
     * @param action A ação associada a este botão (da enum ButtonAction).
     * @param activeLow Se true, o botão é considerado pressionado quando o pino está LOW (ex: com INPUT_PULLUP).
     * Se false, o botão é pressionado quando o pino está HIGH (ex: com INPUT_PULLDOWN).
     * @return True se o botão foi adicionado com sucesso, false se o limite de botões foi atingido.
     */
    bool addButton(uint8_t pin, ButtonAction action, bool activeLow = true);

    /**
     * @brief Define a função de callback a ser chamada quando uma ação de botão é detectada.
     * @param callback A função que será chamada.
     */
    void onButtonPressed(ButtonActionCallback callback);

    /**
     * @brief Atualiza o estado de todos os botões. Deve ser chamado repetidamente no loop() principal.
     */
    void update();

private:
    struct Button {
        uint8_t pin;
        ButtonAction action;
        bool activeLow;
        int lastState;
        unsigned long lastDebounceTime;
        bool waitingForRelease; // Para evitar múltiplas ativações com um único pressionamento longo
    };

    Button _buttons[MAX_BUTTONS];
    uint8_t _buttonCount;
    unsigned long _debounceDelay;
    ButtonActionCallback _actionCallback;
};

#endif // BUTTON_MANAGER_HPP