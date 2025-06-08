#include "ArduinoFake.h"

// Definição das variáveis globais dos mocks
unsigned long FAKE_MILLIS_VALUE = 0;
std::map<uint8_t, uint8_t> PIN_MODE_MAP;
std::map<uint8_t, int> DIGITAL_READ_MAP;
std::map<uint8_t, int> DIGITAL_WRITE_MAP;
std::vector<ToneCall> TONE_CALLS;
std::map<uint8_t, PulseInBehavior> PULSEIN_BEHAVIOR_MAP;
FakeSerial Serial;
std::vector<std::string> FakeSerial::PRINTED_DATA;

// --- Implementações ---

unsigned long millis() { return FAKE_MILLIS_VALUE; }
void advanceMillis(unsigned long ms_increment) { FAKE_MILLIS_VALUE += ms_increment; }
void delay(unsigned long ms) { advanceMillis(ms); }
void delayMicroseconds(unsigned int us) { advanceMillis(us / 1000); }

void pinMode(uint8_t pin, uint8_t mode) { PIN_MODE_MAP[pin] = mode; }
int digitalRead(uint8_t pin) {
    if (DIGITAL_READ_MAP.count(pin)) return DIGITAL_READ_MAP[pin];
    return LOW;
}
void digitalWrite(uint8_t pin, uint8_t val) { DIGITAL_WRITE_MAP[pin] = val; }
void setDigitalReadResult(uint8_t pin, int value) { DIGITAL_READ_MAP[pin] = value; }

void tone(uint8_t pin, unsigned int frequency, unsigned long duration) { TONE_CALLS.push_back({pin, frequency, duration}); }
void noTone(uint8_t pin) { TONE_CALLS.push_back({pin, 0, 0}); } // Freq 0 = noTone

FakeSerial::operator bool() { return true; } // Simula que a Serial está sempre pronta
void FakeSerial::begin(unsigned long baud) { (void)baud; /* ignora o parâmetro */ }
size_t FakeSerial::print(const char str[]) { PRINTED_DATA.push_back(std::string(str)); return 0; }
size_t FakeSerial::print(int val) { PRINTED_DATA.push_back(std::to_string(val)); return 0; }
size_t FakeSerial::println(const char str[]) { PRINTED_DATA.push_back(std::string(str) + "\n"); return 0; }
size_t FakeSerial::println(int val) { PRINTED_DATA.push_back(std::to_string(val) + "\n"); return 0; }

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
long constrain(long x, long a, long b) {
    return std::min(std::max(x, a), b);
}

void setPulseInBehavior(uint8_t pin, unsigned long duration, bool should_timeout) { PULSEIN_BEHAVIOR_MAP[pin] = {duration, should_timeout}; }
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
    (void)state; // Ignora o estado por enquanto para simplificar
    if (PULSEIN_BEHAVIOR_MAP.count(pin)) {
        auto behavior = PULSEIN_BEHAVIOR_MAP[pin];
        if (behavior.timeout || behavior.duration > timeout) {
            advanceMillis(timeout / 1000);
            return 0;
        } else {
            advanceMillis(behavior.duration / 1000);
            return behavior.duration;
        }
    }
    advanceMillis(timeout / 1000);
    return 0;
}

void resetArduinoFakeState() {
    FAKE_MILLIS_VALUE = 0;
    PIN_MODE_MAP.clear();
    DIGITAL_READ_MAP.clear();
    DIGITAL_WRITE_MAP.clear();
    TONE_CALLS.clear();
    PULSEIN_BEHAVIOR_MAP.clear();
    FakeSerial::PRINTED_DATA.clear();
}