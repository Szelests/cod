#ifndef LED_MANAGER_HPP
#define LED_MANAGER_HPP

#include <Arduino.h>
#include <PCF8574.h> 

class LedManager {
public:
    LedManager(uint8_t rPin, uint8_t gPin, uint8_t bPin, PCF8574& expander, const uint8_t* expanderAnodePins, size_t numLeds);

    void begin();
    void displayColor(uint8_t ledIndex, uint8_t r, uint8_t g, uint8_t b, unsigned long duration);
    void turnOffAll();
    void update();

private:
    uint8_t _rPin, _gPin, _bPin;
    PCF8574& _expander;
    const uint8_t* _anodePinsOnExpander;
    size_t _numLeds;
    
    bool _isLedOn;
    unsigned long _turnOffTime;
};

#endif // LED_MANAGER_HPP