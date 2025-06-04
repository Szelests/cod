#include "tcs3200.hpp"

TCS3200::TCS3200(uint8_t S0, uint8_t S1, uint8_t S2, uint8_t S3, uint8_t OUT)
: _S0(S0), _S1(S1), _S2(S2), _S3(S3), _OUT(OUT) {}

void TCS3200::begin() 
{
    pinMode(_S0, OUTPUT);
    pinMode(_S1, OUTPUT);
    pinMode(_S2, OUTPUT);
    pinMode(_S3, OUTPUT);
    pinMode(_OUT, INPUT);

    digitalWrite(_S0, HIGH);
    digitalWrite(_S1, LOW);
}

void TCS3200::readColor(uint16_t &red, uint16_t &green, uint16_t &blue) 
{
    digitalWrite(_S2, LOW);
    digitalWrite(_S3, LOW);
    red = pulseIn(_OUT, LOW);

    digitalWrite(_S2, HIGH);
    digitalWrite(_S3, HIGH);
    green = pulseIn(_OUT, LOW);

    digitalWrite(_S2, LOW);
    digitalWrite(_S3, HIGH);
    blue = pulseIn(_OUT, LOW);
}