/*
   I2C LCD PINOUT FINDER / CONFIGURATION SCANNER
   Este código testa várias configurações de pinos comuns para módulos
   I2C LCD e ajuda a encontrar a correta para o seu hardware.
   Requer a biblioteca "New-LiquidCrystal" de F. Malpartida.
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Endereço do seu LCD
#define I2C_ADDR 0x23

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Iniciando Caçador de Configuracao de Pinos I2C LCD...");
  Serial.println("Observe a tela do seu LCD atentamente!");
  delay(1000);

  // Testa a configuração mais comum
  testarConfig(I2C_ADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE, "Config #1 (Comum)");

  // Testa uma variação comum
  testarConfig(I2C_ADDR, 0, 1, 2, 4, 5, 6, 7, 3, POSITIVE, "Config #2 (Variacao)");
  
  // Testa outra variação encontrada em alguns módulos azuis
  testarConfig(I2C_ADDR, 4, 5, 6, 0, 1, 2, 3, 7, NEGATIVE, "Config #3 (Módulos Azuis)");

  Serial.println("-------------------------------------");
  Serial.println("Teste concluido.");
  Serial.println("Se nenhuma config funcionou, verifique a solda e o contraste.");
}

void loop() {
  // Nada aqui
}

// Função que testa uma configuração específica
void testarConfig(uint8_t addr, uint8_t en, uint8_t rw, uint8_t rs, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t bl, uint8_t blpol, const char* configName) {
  Serial.println("-------------------------------------");
  Serial.print("Testando ");
  Serial.println(configName);
  
  // Tenta criar o objeto LCD com a configuração atual
  LiquidCrystal_I2C lcd(addr, en, rw, rs, d4, d5, d6, d7, bl, blpol);
  
  // Tenta inicializar e escrever
  lcd.begin(16, 2); // Para esta biblioteca, .begin() é usado
  lcd.clear();
  lcd.print("FUNCIONOU!");
  lcd.setCursor(0,1);
  lcd.print(configName);

  // Se o texto apareceu na sua tela, esta é a configuração correta!
  Serial.println(" -> Se voce ve texto na tela AGORA, esta e a config correta!");
  
  delay(5000); // Espera 5 segundos para você poder ver e anotar
}