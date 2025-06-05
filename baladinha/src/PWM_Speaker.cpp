#include "PWMSpeaker.hpp"

PWMSpeaker::PWMSpeaker(uint8_t speakerPin)
    : _speakerPin(speakerPin) {
    // O construtor apenas armazena o pino.
    // A inicialização do pino (pinMode) é feita em begin().
}

void PWMSpeaker::begin() {
    pinMode(_speakerPin, OUTPUT); // Configura o pino do alto-falante como saída
}

void PWMSpeaker::playTone(unsigned int frequency, unsigned long duration) {
    if (frequency == 0) { // Frequência 0 pode ser usada para silêncio ou parar
        noTone(_speakerPin);
        return;
    }
    if (duration > 0) {
        tone(_speakerPin, frequency, duration);
    } else {
        tone(_speakerPin, frequency); // Toca continuamente se a duração for 0
    }
}

void PWMSpeaker::stopTone() {
    noTone(_speakerPin);
}

void PWMSpeaker::beep(unsigned int frequency, unsigned long duration) {
    if (frequency > 0 && duration > 0) {
        tone(_speakerPin, frequency, duration);
        // Importante: tone() com duração é não-bloqueante.
        // O som toca em background usando timers.
        // Se você quisesse que o beep fosse bloqueante, adicionaria um delay(duration); aqui,
        // mas isso geralmente não é desejável pois para todo o resto do código.
    }
}