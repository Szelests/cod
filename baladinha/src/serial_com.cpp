#include "serial_com.h"

void SerialCom::begin()
{
    Serial.begin(9600);
}

void SerialCom::sendColor(uint16_t red, uint16_t green, uint16_t blue)
{
    Serial.print(" R:");
    Serial.print(red);
    Serial.print(" G:");
    Serial.print(green);
    Serial.print(" B:");
    Serial.println(blue);
}