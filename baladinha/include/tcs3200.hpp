#ifndef TCS3200_H
#define TCS3200_H

#include <Arduino.h>

class TCS3200 
{
public:
    TCS3200(uint8_t S0, uint8_t S1, uint8_t S2, uint8_t S3, uint8_t OUT);
    void begin();
    void readColor(uint16_t &red, uint16_t &green, uint16_t &blue); 

private: 
    uint16_t _S0, _S1, _S2, _S3, _OUT;
};

#endif