#include <Arduino.h>
#include <math.h> // Para fmod na conversão de Hue
#include "TCS3200Sensor.hpp"
#include "PWMSpeaker.hpp"
#include "ButtonManager.hpp"
#include "SoundStack.hpp"

// --- Configuração dos Pinos do Sensor --
//MARROM CLARO - GND
//AZUL CLARO - VCC
#define PIN_S0 4  // AZUL ESCURO
#define PIN_S1 5  // LARANJA
#define PIN_S2 6  // BRANCO
#define PIN_S3 7  // VERDE CLARO
#define PIN_OUT_SENSOR 8 // VERDE ESCURO

// --- Pino do Alto-falante/Buzzer ---
#define PIN_SPEAKER 9 // Pino do alto-falante (PWM)

// --- Pinos dos Botões ---
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

// Instâncias das classes
TCS3200Sensor meuSensor(PIN_S0, PIN_S1, PIN_S2, PIN_S3, PIN_OUT_SENSOR);
PWMSpeaker meuAltoFalante(PIN_SPEAKER);
ButtonManager meuGerenciadorDeBotoes;
SoundStack minhaPilhaDeSons; 
ButtonManager* globalButtonManager = nullptr;

enum class ProgramState { NORMAL_OPERATION, WAITING_FOR_WHITE_CAL, WAITING_FOR_BLACK_CAL };
ProgramState currentState = ProgramState::NORMAL_OPERATION;

// --- Variáveis para detectar o combo de botões ---
unsigned long comboPressStartTime = 0;
bool comboInProgress = false;
const unsigned long COMBO_HOLD_DURATION = 1500; // Segurar por 1.5 segundos

// Variáveis de cor
int r_atual, g_atual, b_atual;

// Notas de Piano
const uint16_t pianoNotes[] = {
  220, // A3
  233, // A#3
  247, // B3
  262, // C4 (Dó central)
  277, // C#4
  294, // D4
  311, // D#4
  330, // E4
  349, // F4
  370, // F#4
  392, // G4
  415, // G#4
  440, // A4 (Lá padrão)
  466, // A#4
  494, // B4
  523, // C5
  554, // C#5
  587, // D5
  622, // D#5
  659, // E5
  698, // F5
  740, // F#5
  784, // G5
  831, // G#5
  880, // A5
  932, // A#5
  988  // B5
};
const uint8_t numPianoNotes = sizeof(pianoNotes) / sizeof(pianoNotes[0]);



// Volume (conceitual)
uint8_t volumeLevel = 5; // 0-10
unsigned long defaultSoundDuration = 300; // Duração padrão do som em ms

// ----- Funções Auxiliares -----
// --- Rotina de Serviço de Interrupção (ISR) ---
// Esta é a função que o hardware chama quando o pino 2 muda de HIGH para LOW.
void captureButtonISR() {
  if (globalButtonManager != nullptr) {
    globalButtonManager->handleInterrupt();
  }
}

void enterCalibrationMode() {
    Serial.println(">>> MODO DE CALIBRACAO ATIVADO <<<");
    minhaPilhaDeSons.reset(); // Reseta a pilha de sons
    Serial.println("Pilha de sons resetada.");
    currentState = ProgramState::WAITING_FOR_WHITE_CAL;

    // Feedback sonoro de entrada no modo
    meuAltoFalante.playTone(1000, 100); delay(150);
    meuAltoFalante.playTone(1500, 100); delay(150);
    meuAltoFalante.playTone(2000, 100); delay(150);
    
    // Som para indicar "Pronto para o BRANCO"
    Serial.println("Posicione no BRANCO e pressione 'Aumentar Volume'.");
    meuAltoFalante.beep(2200, 300); // Som agudo
}

/**
 * @brief Converte valores RGB (0-255) para um valor de Hue (0-359).
 * Usa o algoritmo de conversão de RGB para HSV.
 * @param r Valor vermelho (0-255).
 * @param g Valor verde (0-255).
 * @param b Valor azul (0-255).
 * @return Valor de Hue (0-359).
 */
uint32_t rgbToHue(uint8_t r, uint8_t g, uint8_t b) {
    float r_norm = r / 255.0f;
    float g_norm = g / 255.0f;
    float b_norm = b / 255.0f;

    float cmax = r_norm;
    if (g_norm > cmax) cmax = g_norm;
    if (b_norm > cmax) cmax = b_norm;

    float cmin = r_norm;
    if (g_norm < cmin) cmin = g_norm;
    if (b_norm < cmin) cmin = b_norm;
    
    float delta = cmax - cmin;
    float hue = 0;

    if (delta < 0.0001f) { // Quase igual, considera escala de cinza
        hue = 0;
    } else {
        if (cmax == r_norm) {
            hue = 60 * fmod(((g_norm - b_norm) / delta), 6.0f);
        } else if (cmax == g_norm) {
            hue = 60 * (((b_norm - r_norm) / delta) + 2.0f);
        } else if (cmax == b_norm) {
            hue = 60 * (((r_norm - g_norm) / delta) + 4.0f);
        }
    }
    if (hue < 0) {
        hue += 360.0f;
    }
    return (uint32_t)hue;
}

/**
 * @brief Mapeia uma cor RGB para uma frequência de nota de piano.
 * Considera a cor preta ou muito escura como sem nota (silêncio).
 * @param r Valor vermelho (0-255).
 * @param g Valor verde (0-255).
 * @param b Valor azul (0-255).
 * @return Frequência da nota correspondente (0 para silêncio).
 */
uint16_t mapColorToFrequency(uint8_t r, uint8_t g, uint8_t b) {
    // Considera preto ou muito escuro como sem nota, para evitar notas aleatórias com baixa leitura
    if (r < 20 && g < 20 && b < 20 && (r+g+b) < 50) { 
        Serial.println("Cor muito escura/preta, sem nota gerada.");
        return 0; // Retorna 0 para silêncio
    }

    uint32_t hue = rgbToHue(r, g, b);
    uint8_t noteIndex = map(hue, 0, 359, 0, numPianoNotes - 1);
    noteIndex = constrain(noteIndex, 0, numPianoNotes - 1);

    uint16_t freq = pianoNotes[noteIndex];

    Serial.print("Cor (R:"); Serial.print(r);
    Serial.print(" G:"); Serial.print(g);
    Serial.print(" B:"); Serial.print(b);
    Serial.print(") -> Hue: "); Serial.print(hue);
    Serial.print(" -> IdxNota: "); Serial.print(noteIndex);
    Serial.print(" -> Freq: "); Serial.println(freq);
    
    return freq;
}


// Função de callback para tratar ações dos botões// Em src/main.cpp
/**
 * @brief Trata a ação do botão pressionado.
 * Dependendo do botão, toca um som, ajusta o volume ou captura a cor atual.
 * @param action Ação do botão pressionado.
 * @return void
 */
void handleButtonAction(ButtonAction action) {
    Serial.print("Acao do botao: ");
    // --- LÓGICA DE CALIBRAÇÃO (usa botões de volume) ---
    if (currentState == ProgramState::WAITING_FOR_WHITE_CAL) {
        if (action == ButtonAction::VOLUME_UP) {
            Serial.println("Capturando referencia de BRANCO com o botao 'Aumentar Volume'...");
            int r, g, b;
            meuSensor.getRawAverageRGB(r, g, b);
            meuSensor.setWhiteBalance(r, g, b);
            meuAltoFalante.playTone(1500, 100); delay(100); meuAltoFalante.playTone(2000, 150); // Som de sucesso
            
            Serial.println("Posicione no PRETO e pressione 'Diminuir Volume'.");
            meuAltoFalante.beep(800, 300); // Som grave para preto
            currentState = ProgramState::WAITING_FOR_BLACK_CAL;
        }
        return; // Ignora outras ações de botão enquanto estiver calibrando
    }

    if (currentState == ProgramState::WAITING_FOR_BLACK_CAL) {
        if (action == ButtonAction::VOLUME_DOWN) {
            Serial.println("Capturando referencia de PRETO com o botao 'Diminuir Volume'...");
            int r, g, b;
            meuSensor.getRawAverageRGB(r, g, b);
            meuSensor.setBlackBalance(r, g, b);
            meuAltoFalante.playTone(2000, 100); delay(100); meuAltoFalante.playTone(1500, 100); delay(100); meuAltoFalante.playTone(2000, 200);
            
            Serial.println("Calibracao concluida! Retornando ao modo normal.");
            currentState = ProgramState::NORMAL_OPERATION;
        }
        return; // Ignora outras ações de botão enquanto estiver calibrando
    }
    uint16_t freqToPlay = 0;
    int soundIndex = -1; // Padrão para nenhum índice de som

    switch (action) {
        // --- CASOS PARA OS BOTÕES DE SOM ---
        case ButtonAction::PLAY_SOUND_1:
            Serial.println("PLAY_SOUND_1");
            soundIndex = 0; // Define o índice da pilha a ser tocado
            break;
        case ButtonAction::PLAY_SOUND_2:
            Serial.println("PLAY_SOUND_2");
            soundIndex = 1;
            break;
        case ButtonAction::PLAY_SOUND_3:
            Serial.println("PLAY_SOUND_3");
            soundIndex = 2;
            break;
        case ButtonAction::PLAY_SOUND_4:
            Serial.println("PLAY_SOUND_4");
            soundIndex = 3;
            break;
        case ButtonAction::PLAY_SOUND_5:
            Serial.println("PLAY_SOUND_5");
            soundIndex = 4;
            break;
        case ButtonAction::PLAY_SOUND_6:
            Serial.println("PLAY_SOUND_6");
            soundIndex = 5;
            break;
        case ButtonAction::PLAY_SOUND_7:
            Serial.println("PLAY_SOUND_7");
            soundIndex = 6;
            break;
        case ButtonAction::PLAY_SOUND_8:
            Serial.println("PLAY_SOUND_8");
            soundIndex = 7;
            break;

        // --- CASOS PARA OS BOTÕES DE VOLUME ---
        case ButtonAction::VOLUME_UP: {
            Serial.print("VOLUME_UP. Volume: ");
            if (volumeLevel < 10) volumeLevel++;
            Serial.println(volumeLevel);
            meuAltoFalante.beep(1500 + volumeLevel * 50, 50 + volumeLevel * 5);
            break;
        }
        case ButtonAction::VOLUME_DOWN: {
            Serial.print("VOLUME_DOWN. Volume: ");
            if (volumeLevel > 0) volumeLevel--;
            Serial.println(volumeLevel);
            meuAltoFalante.beep(1000 - volumeLevel * 50, 50 - volumeLevel * 3);
            break;
        }
        
        // --- CASO PARA O BOTÃO DE CAPTURA ---
        case ButtonAction::CAPTURE_COLOR: {
            Serial.println("CAPTURE_COLOR");
            uint16_t mappedFreq = mapColorToFrequency(r_atual, g_atual, b_atual);
            minhaPilhaDeSons.push(mappedFreq);
            meuAltoFalante.beep(mappedFreq > 0 ? mappedFreq : 2000, 100);
            break;
        }
        
        default:
            Serial.println("Acao desconhecida ou nao tratada.");
            break;
    }

    // --- LÓGICA PARA TOCAR O SOM ---
    // Este bloco só é executado se um dos botões PLAY_SOUND foi pressionado
    if (soundIndex != -1) {
        if ((size_t)soundIndex < minhaPilhaDeSons.getCount()) {
            freqToPlay = minhaPilhaDeSons.getNoteAt(soundIndex);

            if (freqToPlay > 0) { // Garante que a nota não seja silêncio
                Serial.print("Tocando nota da posicao "); Serial.print(soundIndex);
                Serial.print(": "); Serial.println(freqToPlay);
                
                // O "volume" afeta a duração do som
                unsigned long currentDuration = defaultSoundDuration + (long(volumeLevel) - 5) * 20;
                if (currentDuration < 50) currentDuration = 50;
                if (currentDuration > 1000) currentDuration = 1000;

                meuAltoFalante.playTone(freqToPlay, currentDuration);
            } else {
                 Serial.print("Posicao "); Serial.print(soundIndex); Serial.println(" da pilha esta vazia (nota 0).");
                 meuAltoFalante.beep(100, 50);
            }
        } else {
            Serial.print("Nenhuma nota valida na posicao "); Serial.print(soundIndex);
            Serial.print(" da pilha (contagem: "); Serial.print(minhaPilhaDeSons.getCount()); Serial.println(")");
            meuAltoFalante.beep(100, 50); // Bip de erro/vazio
        }
    }
}
// =================================================================
// --- SETUP ---
// =================================================================
void setup() {
    Serial.begin(115200); // Lembre-se de configurar monitor_speed = 115200 no platformio.ini
    while (!Serial);
    Serial.println("Iniciando Color Piano...");

    meuSensor.begin(TCS3200Sensor::FrequencyScaling::SCALE_20_PERCENT);
    meuAltoFalante.begin();
    
    // Configura o ponteiro global para a ISR poder usá-lo
    globalButtonManager = &meuGerenciadorDeBotoes;

    // Registra a função de callback que tratará TODAS as ações de botão
    meuGerenciadorDeBotoes.onButtonPressed(handleButtonAction);
    
    // --- Adiciona TODOS os botões de POLLING ---
    // (A sua versão estava faltando os botões de som 2 a 8)
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
    
    // --- Adiciona e Ativa o Botão de INTERRUPÇÃO ---
    meuGerenciadorDeBotoes.addInterruptButton(PIN_BTN_CAPTURE, ButtonAction::CAPTURE_COLOR);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_CAPTURE), captureButtonISR, FALLING);
    
    Serial.println("Sistema pronto. Para calibrar, segure os dois botoes de volume por 1.5s.");
    meuAltoFalante.beep(1800, 200);
}

// =================================================================
// --- LOOP PRINCIPAL (com a nova lógica de detecção de combo) ---
// =================================================================
void loop() {
    // 1. A lógica de atualização dos botões e leitura do sensor sempre rodam
    meuGerenciadorDeBotoes.update(); 
    meuSensor.getCalibratedRGB(r_atual, g_atual, b_atual);

    // 2. Lógica específica por estado
    if (currentState == ProgramState::NORMAL_OPERATION) {
        // Verifica se os dois botões de volume estão pressionados para entrar em modo de calibração
        bool volUpPressed = (digitalRead(PIN_BTN_VOL_UP) == LOW);
        bool volDownPressed = (digitalRead(PIN_BTN_VOL_DOWN) == LOW);

        if (volUpPressed && volDownPressed) {
            if (!comboInProgress) {
                // Inicia a contagem quando o combo é pressionado pela primeira vez
                comboInProgress = true;
                comboPressStartTime = millis();
            } else if (millis() - comboPressStartTime > COMBO_HOLD_DURATION) {
                // Se segurou por tempo suficiente, ativa o modo de calibração
                enterCalibrationMode(); 
                comboInProgress = false; // Reseta para não reativar
            }
        } else {
            // Se um dos botões for solto, o combo é interrompido
            comboInProgress = false;
        }
    }
    // Se estiver em modo de calibração, o loop não faz nada extra, apenas
    // espera a lógica que está dentro do handleButtonAction ser acionada.
    
    delay(20);
}