#include <Arduino.h>
#include <math.h>
#include "TCS3200Sensor.hpp"
#include "PWMSpeaker.hpp"
#include "ButtonManager.hpp"
#include "SoundStack.hpp"
#include "LedManager.hpp"

// --- DEFINIÇÕES DE PINOS ---
//DEFINIÇÃO SENSOR DE CORES
#define PIN_S0 4 // AZUL ESCURO
#define PIN_S1 5 // LARANJA
#define PIN_S2 6 // BRANCO
#define PIN_S3 7 //VERDE CLARO
#define PIN_OUT_SENSOR 8 // VERDE ESCURO

// BOTÕES
#define PIN_SPEAKER 9 
#define PIN_BTN_SOUND_1 10
#define PIN_BTN_SOUND_2 11
#define PIN_BTN_SOUND_3 12
#define PIN_BTN_SOUND_4 13
#define PIN_BTN_SOUND_5 A5
#define PIN_BTN_SOUND_6 A4
#define PIN_BTN_SOUND_7 A3
#define PIN_BTN_SOUND_8 A2
#define PIN_BTN_VOL_UP A1
#define PIN_BTN_VOL_DOWN A0
#define PIN_BTN_CAPTURE 2

// --- INSTÂNCIAS DAS CLASSES E VARIÁVEIS GLOBAIS ---
TCS3200Sensor meuSensor(PIN_S0, PIN_S1, PIN_S2, PIN_S3, PIN_OUT_SENSOR);
PWMSpeaker meuAltoFalante(PIN_SPEAKER);
ButtonManager meuGerenciadorDeBotoes;
SoundStack minhaPilhaDeSons; 

ButtonManager* globalButtonManager = nullptr; // Ponteiro para a ISR

enum class ProgramState { NORMAL_OPERATION, WAITING_FOR_WHITE_CAL, WAITING_FOR_BLACK_CAL };
ProgramState currentState = ProgramState::NORMAL_OPERATION;

unsigned long comboPressStartTime = 0;
bool comboInProgress = false;
const unsigned long COMBO_HOLD_DURATION = 3000; // Segurar por 3 segundos
unsigned long lastCaptureTime = 0;

int r_atual, g_atual, b_atual;
uint8_t volumeLevel = 5; // 0-10
unsigned long defaultSoundDuration = 300; // Duração do som em ms

const uint16_t pianoNotes[] = { 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988 };
const uint8_t numPianoNotes = sizeof(pianoNotes) / sizeof(pianoNotes[0]);

// --- ROTINA DE SERVIÇO DE INTERRUPÇÃO (ISR) ---
void captureButtonISR() {
  if (millis() - lastCaptureTime < 1000) { return; } // Cooldown de 1s para evitar captura dupla
  if (globalButtonManager != nullptr) {
    globalButtonManager->handleInterrupt();
  }
}

// --- FUNÇÃO AUXILIAR PARA ENTRAR EM MODO DE CALIBRAÇÃO ---
void enterCalibrationMode() {
    Serial.println(">>> MODO DE CALIBRACAO ATIVADO <<<");
    minhaPilhaDeSons.reset();
    Serial.println("Pilha de sons resetada.");
    currentState = ProgramState::WAITING_FOR_WHITE_CAL;

    meuAltoFalante.playTone(1000, 100); delay(150);
    meuAltoFalante.playTone(1500, 100); delay(150);
    meuAltoFalante.playTone(2000, 100); delay(150);
    
    Serial.println("Posicione no BRANCO e pressione 'Aumentar Volume'.");
    meuAltoFalante.beep(2200, 300);
}

// --- FUNÇÕES DE CONVERSÃO DE COR ---
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
    return pianoNotes[noteIndex];
}

// --- FUNÇÃO DE CALLBACK PARA TRATAR AÇÕES DOS BOTÕES ---
void handleButtonAction(ButtonAction action) {
    // --- LÓGICA DE CALIBRAÇÃO ---
    if (currentState == ProgramState::WAITING_FOR_WHITE_CAL) {
        if (action == ButtonAction::VOLUME_UP) {
            Serial.println("Capturando BRANCO...");
            int r, g, b;
            meuSensor.getRawAverageRGB(r, g, b);
            meuSensor.setWhiteBalance(r, g, b);
            meuAltoFalante.playTone(1500, 100); delay(100); meuAltoFalante.playTone(2000, 150);
            Serial.println("Posicione no PRETO e pressione 'Diminuir Volume'.");
            meuAltoFalante.beep(800, 300);
            currentState = ProgramState::WAITING_FOR_BLACK_CAL;
        }
        return;
    }

    if (currentState == ProgramState::WAITING_FOR_BLACK_CAL) {
        if (action == ButtonAction::VOLUME_DOWN) {
            Serial.println("Capturando PRETO...");
            int r, g, b;
            meuSensor.getRawAverageRGB(r, g, b);
            meuSensor.setBlackBalance(r, g, b);
            meuAltoFalante.playTone(2000, 100); delay(100); meuAltoFalante.playTone(2000, 200);
            Serial.println("Calibracao concluida! Retornando ao modo normal.");
            currentState = ProgramState::NORMAL_OPERATION;
        }
        return;
    }

    // --- LÓGICA DE OPERAÇÃO NORMAL ---
    if (action == ButtonAction::CAPTURE_COLOR) {
        lastCaptureTime = millis();
        Serial.println("Botao de Captura Pressionado!");
        uint16_t mappedFreq = mapColorToFrequency(r_atual, g_atual, b_atual);
        minhaPilhaDeSons.push(mappedFreq);
        meuAltoFalante.beep(mappedFreq > 0 ? mappedFreq : 2000, 100);
        return;
    }

    int soundIndex = -1;
    switch (action) {
        case ButtonAction::PLAY_SOUND_1: soundIndex = 0; break;
        case ButtonAction::PLAY_SOUND_2: soundIndex = 1; break;
        case ButtonAction::PLAY_SOUND_3: soundIndex = 2; break;
        case ButtonAction::PLAY_SOUND_4: soundIndex = 3; break;
        case ButtonAction::PLAY_SOUND_5: soundIndex = 4; break;
        case ButtonAction::PLAY_SOUND_6: soundIndex = 5; break;
        case ButtonAction::PLAY_SOUND_7: soundIndex = 6; break;
        case ButtonAction::PLAY_SOUND_8: soundIndex = 7; break;
        case ButtonAction::VOLUME_UP: if (volumeLevel < 10) volumeLevel++; Serial.print("Volume: "); Serial.println(volumeLevel); break;
        case ButtonAction::VOLUME_DOWN: if (volumeLevel > 0) volumeLevel--; Serial.print("Volume: "); Serial.println(volumeLevel); break;
        default: break;
    }

    if (soundIndex != -1) {
        uint16_t freqToPlay = minhaPilhaDeSons.getNoteAt(soundIndex);
        if (freqToPlay > 0) {
            Serial.print("Tocando nota da posicao "); Serial.print(soundIndex); Serial.print(": "); Serial.println(freqToPlay);
            unsigned long currentDuration = defaultSoundDuration + (long(volumeLevel) - 5) * 40;
            meuAltoFalante.playTone(freqToPlay, currentDuration);
        } else {
            Serial.print("Posicao "); Serial.print(soundIndex); Serial.println(" da pilha vazia.");
            meuAltoFalante.beep(100, 50);
        }
    }
}

// =================================================================
// --- SETUP ---
// =================================================================
void setup() {
    Serial.begin(9600);
    while (!Serial);
    Serial.println("Iniciando Color Piano...");

    meuSensor.begin(TCS3200Sensor::FrequencyScaling::SCALE_20_PERCENT);
    meuAltoFalante.begin();
    
    globalButtonManager = &meuGerenciadorDeBotoes;
    meuGerenciadorDeBotoes.onButtonPressed(handleButtonAction);

    // Adiciona os botões de POLLING (SOM e VOLUME)
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_1, ButtonAction::PLAY_SOUND_1);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_2, ButtonAction::PLAY_SOUND_2);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_3, ButtonAction::PLAY_SOUND_3);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_4, ButtonAction::PLAY_SOUND_4);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_5, ButtonAction::PLAY_SOUND_5);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_6, ButtonAction::PLAY_SOUND_6);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_7, ButtonAction::PLAY_SOUND_7);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_SOUND_8, ButtonAction::PLAY_SOUND_8);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_VOL_UP, ButtonAction::VOLUME_UP);
    meuGerenciadorDeBotoes.addButton(PIN_BTN_VOL_DOWN, ButtonAction::VOLUME_DOWN);
    
    // Adiciona e Ativa o Botão de INTERRUPÇÃO
    meuGerenciadorDeBotoes.addInterruptButton(PIN_BTN_CAPTURE, ButtonAction::CAPTURE_COLOR);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_CAPTURE), captureButtonISR, FALLING);
    
    Serial.println("Sistema pronto. Para calibrar, segure os dois botoes de volume por 3s.");
    meuAltoFalante.beep(1800, 200);
}

// =================================================================
// --- LOOP PRINCIPAL ---
// =================================================================
void loop() {
    meuGerenciadorDeBotoes.update(); 
    meuSensor.getCalibratedRGB(r_atual, g_atual, b_atual);

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