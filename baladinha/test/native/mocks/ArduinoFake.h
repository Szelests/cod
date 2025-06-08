#ifndef ARDUINO_FAKE_H
#define ARDUINO_FAKE_H

#include <stdint.h>
#include <vector>
#include <map>
#include <string>
#include <algorithm> // Para std::min/max que usaremos no mock de constrain

// Tipos e Constantes básicas do Arduino
#define HIGH 0x1
#define LOW  0x0
#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2
const uint8_t A0 = 14;
const uint8_t A1 = 15;
const uint8_t A2 = 16;
const uint8_t A3 = 17;
const uint8_t A4 = 18;
const uint8_t A5 = 19;

// --- Simulação do Tempo ---
extern unsigned long FAKE_MILLIS_VALUE;
unsigned long millis();
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
void advanceMillis(unsigned long ms_increment);

// --- Simulação de I/O Digital ---
extern std::map<uint8_t, uint8_t> PIN_MODE_MAP;
extern std::map<uint8_t, int> DIGITAL_READ_MAP;
extern std::map<uint8_t, int> DIGITAL_WRITE_MAP;
void pinMode(uint8_t pin, uint8_t mode);
int digitalRead(uint8_t pin);
void digitalWrite(uint8_t pin, uint8_t val);
void setDigitalReadResult(uint8_t pin, int value);

// --- Simulação de Som ---
struct ToneCall {
    uint8_t pin;
    unsigned int frequency;
    unsigned long duration;
};
extern std::vector<ToneCall> TONE_CALLS;
void tone(uint8_t pin, unsigned int frequency, unsigned long duration = 0);
void noTone(uint8_t pin);

// --- Simulação de Comunicação Serial ---
class FakeSerial {
public:
    static std::vector<std::string> PRINTED_DATA;
    explicit operator bool(); // Para simular 'if (Serial)'
    void begin(unsigned long baud);
    size_t print(const char str[]);
    size_t print(int val);
    size_t println(const char str[]);
    size_t println(int val);
};
extern FakeSerial Serial;

// --- Simulação de Funções Matemáticas e de Pulso ---
long map(long x, long in_min, long in_max, long out_min, long out_max);
long constrain(long x, long a, long b);

struct PulseInBehavior { unsigned long duration; bool timeout; };
extern std::map<uint8_t, PulseInBehavior> PULSEIN_BEHAVIOR_MAP;
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);
void setPulseInBehavior(uint8_t pin, unsigned long duration, bool should_timeout = false);

// --- Função de Reset ---
void resetArduinoFakeState();

#endif // ARDUINO_FAKE_H