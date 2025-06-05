#ifndef PWM_SPEAKER_HPP
#define PWM_SPEAKER_HPP

#include <Arduino.h>

class PWMSpeaker {
public:
    /**
     * @brief Construtor da classe PWMSpeaker.
     * @param speakerPin O pino do Arduino ao qual o alto-falante/buzzer está conectado.
     */
    PWMSpeaker(uint8_t speakerPin);

    /**
     * @brief Inicializa o pino do alto-falante. Deve ser chamado no setup().
     */
    void begin();

    /**
     * @brief Toca um tom com uma frequência e duração específicas.
     * A execução do restante do código NÃO é bloqueada enquanto o tom toca (a função tone() usa timers).
     * @param frequency A frequência do tom em Hertz (Hz).
     * @param duration A duração do tom em milissegundos (ms). Se 0, toca continuamente até stopTone().
     */
    void playTone(unsigned int frequency, unsigned long duration = 0);

    /**
     * @brief Para qualquer tom que esteja tocando no pino do alto-falante.
     */
    void stopTone();

    /**
     * @brief Toca um bipe simples com frequência e duração padrão ou especificadas.
     * @param frequency A frequência do bipe em Hertz (Hz). Padrão: 1000 Hz.
     * @param duration A duração do bipe em milissegundos (ms). Padrão: 100 ms.
     */
    void beep(unsigned int frequency = 1000, unsigned long duration = 100);

private:
    uint8_t _speakerPin; // Pino conectado ao alto-falante/buzzer
};

#endif // PWM_SPEAKER_HPP