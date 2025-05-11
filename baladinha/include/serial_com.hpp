#ifndef SERIAL_COM_H
#define SERIAL_COM_H

#include <Arduino.h>

class SerialCom 
{
public:
    void begin();
    void sendColor(uint16_t red, uint16_t green, uint16_t blue);
};

#endif