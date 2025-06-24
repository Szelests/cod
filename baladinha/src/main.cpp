#include "ColorPiano.hpp"

// Cria a única instância do nosso projeto. É a única variável global.
ColorPiano meuColorPiano;

// A ISR global agora chama o método wrapper estático da classe
void captureButtonISR() {
    ColorPiano::isr_wrapper();
}

// O setup global apenas chama o setup do nosso objeto
void setup() {
    meuColorPiano.setup();
}

// O loop global apenas chama o loop do nosso objeto
void loop() {
    meuColorPiano.loop();
}