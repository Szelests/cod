#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <PCF8574.h>
#include <LiquidCrystal_I2C.h>
// --- INCLUSÕES DE CABEÇALHO ---
#include "TCS3200Sensor.hpp"
#include "PWMSpeaker.hpp"
#include "ButtonManager.hpp"
#include "SoundStack.hpp"
#include "LedManager.hpp"

// --- DEFINIÇÕES DE PINOS E ENDEREÇOS ---
// Sensor de Cores
#define PIN_S0 4
#define PIN_S1 5
#define PIN_S2 6
#define PIN_S3 7
#define PIN_OUT_SENSOR 8
// Alto-Falante
#define PIN_SPEAKER 3 
// Botões (Conectados diretamente no Arduino)
#define PIN_BTN_SOUND_1 12
#define PIN_BTN_SOUND_2 13
#define PIN_BTN_SOUND_3 A2
#define PIN_BTN_SOUND_4 A3
#define PIN_BTN_VOL_DOWN A0
#define PIN_BTN_VOL_UP A1
#define PIN_BTN_CAPTURE 2 
// LEDs RGB (Cores nos pinos PWM do Arduino)
#define PIN_LED_R 9
#define PIN_LED_G 10
#define PIN_LED_B 11
// Endereço I2C do Expansor
#define I2C_ADDR_LEDS 0x20
#define I2C_ADDR_LCD 0x27 // Endereço I2C do LCD pode ser 0x27 ou 0x3F dependendo do modelo

// Pinos NO EXPANSOR para controlar os anodos dos 4 LEDs
const uint8_t EXPANDER_ANODE_PINS[] = { P0, P1, P2, P3 };

// --- INSTÂNCIAS DAS CLASSES E VARIÁVEIS GLOBAIS ---
LiquidCrystal_I2C lcd(I2C_ADDR_LCD, 16, 2);
PCF8574 expansorLEDS(I2C_ADDR_LEDS);
TCS3200Sensor meuSensor(PIN_S0, PIN_S1, PIN_S2, PIN_S3, PIN_OUT_SENSOR);
PWMSpeaker meuAltoFalante(PIN_SPEAKER);
SoundStack minhaPilhaDeSons; 
LedManager meuGerenciadorDeLeds(PIN_LED_R, PIN_LED_G, PIN_LED_B, expansorLEDS, EXPANDER_ANODE_PINS, 4);
ButtonManager meuGerenciadorDeBotoes;

ButtonManager* globalButtonManager = nullptr;
enum class ProgramState { NORMAL_OPERATION, WAITING_FOR_WHITE_CAL, WAITING_FOR_BLACK_CAL };
ProgramState currentState = ProgramState::NORMAL_OPERATION;

unsigned long comboPressStartTime = 0;
bool comboInProgress = false;
const unsigned long COMBO_HOLD_DURATION = 3000;
unsigned long lastCaptureTime = 0;

int r_atual, g_atual, b_atual;
uint8_t volumeLevel = 5;
unsigned long defaultSoundDuration = 800;

const uint16_t pianoNotes[] = { 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988 };
const uint8_t numPianoNotes = sizeof(pianoNotes) / sizeof(pianoNotes[0]);

// --- ISR ---
void captureButtonISR() {
  if (millis() - lastCaptureTime < 1000) { return; }
  if (globalButtonManager != nullptr) {
    globalButtonManager->handleInterrupt();
  }
}

// --- FUNÇÕES AUXILIARES ---
const char* rgbToColorName(uint8_t r, uint8_t g, uint8_t b) {
    if (r > 200 && g > 200 && b > 200) return "Branco";
    if (r > 200 && g > 200 && b < 50) return "Amarelo";
    if (r > 200 && b > 200 && g < 50) return "Magenta";
    if (g > 200 && b > 200 && r < 50) return "Ciano";
    if (r > 200 && g < 100 && b < 100) return "Vermelho";
    if (g > 200 && r < 100 && b < 100) return "Verde";
    if (b > 200 && r < 100 && g < 100) return "Azul";
    return "Cor Mista";
}

uint16_t rgbToHue(int r, int g, int b) {
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

uint16_t mapColorToFrequency(int r, int g, int b) {
    if (r < 20 && g < 20 && b < 20 && (r + g + b) < 50) { return 0; }
    uint16_t hue = rgbToHue(r, g, b);
    uint8_t noteIndex = map(hue, 0, 359, 0, numPianoNotes - 1);
    noteIndex = constrain(noteIndex, 0, numPianoNotes - 1);
    uint16_t freq = pianoNotes[noteIndex];
    Serial.print("Cor(R,G,B):"); Serial.print(r); Serial.print(","); Serial.print(g); Serial.print(","); Serial.print(b);
    Serial.print(" -> Hue:"); Serial.print(hue); Serial.print(" -> Freq:"); Serial.println(freq);
    return freq;
}

void enterCalibrationMode() {
    Serial.println(">>> MODO DE CALIBRACAO ATIVADO <<<");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calibrando cores");

    minhaPilhaDeSons.reset();
    Serial.println("Pilha de sons resetada.");
    currentState = ProgramState::WAITING_FOR_WHITE_CAL;
    meuAltoFalante.playTone(1000, 100); delay(150); meuAltoFalante.playTone(1500, 100); delay(150); meuAltoFalante.playTone(2000, 100); delay(150);
    Serial.println("Posicione no BRANCO e pressione 'Aumentar Volume'.");
    lcd.setCursor(0, 1);
    lcd.print("Posicione no BRANCO");
    meuAltoFalante.beep(2200, 300);
}

// --- FUNÇÃO DE CALLBACK PRINCIPAL ---
void handleButtonAction(ButtonAction action) {
    if (currentState == ProgramState::WAITING_FOR_WHITE_CAL) {
        if (action == ButtonAction::VOLUME_UP) {
            int r, g, b;
            meuSensor.getRawAverageRGB(r, g, b);
            meuSensor.setWhiteBalance(r, g, b);
            meuAltoFalante.playTone(1500, 100); delay(100); meuAltoFalante.playTone(2000, 150);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("BRANCO calibrado!");
            Serial.println("BRANCO calibrado. Posicione no PRETO e pressione 'Diminuir Volume'.");
            lcd.setCursor(0, 1);
            lcd.print("Posicione no PRETO");
            meuAltoFalante.beep(800, 300);
            currentState = ProgramState::WAITING_FOR_BLACK_CAL;
        }
        return;
    }
    if (currentState == ProgramState::WAITING_FOR_BLACK_CAL) {
        if (action == ButtonAction::VOLUME_DOWN) {
            int r, g, b;
            meuSensor.getRawAverageRGB(r, g, b);
            meuSensor.setBlackBalance(r, g, b);
            meuAltoFalante.playTone(2000, 100); delay(100); meuAltoFalante.playTone(2000, 200);
            lcd.clear();
            lcd.print("PRETO Calibrado.");
            delay(1000);
            lcd.clear();
            lcd.print("Calibracao concluida!");
            Serial.println("PRETO calibrado. Calibracao concluida!");
            delay(500);
            lcd.clear();
            lcd.print("===DISCO MULTISSENSORIAL===");
            currentState = ProgramState::NORMAL_OPERATION;
        }
        return;
    }

    // --- Lógica de Operação Normal ---
    if (action == ButtonAction::CAPTURE_COLOR) {
        lastCaptureTime = millis();
        SoundData capturedData;
        capturedData.r = r_atual;
        capturedData.g = g_atual;
        capturedData.b = b_atual;
        Serial.print("Capturando cor: R="); Serial.print(capturedData.r); 
        Serial.print(", G="); Serial.print(capturedData.g);
        Serial.print(", B="); Serial.println(capturedData.b);
        capturedData.frequency = mapColorToFrequency(r_atual, g_atual, b_atual);
        minhaPilhaDeSons.push(capturedData);
        meuAltoFalante.beep(capturedData.frequency > 0 ? capturedData.frequency : 2000, 100);
        return;
    }

    int soundIndex = -1;
    switch (action) {
        case ButtonAction::PLAY_SOUND_1: soundIndex = 0; break;
        case ButtonAction::PLAY_SOUND_2: soundIndex = 1; break;
        case ButtonAction::PLAY_SOUND_3: soundIndex = 2; break;
        case ButtonAction::PLAY_SOUND_4: soundIndex = 3; break;
        case ButtonAction::VOLUME_UP: if (volumeLevel < 10) volumeLevel++; Serial.print("Volume: "); Serial.println(volumeLevel); break;
        case ButtonAction::VOLUME_DOWN: if (volumeLevel > 0) volumeLevel--; Serial.print("Volume: "); Serial.println(volumeLevel); break;
        default: break;
    }

    if (soundIndex != -1) {
        SoundData dataToPlay = minhaPilhaDeSons.getSoundDataAt(soundIndex);
        if (dataToPlay.frequency > 0) {
            unsigned long currentDuration = defaultSoundDuration + (long(volumeLevel) - 5) * 40;

            // ATIVA TODAS AS SAÍDAS: SOM, LUZ E DISPLAY!
            meuAltoFalante.playTone(dataToPlay.frequency, currentDuration);
            meuGerenciadorDeLeds.displayColor(soundIndex, dataToPlay.r, dataToPlay.g, dataToPlay.b, currentDuration);

            // ATUALIZA O LCD
            const char* colorName = rgbToColorName(dataToPlay.r, dataToPlay.g, dataToPlay.b);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Nota: ");
            lcd.print(dataToPlay.frequency);
            lcd.print(" Hz");
            lcd.setCursor(0, 1);
            lcd.print("Cor: ");
            lcd.print(colorName);
        } else {
            meuAltoFalante.beep(100, 50);
        }
    }
}

// ======================= SETUP =======================
void setup() {
    Serial.begin(9600);
    Wire.begin();

    // Inicializa o LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("---DISCO MULTISSENSORIAL---");
    lcd.setCursor(0, 1);
    lcd.print("Iniciando...");

    expansorLEDS.begin();
    meuSensor.begin(TCS3200Sensor::FrequencyScaling::SCALE_20_PERCENT);
    meuAltoFalante.begin();
    meuGerenciadorDeLeds.begin();
    
    globalButtonManager = &meuGerenciadorDeBotoes;
    meuGerenciadorDeBotoes.onButtonPressed(handleButtonAction);

    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_1, ButtonAction::PLAY_SOUND_1);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_2, ButtonAction::PLAY_SOUND_2);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_3, ButtonAction::PLAY_SOUND_3);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_4, ButtonAction::PLAY_SOUND_4);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_VOL_UP, ButtonAction::VOLUME_UP);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_VOL_DOWN, ButtonAction::VOLUME_DOWN);
    
    meuGerenciadorDeBotoes.addInterruptButton(PIN_BTN_CAPTURE, ButtonAction::CAPTURE_COLOR);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_CAPTURE), captureButtonISR, FALLING);
    
    delay(2000);
    lcd.clear();
    lcd.print("Sistema iniciado!");
    delay(1000);
    lcd.clear();
    Serial.println("Sistema pronto.");
    meuAltoFalante.beep(1800, 200);
}

// ======================= LOOP =======================
void loop() {
    meuGerenciadorDeBotoes.update(); 
    meuSensor.getCalibratedRGB(r_atual, g_atual, b_atual);
    meuGerenciadorDeLeds.update();

    if (currentState == ProgramState::NORMAL_OPERATION) {
        bool volUpPressed = (digitalRead(PIN_BTN_VOL_UP) == LOW);
        bool volDownPressed = (digitalRead(PIN_BTN_VOL_DOWN) == LOW);
        if (volUpPressed && volDownPressed) {
            if (!comboInProgress) {
                comboInProgress = true;
                comboPressStartTime = millis();
            } else if (millis() - comboPressStartTime > COMBO_HOLD_DURATION) {
                enterCalibrationMode(); 
                comboInProgress = false;
            }
        } else {
            comboInProgress = false;
        }
    }
    
    delay(20);
}