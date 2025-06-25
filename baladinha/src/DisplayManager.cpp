#include "DisplayManager.hpp"

DisplayManager::DisplayManager(uint8_t address, uint8_t cols, uint8_t rows)
    : _lcd(address, cols, rows)  {}

void DisplayManager::begin() {
    _lcd.init();
    _lcd.backlight();
}

void DisplayManager::showWelcome() {
    _lcd.clear();
    _lcd.setCursor(0, 0); _lcd.print("==DISCO MULTISSENSORIAL==");
    _lcd.setCursor(0, 1); _lcd.print("Iniciando...");
}

void DisplayManager::showReadyToCalibrate() {
    _lcd.clear();
    _lcd.print("Segure + e -");
    _lcd.setCursor(0, 1);
    _lcd.print("p/ calibrar");
}

void DisplayManager::showCalibrationMode() {
    _lcd.clear();
    _lcd.print("Modo Calibracao");
    _lcd.setCursor(0, 1);
    _lcd.print("Branco -> Vol(+)");
}

void DisplayManager::showCalibratingWhite() {
    _lcd.clear(); _lcd.print("Branco OK!");
    _lcd.setCursor(0, 1); _lcd.print("Preto -> Vol(-)");
}

void DisplayManager::showCalibratingBlack() {
    _lcd.clear(); _lcd.print("Calibrado!");
}

void DisplayManager::showCalibrationDone() {
    // Adicionando a implementação que faltava
    showCalibratingBlack(); // Pode reutilizar a mesma lógica
}

void DisplayManager::showRealtimeMode() {
    _lcd.clear();
    _lcd.print("Aponte o sensor");
    _lcd.setCursor(0, 1);
    _lcd.print("ou toque um som");
}

void DisplayManager::showPlayback(const SoundData& data, const char* colorName) {
    _lcd.clear();
    _lcd.setCursor(0, 0); _lcd.print("Cor: "); _lcd.print(colorName);
    _lcd.setCursor(0, 1); _lcd.print("Nota: "); _lcd.print(data.frequency); _lcd.print("Hz");
}

void DisplayManager::showCaptured(const char* colorName) {
    _lcd.clear();
    _lcd.setCursor(0, 0); _lcd.print("Capturado:");
    _lcd.setCursor(0, 1); _lcd.print(colorName);
}

void DisplayManager::showEmptySlot() {
    _lcd.clear();
    _lcd.print("Slot Vazio");
}

void DisplayManager::showVolume(uint8_t level){
    _lcd.clear();
    _lcd.print("Volume: ");
    _lcd.print(level);
}