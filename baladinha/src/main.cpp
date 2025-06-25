#include "ColorPiano.hpp"

ColorPiano meuColorPiano;

void captureButtonISR() {
    ColorPiano::isr_wrapper();
}

void setup() { meuColorPiano.setup(); }
void loop() { meuColorPiano.loop(); }