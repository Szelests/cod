#include <Arduino.h>
#include <math.h> // Para fmod na conversão de Hue
#include "TCS3200Sensor.hpp"
#include "PWMSpeaker.hpp"
#include "ButtonManager.hpp"

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

#define SOUND_STACK_SIZE 8 // Tamanho da pilha LIFO para notas de som

// Instâncias das classes
TCS3200Sensor meuSensor(PIN_S0, PIN_S1, PIN_S2, PIN_S3, PIN_OUT_SENSOR);
PWMSpeaker meuAltoFalante(PIN_SPEAKER);
ButtonManager meuGerenciadorDeBotoes;

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

// Pilha LIFO para as notas
uint16_t soundStack[SOUND_STACK_SIZE] = {0}; // Inicializa com 0s (silêncio)
uint8_t soundStackCount = 0; // Número de notas válidas na pilha

// Volume (conceitual)
uint8_t volumeLevel = 5; // 0-10
unsigned long defaultSoundDuration = 300; // Duração padrão do som em ms

// ----- Funções Auxiliares -----
// Função para converter RGB para Hue
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

// Função para mapear cor para frequência
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

// Função para adicionar nota à pilha LIFO
void addNoteToSoundStack(uint16_t newNote) {
    if (newNote == 0) {
        Serial.println("Tentativa de adicionar nota 0 (silêncio) a pilha. Ignorado.");
        return;
    }

    for (int i = SOUND_STACK_SIZE - 1; i > 0; i--) {
        soundStack[i] = soundStack[i - 1];
    }
    soundStack[0] = newNote;

    if (soundStackCount < SOUND_STACK_SIZE) {
        soundStackCount++;
    }

    Serial.print("Nota adicionada a pilha: "); Serial.println(newNote);
    Serial.print("Pilha (Botao1[0] -> Botao8[7]): ");
    for(int i=0; i < soundStackCount; i++){
        Serial.print(soundStack[i]);
        if (i < soundStackCount - 1) Serial.print(", ");
    }
    Serial.println();
}

// Função de callback para tratar ações dos botões// Em src/main.cpp
void handleButtonAction(ButtonAction action) {
    Serial.print("Acao do botao: ");
    uint16_t freqToPlay = 0;
    int8_t soundIndex = -1; // Padrão para nenhum índice de som

    switch (action) {
        case ButtonAction::PLAY_SOUND_1:
            Serial.println("PLAY_SOUND_1");
            soundIndex = 0;
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

        case ButtonAction::VOLUME_UP: { // Usar chaves por boa prática
            Serial.print("VOLUME_UP. Volume: ");
            if (volumeLevel < 10) volumeLevel++;
            Serial.println(volumeLevel);
            meuAltoFalante.beep(1500 + volumeLevel * 50, 50 + volumeLevel * 5);
            break;
        }
        case ButtonAction::VOLUME_DOWN: { // Usar chaves por boa prática
            Serial.print("VOLUME_DOWN. Volume: ");
            if (volumeLevel > 0) volumeLevel--;
            Serial.println(volumeLevel);
            meuAltoFalante.beep(1000 - volumeLevel * 50, 50 - volumeLevel * 3);
            break;
        }
        case ButtonAction::CAPTURE_COLOR: { // *** CHAVES AQUI SÃO CRUCIAIS ***
            Serial.println("CAPTURE_COLOR");
            // A cor atual (r_atual, g_atual, b_atual) já foi lida no loop principal
            unsigned int mappedFreq = mapColorToFrequency(r_atual, g_atual, b_atual); // Inicialização agora dentro de um escopo seguro
            addNoteToSoundStack(mappedFreq);
            meuAltoFalante.beep(mappedFreq > 0 ? mappedFreq : 2000, 100);
            break;
        }
        case ButtonAction::NONE: { // Tratar explicitamente
            Serial.println("Nenhuma acao especifica (NONE).");
            break; 
        }
        default: { // Tratar explicitamente
            Serial.println("Acao desconhecida ou nao tratada.");
            break;
        }
    }

    // Se foi uma ação PLAY_SOUND_X, soundIndex terá sido definido para >= 0
    if (soundIndex != -1) {
        if (soundIndex < soundStackCount && soundStack[soundIndex] > 0) {
            freqToPlay = soundStack[soundIndex];
            Serial.print("Tocando nota da posicao "); Serial.print(soundIndex);
            Serial.print(" (Pilha[0] = mais nova): "); Serial.println(freqToPlay);
            
            unsigned long currentDuration = defaultSoundDuration;
            // Exemplo de como o 'volumeLevel' poderia afetar a duração (ou outro parâmetro no futuro)
            // Aumentar o volume poderia significar um som um pouco mais longo ou mais intenso.
            // Diminuir poderia significar um som mais curto.
            // Este é um ajuste simples, você pode torná-lo mais sofisticado.
            if (volumeLevel > 5) {
                currentDuration += (volumeLevel - 5) * 30; // Aumenta duração com volume
            } else if (volumeLevel < 5) {
                currentDuration -= (5 - volumeLevel) * 30; // Diminui duração com volume
            }
            if (currentDuration < 50) currentDuration = 50; // Duração mínima
            if (currentDuration > 1000) currentDuration = 1000; // Duração máxima para não bloquear demais

            meuAltoFalante.playTone(freqToPlay, currentDuration);
        } else {
            Serial.print("Nenhuma nota valida na posicao "); Serial.print(soundIndex);
            Serial.print(" da pilha (contagem: "); Serial.print(soundStackCount); Serial.println(")");
            meuAltoFalante.beep(100, 50); // Bip de erro/vazio
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Iniciando Color Piano...");

    meuSensor.begin(TCS3200Sensor::FrequencyScaling::SCALE_20_PERCENT);
    meuAltoFalante.begin();
    
    Serial.println("Calibrando sensor de cor (siga as instrucoes)...");
    meuSensor.calibrate(); 
    Serial.println("Calibracao concluida.");

    meuGerenciadorDeBotoes.onButtonPressed(handleButtonAction);

    // Adiciona os botões (conecte-os com INPUT_PULLUP, então entre o pino e GND)
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
    
    meuGerenciadorDeBotoes.addButton(PIN_BTN_CAPTURE, ButtonAction::CAPTURE_COLOR);

    Serial.println("Sistema pronto. Aponte o sensor para uma cor e pressione Capturar.");
    Serial.println("Depois, pressione os botoes de som 1-8.");
    meuAltoFalante.beep(1800, 200); // Bip de inicialização
}

void loop() {
    meuGerenciadorDeBotoes.update();
    meuSensor.getCalibratedRGB(r_atual, g_atual, b_atual);

    // Opcional: imprimir a cor lida continuamente para debug
    // Serial.print("Cor Atual R:"); Serial.print(r_atual);
    // Serial.print(" G:"); Serial.print(g_atual);
    // Serial.print(" B:"); Serial.println(b_atual);

    delay(20); // Pequeno delay para estabilidade e responsividade
}