#ifndef COLOR_PIANO_HPP
#define COLOR_PIANO_HPP

#include "TCS3200Sensor.hpp"
#include "PWMSpeaker.hpp"
#include "ButtonManager.hpp"
#include "SoundStack.hpp"
#include "LedManager.hpp"
#include "DisplayManager.hpp"

class ColorPiano {
public:
    ColorPiano();
    void setup();
    void loop();
    static void isr_wrapper();
    static void button_action_wrapper(ButtonAction action);

private:
    PCF8574 _ledExpander;
    DisplayManager _display;
    TCS3200Sensor _sensor;
    PWMSpeaker _speaker;
    SoundStack _soundStack;
    LedManager _ledManager;
    ButtonManager _buttonManager;

    enum class Mode { REALTIME_DISPLAY, PLAYBACK, CALIBRATION };
    Mode _currentMode;
    enum class CalibState { IDLE, WAIT_WHITE, WAIT_BLACK };
    CalibState _calibState;
    
    int _r, _g, _b;
    uint8_t _volume;
    unsigned long _freezeLedsUntil;
    unsigned long _comboPressStartTime;
    unsigned long _lastInteractionTime;
    bool _comboInProgress;
    
    static ColorPiano* _instance;
    
    void _handleButtonAction(ButtonAction action);
    void _runRealtimeMode();
    void _enterCalibrationMode();
    void _handleInterrupt();
    const char* _rgbToColorName(uint8_t r, uint8_t g, uint8_t b);
    uint16_t _mapColorToFrequency(int r, int g, int b);
    uint16_t _rgbToHue(int r, int g, int b); // <<< ADICIONE ESTA LINHA FALTANTE

};
#endif