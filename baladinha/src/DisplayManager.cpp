#include "DisplayManager.hpp"
#include <Arduino.h>

DisplayManager::DisplayManager(uint8_t address, uint8_t cols, uint8_t rows) : _lcd(address, cols, rows) {}

void DisplayManager::begin() { _lcd.init(); _lcd.backlight(); }
void DisplayManager::showWelcome() { _lcd.clear(); _lcd.setCursor(0, 0); _lcd.print("Color Piano"); _lcd.setCursor(0, 1); _lcd.print("Iniciando..."); }
void DisplayManager::showReadyToCalibrate() { _lcd.clear(); _lcd.print("Segure Vol p/cal"); }
void DisplayManager::showCalibrationMode() { _lcd.clear(); _lcd.print("Modo Calibracao"); _lcd.setCursor(0, 1); _lcd.print("Branco -> Vol(+)"); }
void DisplayManager::showCalibratingWhite() { _lcd.clear(); _lcd.print("Branco OK!"); _lcd.setCursor(0, 1); _lcd.print("Preto -> Vol(-)"); }
void DisplayManager::showCalibratingBlack() { _lcd.clear(); _lcd.print("Calibrado!"); }
void DisplayManager::showCalibrationDone() { showCalibratingBlack(); }
void DisplayManager::showRealtimeMode() { _lcd.clear(); _lcd.print("Modo Leitura"); _lcd.setCursor(0, 1); _lcd.print("Aponte o sensor"); }
void DisplayManager::showPlayback(const SoundData& data, const char* colorName) { _lcd.clear(); _lcd.setCursor(0, 0); _lcd.print(colorName); _lcd.setCursor(0, 1); _lcd.print("Nota: "); _lcd.print(data.frequency); }
void DisplayManager::showCaptured(const char* colorName) { _lcd.clear(); _lcd.setCursor(0, 0); _lcd.print("Capturado:"); _lcd.setCursor(0, 1); _lcd.print(colorName); }
void DisplayManager::showEmptySlot() { _lcd.clear(); _lcd.print("Slot Vazio"); }
void DisplayManager::showVolume(uint8_t level){ _lcd.clear(); _lcd.print("Volume: "); _lcd.print(level); }
void DisplayManager::showGamePrompt() { _lcd.clear(); _lcd.print("Jogar Genius?"); _lcd.setCursor(0, 1); _lcd.print("Sim(+) Nao(-)"); }
void DisplayManager::showGameMemorize() { _lcd.clear(); _lcd.print("Memorize!"); }
void DisplayManager::showGameYourTurn() { _lcd.clear(); _lcd.print("Sua vez..."); }
void DisplayManager::showGameWin() { _lcd.clear(); _lcd.print("VOCE VENCEU!"); }
void DisplayManager::showGameLose() { _lcd.clear(); _lcd.print("ERROU!"); }
void DisplayManager::showTutorial() {
    _lcd.clear(); _lcd.print("Tutorial:"); _lcd.setCursor(0, 1); _lcd.print("Aponte p/ cores"); delay(4000);
    _lcd.clear(); _lcd.print("Botao (I) captura"); _lcd.setCursor(0, 1); _lcd.print("a cor e o som"); delay(4000);
    _lcd.clear(); _lcd.print("Botoes 1-4 tocam"); _lcd.setCursor(0, 1); _lcd.print("as notas salvas"); delay(4000);
}