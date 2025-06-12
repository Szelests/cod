#include "LedManager.hpp"

LedManager::LedManager(uint8_t rPin, uint8_t gPin, uint8_t bPin, PCF8574& expander, const uint8_t* expanderAnodePins, size_t numLeds)
    : _rPin(rPin), _gPin(gPin), _bPin(bPin), _expander(expander), 
      _anodePinsOnExpander(expanderAnodePins), _numLeds(numLeds),
      _isLedOn(false), _turnOffTime(0) {}

void LedManager::begin() {
    pinMode(_rPin, OUTPUT);
    pinMode(_gPin, OUTPUT);
    pinMode(_bPin, OUTPUT);
    
    for (size_t i = 0; i < _numLeds; ++i) {
        _expander.pinMode(_anodePinsOnExpander[i], OUTPUT);
    }
    
    turnOffAll();
}

void LedManager::turnOffAll() {
    // Desliga todos os LEDs colocando seus pinos de anodo em LOW
    for (size_t i = 0; i < _numLeds; i++) {
        _expander.digitalWrite(_anodePinsOnExpander[i], LOW);
    }

    // Garante que os pinos de cor não estejam drenando corrente (valor máximo de PWM para anodo comum)
    analogWrite(_rPin, 255);
    analogWrite(_gPin, 255);
    analogWrite(_bPin, 255);

    _isLedOn = false;
    _turnOffTime = 0;
}

void LedManager::displayColor(uint8_t ledIndex, uint8_t r, uint8_t g, uint8_t b, unsigned long duration) {
    if (ledIndex >= _numLeds) return;

    turnOffAll(); // Garante que tudo esteja desligado antes de começar

    // Define a cor nos pinos PWM do Arduino (lógica invertida para anodo comum)
    analogWrite(_rPin, 255 - r);
    analogWrite(_gPin, 255 - g);
    analogWrite(_bPin, 255 - b);

    // Liga APENAS o anodo do LED desejado, colocando seu pino no expansor em HIGH
    _expander.digitalWrite(_anodePinsOnExpander[ledIndex], HIGH);

    _isLedOn = true;
    _turnOffTime = millis() + duration;
}

void LedManager::update() {
    if (_isLedOn && millis() >= _turnOffTime) {
        turnOffAll();
    }
}