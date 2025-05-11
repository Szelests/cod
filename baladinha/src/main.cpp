#include <Arduino.h>
#include "tcs3200.h"
#include "serial_com.h"

// Define the IOs
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define OUT 8

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

