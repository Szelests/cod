#include "LedManager.hpp"

LedManager::LedManager(uint8_t rPin, uint8_t gPin, uint8_t bPin, PCF8574& expander)
    : _rPin(rPin), _gPin(gPin), _bPin(bPin), _expander(expander) {}

void LedManager::begin(const uint8_t* anodePins, size_t numLeds) {
    _anodePins = anodePins;
    _numLeds = numLeds;
    pinMode(_rPin, OUTPUT); pinMode(_gPin, OUTPUT); pinMode(_bPin, OUTPUT);
    for (size_t i = 0; i < _numLeds; ++i) {
        _expander.pinMode(_anodePins[i], OUTPUT);
    }
    turnOffAll();
}

void LedManager::turnOffAll() {
    for (size_t i = 0; i < _numLeds; i++) {
        _expander.digitalWrite(_anodePins[i], LOW);
    }
    analogWrite(_rPin, 255); analogWrite(_gPin, 255); analogWrite(_bPin, 255);
    _isLedOn = false; _isContinuous = false; _turnOffTime = 0;
}

void LedManager::setContinuousColorOnAll(uint8_t r, uint8_t g, uint8_t b) {
    turnOffAll(); 
    _isLedOn = true; _isContinuous = true;
    analogWrite(_rPin, 255 - r); analogWrite(_gPin, 255 - g); analogWrite(_bPin, 255 - b);
    for (size_t i = 0; i < _numLeds; i++) {
        _expander.digitalWrite(_anodePins[i], HIGH);
    }
}

void LedManager::displayColorOnAll(uint8_t r, uint8_t g, uint8_t b, unsigned long duration) {
    setContinuousColorOnAll(r, g, b);
    _isContinuous = false;
    _turnOffTime = millis() + duration;
}

void LedManager::displayColorOnOne(uint8_t ledIndex, uint8_t r, uint8_t g, uint8_t b, unsigned long duration) {
    if (ledIndex >= _numLeds) return;
    turnOffAll();
    _isLedOn = true; _isContinuous = false;
    _turnOffTime = millis() + duration;
    analogWrite(_rPin, 255 - r); analogWrite(_gPin, 255 - g); analogWrite(_bPin, 255 - b);
    _expander.digitalWrite(_anodePins[ledIndex], HIGH);
}

void LedManager::update() {
    if (_isLedOn && !_isContinuous && millis() >= _turnOffTime) {
        turnOffAll();
    }
}