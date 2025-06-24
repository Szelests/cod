#include "ColorPiano.hpp"

// --- DEFINIÇÕES ---
#define PIN_S0 4
#define PIN_S1 5
#define PIN_S2 6
#define PIN_S3 7
#define PIN_OUT_SENSOR 8
#define PIN_SPEAKER 9 
#define PIN_LED_R 3
#define PIN_LED_G 5
#define PIN_LED_B 6
#define PIN_BTN_SOUND_1 10
#define PIN_BTN_SOUND_2 11
#define PIN_BTN_SOUND_3 12
#define PIN_BTN_SOUND_4 13
#define PIN_BTN_VOL_DOWN A0
#define PIN_BTN_VOL_UP A1
#define PIN_BTN_CAPTURE 2 
#define I2C_ADDR_LEDS 0x27   
#define I2C_ADDR_LCD  0x23   
const uint8_t EXPANDER_ANODE_PINS[] = { P0, P1, P2, P3 };
const unsigned long COMBO_HOLD_DURATION = 2000;
const unsigned long TIMEOUT_PLAYBACK_MODE = 5000;
const uint16_t pianoNotes[] = { 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988 };
const uint8_t numPianoNotes = sizeof(pianoNotes) / sizeof(pianoNotes[0]);

ColorPiano* ColorPiano::_instance = nullptr;

// --- IMPLEMENTAÇÃO DA CLASSE COLORPIANO ---

ColorPiano::ColorPiano() : 
    _ledExpander(I2C_ADDR_LEDS),
    _display(I2C_ADDR_LCD, 16, 2),
    _sensor(PIN_S0, PIN_S1, PIN_S2, PIN_S3, PIN_OUT_SENSOR),
    _speaker(PIN_SPEAKER),
    _ledManager(PIN_LED_R, PIN_LED_G, PIN_LED_B, _ledExpander),
    _buttonManager()
{
    _instance = this;
    // Inicialização das variáveis de estado
    _currentMode = Mode::REALTIME_DISPLAY;
    _calibState = CalibState::IDLE;
    _volume = 5;
    _freezeLedsUntil = 0;
    _comboPressStartTime = 0;
    _lastInteractionTime = 0;
    _comboInProgress = false;
}

void ColorPiano::button_action_wrapper(ButtonAction action) {
    if (_instance) {
        _instance->_handleButtonAction(action);
    }
}

void ColorPiano::setup() {
    Serial.begin(115200);
    Wire.begin(); 
    _display.begin();
    _display.showWelcome();
    
    _ledExpander.begin();
    _sensor.begin(TCS3200Sensor::FrequencyScaling::SCALE_20_PERCENT);
    _speaker.begin();
    _ledManager.begin(EXPANDER_ANODE_PINS, 4);
    
    // --- CORREÇÃO CRÍTICA AQUI ---
    // Em vez da lambda, agora registramos nossa função wrapper estática.
    _buttonManager.onButtonPressed(button_action_wrapper);

    // O registro dos botões continua o mesmo
    _buttonManager.addButton(PIN_BTN_SOUND_1, ButtonAction::PLAY_SOUND_1);
    _buttonManager.addButton(PIN_BTN_SOUND_2, ButtonAction::PLAY_SOUND_2);
    _buttonManager.addButton(PIN_BTN_SOUND_3, ButtonAction::PLAY_SOUND_3);
    _buttonManager.addButton(PIN_BTN_SOUND_4, ButtonAction::PLAY_SOUND_4);
    _buttonManager.addButton(PIN_BTN_VOL_UP, ButtonAction::VOLUME_UP);
    _buttonManager.addButton(PIN_BTN_VOL_DOWN, ButtonAction::VOLUME_DOWN);
    
    // A configuração da interrupção também usa um wrapper (que deve estar no main.cpp)
    pinMode(PIN_BTN_CAPTURE, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_CAPTURE), isr_wrapper, FALLING);
    
    delay(2000);
    _enterCalibrationMode();
}

void ColorPiano::loop() {
    _buttonManager.update(); 
    _ledManager.update();

    bool volUpPressed = (digitalRead(PIN_BTN_VOL_UP) == LOW);
    bool volDownPressed = (digitalRead(PIN_BTN_VOL_DOWN) == LOW);
    if (volUpPressed && volDownPressed && _currentMode != Mode::CALIBRATION) {
        if (!_comboInProgress) {
            _comboInProgress = true; _comboPressStartTime = millis();
        } else if (millis() - _comboPressStartTime > COMBO_HOLD_DURATION) {
            _enterCalibrationMode(); 
            _comboInProgress = false;
        }
    } else {
        _comboInProgress = false;
    }
    
    if (_currentMode == Mode::REALTIME_DISPLAY) {
        _runRealtimeMode();
    } 
    else if (_currentMode == Mode::PLAYBACK) {
        if (millis() - _lastInteractionTime > TIMEOUT_PLAYBACK_MODE) {
            _currentMode = Mode::REALTIME_DISPLAY;
            _display.showRealtimeMode();
        }
    }
    
    delay(20);
}

void ColorPiano::isr_wrapper() {
    if (_instance) _instance->_handleInterrupt();
}

void ColorPiano::_handleInterrupt() {
    _handleButtonAction(ButtonAction::CAPTURE_COLOR);
}

void ColorPiano::_handleButtonAction(ButtonAction action) {
    if (action != ButtonAction::CAPTURE_COLOR && (millis() - _lastInteractionTime < 250)) return; // Debounce extra
    _lastInteractionTime = millis();

    if (_currentMode == Mode::CALIBRATION) {
        // ... Lógica de calibração que você já tem ...
        return;
    }

    // ... Lógica de operação normal que você já tem ...
}

void ColorPiano::_runRealtimeMode() {
    if (millis() > _freezeLedsUntil) {
        _sensor.getCalibratedRGB(_r, _g, _b);
        _ledManager.setContinuousColorOnAll(_r, _g, _b);
    }
}

void ColorPiano::_enterCalibrationMode() {
    _currentMode = Mode::CALIBRATION;
    _calibState = CalibState::WAIT_WHITE;
    _soundStack.reset();
    _ledManager.turnOffAll();
    _display.showCalibrationMode();
    _speaker.playTone(1200, 50);
}

const char* ColorPiano::_rgbToColorName(uint8_t r, uint8_t g, uint8_t b) {
    if (r > 180 && g > 180 && b > 180) return "Branco";
    if (r > 180 && g > 180 && b < 100) return "Amarelo";
    if (r > 180 && b > 180 && g < 100) return "Magenta";
    if (g > 180 && b > 180 && r < 100) return "Ciano";
    if (r > 180 && g < 100 && b < 100) return "Vermelho";
    if (g > 180 && r < 100 && b < 100) return "Verde";
    if (b > 180 && r < 100 && g < 100) return "Azul";
    return "Misto";
}

uint16_t ColorPiano::_mapColorToFrequency(int r, int g, int b) {
    if (r < 20 && g < 20 && b < 20 && (r + g + b) < 50) return 0;
    uint16_t hue = _rgbToHue(r, g, b); 
    uint8_t noteIndex = map(hue, 0, 359, 0, numPianoNotes - 1);
    noteIndex = constrain(noteIndex, 0, numPianoNotes - 1);
    return pianoNotes[noteIndex];
}

uint16_t ColorPiano::_rgbToHue(int r, int g, int b) {
    float r_norm = r / 255.0f; float g_norm = g / 255.0f; float b_norm = b / 255.0f;
    float cmax = max(max(r_norm, g_norm), b_norm); float cmin = min(min(r_norm, g_norm), b_norm);
    float delta = cmax - cmin; float hue = 0;
    if (delta < 0.0001f) { hue = 0; }
    else {
        if (cmax == r_norm) { hue = 60 * fmod(((g_norm - b_norm) / delta), 6.0f); }
        else if (cmax == g_norm) { hue = 60 * (((b_norm - r_norm) / delta) + 2.0f); }
        else { hue = 60 * (((r_norm - g_norm) / delta) + 4.0f); }
    }
    if (hue < 0) { hue += 360.0f; }
    return (uint16_t)hue;
}