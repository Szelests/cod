#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

// Este é o nosso cabeçalho de mock central.
// Nos testes nativos, quando um arquivo fonte fizer #include <Arduino.h>,
// o compilador encontrará ESTE arquivo primeiro, graças à build_flag "-I test/native/mocks/".
// Este arquivo então inclui nossa implementação de mock detalhada.

#include "ArduinoFake.h" 

#endif // MOCK_ARDUINO_H