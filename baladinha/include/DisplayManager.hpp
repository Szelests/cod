#ifndef DISPLAY_MANAGER_HPP
#define DISPLAY_MANAGER_HPP

#include <LiquidCrystal_I2C.h>
#include "SoundStack.hpp"

class DisplayManager {
public:
    DisplayManager(uint8_t address, uint8_t cols, uint8_t rows);
    void begin();
    
    // Métodos de Tela
    void showWelcome();
    void showReadyToCalibrate();
    void showCalibrationMode();
    void showCalibratingWhite();
    void showCalibratingBlack();
    void showCalibrationDone();
    void showRealtimeMode();
    void showPlayback(const SoundData& data, const char* colorName);
    void showCaptured(const char* colorName);
    void showEmptySlot();
    void showVolume(uint8_t level);

    // Métodos para o Minigame
    void showGamePrompt();
    void showGameMemorize();
    void showGameYourTurn();
    void showGameWin();
    void showGameLose();
    void showTutorial();

private:
    LiquidCrystal_I2C _lcd;
};
#endif