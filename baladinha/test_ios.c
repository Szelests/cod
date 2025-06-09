/*
 * Sketch de Teste de Hardware: Botões e Som
 * * Objetivo: Verificar a fiação de 11 botões e do alto-falante.
 * - Cada botão, ao ser pressionado, deve imprimir seu nome no Monitor Serial.
 * - Cada botão deve tocar uma nota musical diferente para confirmar que o
 * alto-falante está funcionando e conectado ao pino correto.
 * * Instruções:
 * 1. Carregue este código no seu Arduino Uno.
 * 2. Abra o Monitor Serial e configure a velocidade para 115200 baud.
 * 3. Pressione cada um dos 11 botões físicos e observe a mensagem no monitor
 * e ouça o som correspondente.
 */

// --- Configuração dos Pinos (Exatamente como no seu projeto principal) ---
#define PIN_S0 4
#define PIN_S1 5
#define PIN_S2 6
#define PIN_S3 7
#define PIN_OUT_SENSOR 8
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

// Array com todos os pinos de botão para facilitar a configuração
const int allButtonPins[] = {
  PIN_BTN_SOUND_1, PIN_BTN_SOUND_2, PIN_BTN_SOUND_3, PIN_BTN_SOUND_4,
  PIN_BTN_SOUND_5, PIN_BTN_SOUND_6, PIN_BTN_SOUND_7, PIN_BTN_SOUND_8,
  PIN_BTN_VOL_UP, PIN_BTN_VOL_DOWN, PIN_BTN_CAPTURE
};
const int numButtons = sizeof(allButtonPins) / sizeof(allButtonPins[0]);

// Frequências de teste únicas para cada botão (Escala de Dó Maior)
const int testFrequencies[] = {
  262, // C4 (Som 1)
  294, // D4 (Som 2)
  330, // E4 (Som 3)
  349, // F4 (Som 4)
  392, // G4 (Som 5)
  440, // A4 (Som 6)
  494, // B4 (Som 7)
  523, // C5 (Som 8)
  659, // E5 (Vol Up)
  587, // D5 (Vol Down)
  784  // G5 (Capture)
};

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("--- Testador de Hardware: Botoes e Som ---");
  Serial.println("Pressione cada botao para testar...");

  // Configura todos os pinos de botão como entrada com pull-up interno
  for (int i = 0; i < numButtons; i++) {
    pinMode(allButtonPins[i], INPUT_PULLUP);
  }

  // Configura o pino do alto-falante como saída
  pinMode(PIN_SPEAKER, OUTPUT);
}

void loop() {
  // O loop verifica cada botão individualmente.
  // A lógica 'while (digitalRead(...) == LOW);' garante que o som toque apenas uma vez por pressionamento.

  if (digitalRead(PIN_BTN_SOUND_1) == LOW) {
    Serial.println("Botao Pressionado: SOUND_1 (Pino 10)");
    tone(PIN_SPEAKER, testFrequencies[0], 200); // Toca a primeira nota
    while (digitalRead(PIN_BTN_SOUND_1) == LOW); // Espera o botão ser solto
    delay(50); // Anti-bounce para a liberação do botão
  }
  if (digitalRead(PIN_BTN_SOUND_2) == LOW) {
    Serial.println("Botao Pressionado: SOUND_2 (Pino 11)");
    tone(PIN_SPEAKER, testFrequencies[1], 200);
    while (digitalRead(PIN_BTN_SOUND_2) == LOW);
    delay(50);
  }
  if (digitalRead(PIN_BTN_SOUND_3) == LOW) {
    Serial.println("Botao Pressionado: SOUND_3 (Pino 12)");
    tone(PIN_SPEAKER, testFrequencies[2], 200);
    while (digitalRead(PIN_BTN_SOUND_3) == LOW);
    delay(50);
  }
  if (digitalRead(PIN_BTN_SOUND_4) == LOW) {
    Serial.println("Botao Pressionado: SOUND_4 (Pino 13)");
    tone(PIN_SPEAKER, testFrequencies[3], 200);
    while (digitalRead(PIN_BTN_SOUND_4) == LOW);
    delay(50);
  }
  if (digitalRead(PIN_BTN_SOUND_5) == LOW) {
    Serial.println("Botao Pressionado: SOUND_5 (Pino A5)");
    tone(PIN_SPEAKER, testFrequencies[4], 200);
    while (digitalRead(PIN_BTN_SOUND_5) == LOW);
    delay(50);
  }
  if (digitalRead(PIN_BTN_SOUND_6) == LOW) {
    Serial.println("Botao Pressionado: SOUND_6 (Pino A4)");
    tone(PIN_SPEAKER, testFrequencies[5], 200);
    while (digitalRead(PIN_BTN_SOUND_6) == LOW);
    delay(50);
  }
  if (digitalRead(PIN_BTN_SOUND_7) == LOW) {
    Serial.println("Botao Pressionado: SOUND_7 (Pino A3)");
    tone(PIN_SPEAKER, testFrequencies[6], 200);
    while (digitalRead(PIN_BTN_SOUND_7) == LOW);
    delay(50);
  }
  if (digitalRead(PIN_BTN_SOUND_8) == LOW) {
    Serial.println("Botao Pressionado: SOUND_8 (Pino A2)");
    tone(PIN_SPEAKER, testFrequencies[7], 200);
    while (digitalRead(PIN_BTN_SOUND_8) == LOW);
    delay(50);
  }
  if (digitalRead(PIN_BTN_VOL_UP) == LOW) {
    Serial.println("Botao Pressionado: VOL_UP (Pino A1)");
    tone(PIN_SPEAKER, testFrequencies[8], 200);
    while (digitalRead(PIN_BTN_VOL_UP) == LOW);
    delay(50);
  }
  if (digitalRead(PIN_BTN_VOL_DOWN) == LOW) {
    Serial.println("Botao Pressionado: VOL_DOWN (Pino A0)");
    tone(PIN_SPEAKER, testFrequencies[9], 200);
    while (digitalRead(PIN_BTN_VOL_DOWN) == LOW);
    delay(50);
  }
  if (digitalRead(PIN_BTN_CAPTURE) == LOW) {
    Serial.println("Botao Pressionado: CAPTURE (Pino 2)");
    tone(PIN_SPEAKER, testFrequencies[10], 200);
    while (digitalRead(PIN_BTN_CAPTURE) == LOW);
    delay(50);
  }
}