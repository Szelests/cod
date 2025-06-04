#include <Arduino.h>
#include "TCS3200Sensor.hpp" // Inclui a nossa classe do sensor

// --- Configuração dos Pinos do Sensor ---
                            //MARROM CLA - GND
                            //AZUL CLA - VCC
const uint8_t PIN_S0 = 3;   // AZUL ESC
const uint8_t PIN_S1 = 4;   // LARANJA
const uint8_t PIN_S2 = 5;   // BRANCO
const uint8_t PIN_S3 = 6;   // VERDE CLA
const uint8_t PIN_OUT = 7;  // VERDE ESC 

// --- Configuração do Pino do Botão ---
// Use um pino que suporte interrupções no Arduino Uno (pinos 2 ou 3)
const uint8_t PIN_BOTAO = 2; // MARRO ESC

// Cria uma instância da nossa classe de sensor
TCS3200Sensor meuSensor(PIN_S0, PIN_S1, PIN_S2, PIN_S3, PIN_OUT, 15);

// Variáveis para armazenar a cor lida continuamente
int r_atual, g_atual, b_atual;

// Variáveis para armazenar a cor capturada pelo botão
int r_capturado, g_capturado, b_capturado;

// Flag para indicar que o botão foi pressionado (deve ser volatile)
volatile bool flagCapturarCor = false;

// Variável para debounce do botão na ISR (opcional, mas bom para estabilidade)
volatile unsigned long ultimaInterrupcao = 0;
const unsigned long debounceDelay = 200; // 200 ms para debounce

/**
 * @brief Rotina de Serviço de Interrupção (ISR) para o botão.
 * Esta função será chamada quando o botão for pressionado.
 * Para Arduino Uno, não é necessário IRAM_ATTR.
 */
void ISR_botaoPressionado() {
  unsigned long agora = millis();
  if (agora - ultimaInterrupcao > debounceDelay) { // Debounce simples
    flagCapturarCor = true;
    ultimaInterrupcao = agora;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Iniciando sistema com TCS3200 e botao de captura...");

  meuSensor.begin(TCS3200Sensor::FrequencyScaling::SCALE_20_PERCENT);
  
  // Opcional: Carregar calibração salva
  // meuSensor.setWhiteBalance(val_r_branco, val_g_branco, val_b_branco);
  // meuSensor.setBlackBalance(val_r_preto, val_g_preto, val_b_preto);
  meuSensor.calibrate(); // Realiza a calibração interativa

  // Configura o pino do botão como entrada com pull-up interno
  // (Conecte o botão entre o pino e o GND)
  pinMode(PIN_BOTAO, INPUT_PULLUP);

  // Anexa a interrupção ao pino do botão
  // digitalPinToInterrupt(PIN_BOTAO) converte o número do pino para o número da interrupção
  // FALLING: aciona a interrupção quando o pino vai de HIGH para LOW (botão pressionado com pull-up)
  attachInterrupt(digitalPinToInterrupt(PIN_BOTAO), ISR_botaoPressionado, FALLING);

  Serial.println("Sistema pronto. Pressione o botao para capturar a cor.");
}

void loop() {
  // 1. Lê continuamente (ou frequentemente) a cor do sensor
  meuSensor.getCalibratedRGB(r_atual, g_atual, b_atual);

  // Opcional: Mostrar a cor que está sendo lida em tempo real
  // Serial.print("Lendo... R: "); Serial.print(r_atual);
  // Serial.print("\tG: "); Serial.print(g_atual);
  // Serial.print("\tB: "); Serial.println(b_atual);

  // 2. Verifica se a flag de captura foi acionada pela interrupção
  if (flagCapturarCor) {
    // Copia a última cor lida para as variáveis de cor capturada
    r_capturado = r_atual;
    g_capturado = g_atual;
    b_capturado = b_atual;

    // Limpa a flag para a próxima captura
    flagCapturarCor = false;

    // Informa ao usuário que a cor foi capturada e qual foi
    Serial.println("------------------------------------");
    Serial.print("COR CAPTURADA: ");
    Serial.print("R="); Serial.print(r_capturado);
    Serial.print(", G="); Serial.print(g_capturado);
    Serial.print(", B="); Serial.println(b_capturado);
    Serial.println("------------------------------------");
  }

  // Pequeno delay para não sobrecarregar o loop, mas permitir leituras rápidas.
  // Ajuste conforme necessário.
  delay(50); 
}