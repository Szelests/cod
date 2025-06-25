#ifndef DISPLAY_MANAGER_HPP
#define DISPLAY_MANAGER_HPP

#include <LiquidCrystal_I2C.h>
#include "SoundStack.hpp"

class DisplayManager {
public:
    // CONSTRUTOR SIMPLIFICADO
    DisplayManager(uint8_t address, uint8_t cols, uint8_t rows);

    void begin();
    
    // Todos os outros métodos show... continuam os mesmos
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

private:
    LiquidCrystal_I2C _lcd;
};

#endif