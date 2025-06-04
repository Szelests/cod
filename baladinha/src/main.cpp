#include <Arduino.h>
#include "tcs3200.hpp"
#include "serial_com.hpp"

// Define the IOs
#define S0 4  // AZUL ESCURO
#define S1 5  // LARANJA
#define S2 6  // BRANCO
#define S3 7  // VERDE CLARO
#define OUT 8 // VERDE ESCURO
              // AZUL CLARO - VCC 5V
              // MARROM CLARO - GND

// Module instances
TCS3200 colorSensor(S0, S1, S2, S3, OUT);
SerialCom serialCom;

void setup() {
  colorSensor.begin();
  serialCom.begin();
}

void loop() {
  uint16_t red, grenn, blue;
  colorSensor.readColor(red, grenn, blue);
  serialCom.sendColor(red, grenn, blue);
  delay(1000);
}

