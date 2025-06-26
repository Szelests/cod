#ifndef COLOR_PIANO_HPP
#define COLOR_PIANO_HPP
#include "TCS3200Sensor.hpp"
#include "PWMSpeaker.hpp"
#include "ButtonManager.hpp"
#include "SoundStack.hpp"
#include "LedManager.hpp"
#include "DisplayManager.hpp"
#include "MiniGame.hpp"

class ColorPiano {
public:
    ColorPiano(); void setup(); void loop();
    static void isr_wrapper();
    static void button_action_wrapper(ButtonAction action);
private:
    PCF8574 _ledExpander; DisplayManager _display;
    TCS3200Sensor _sensor; PWMSpeaker _speaker;
    SoundStack _soundStack; LedManager _ledManager;
    ButtonManager _buttonManager; MiniGame _miniGame;

    enum class Mode { REALTIME_DISPLAY, PLAYBACK, CALIBRATION, MINIGAME };
    Mode _currentMode;
    enum class CalibState { IDLE, WAIT_WHITE, WAIT_BLACK };
    CalibState _calibState;

    int _r, _g, _b; uint8_t _volume;
    unsigned long _defaultSoundDuration;
    unsigned long _freezeLedsUntil;
    unsigned long _comboPressStartTime;
    unsigned long _lastInteractionTime;
    bool _comboInProgress;
    volatile bool _captureRequested;

    static ColorPiano* _instance;
    
    void _handleButtonAction(ButtonAction action);
    void _runRealtimeMode();
    void _runMinigameUpdate();
    void _enterCalibrationMode();
    void _handleInterrupt();
    void _handleCaptureRequest();
    
    int _mapColorToPaletteIndex(int r, int g, int b);
    uint16_t _rgbToHue(int r, int g, int b);
};
#endif