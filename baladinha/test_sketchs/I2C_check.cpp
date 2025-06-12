/**
 * @name I2C Scanner
 * @brief Sketch de diagnóstico para barramento I2C.
 * * Este código varre a faixa de endereços I2C de 1 a 127 e imprime no Monitor Serial
 * o endereço de qualquer dispositivo que responder.
 * * É uma ferramenta essencial para verificar conexões e descobrir os endereços de
 * módulos I2C como displays LCD, sensores, e expansores de porta (PCF8574).
 * * @author Baseado no scanner I2C de Nick Gammon e do playground do Arduino.
 * @date 12 de junho de 2025
 */

#include <Wire.h>

void setup() {
  // Inicia a comunicação serial a uma velocidade alta para resultados rápidos
  Serial.begin(115200);
  while (!Serial); // Espera a porta serial conectar (necessário para placas como Leonardo, mas boa prática para todas)

  Serial.println("\n--- Scanner de Barramento I2C ---");
  Serial.println("Procurando por dispositivos...");

  // Inicia o barramento I2C como mestre
  Wire.begin();

  byte device_count = 0;

  // O loop varre todos os endereços possíveis (7-bit)
  for (byte address = 1; address < 127; address++) {
    // A biblioteca Wire usa o buffer para enviar o endereço.
    // Wire.endTransmission() envia os dados e espera uma resposta (ACK).
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      // error = 0 significa que um dispositivo respondeu (ACK).
      Serial.print("Dispositivo I2C encontrado no endereco 0x");
      if (address < 16) {
        Serial.print("0"); // Adiciona um zero à esquerda para formatação (ex: 0x07)
      }
      Serial.println(address, HEX); // Imprime o endereço em formato hexadecimal

      device_count++;
      
    } else if (error == 4) {
      // error = 4 significa um erro desconhecido no barramento.
      // Pode indicar um problema de fiação (ex: SDA/SCL trocados ou mau contato).
      Serial.print("Erro desconhecido no endereco 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }    
    // Se error for 2, significa que nenhum dispositivo respondeu (NACK), o que é normal para endereços vazios.
  }

  // Imprime um resumo no final
  Serial.println("---------------------------------");
  if (device_count == 0) {
    Serial.println("Nenhum dispositivo I2C encontrado. Verifique a fiacao!");
  } else {
    Serial.print("Scan concluido. Encontrado(s) ");
    Serial.print(device_count);
    Serial.println(" dispositivo(s).");
  }
}

// O loop fica vazio pois o scan só precisa rodar uma vez.
void loop() {
  // Nada aqui.
}