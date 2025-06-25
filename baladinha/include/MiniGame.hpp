#ifndef MINIGAME_HPP
#define MINIGAME_HPP

#include "LedManager.hpp"
#include "DisplayManager.hpp"
#include "PWMSpeaker.hpp"
#include "SoundStack.hpp"
#include "ButtonManager.hpp" // <<< CORREÇÃO: Incluído para conhecer o ButtonAction

class MiniGame {
public:
    MiniGame(LedManager& leds, DisplayManager& display, PWMSpeaker& speaker, SoundStack& soundStack);
    void start();
    void update();
    void handleButtonPress(ButtonAction action);
    bool isRunning() const;
private:
    // CORREÇÃO: Ordem dos membros corrigida para evitar warnings
    LedManager& _leds;
    DisplayManager& _display;
    PWMSpeaker& _speaker;
    SoundStack& _soundStack;

    enum class GameState { INACTIVE, PREVIEW, PROMPT_TO_PLAY, CPU_TURN, PLAYER_TURN, WIN, LOSE };
    GameState _currentState;

    uint8_t _sequence[4];
    uint8_t _currentRound, _playerInputStep, _previewCount, _previewStep;
    unsigned long _nextStepTime;
    void _generateSequence(); void _startPreview(); void _startCpuTurn();
    void _playerCorrect(); void _playerWrong(); void _playerWin();
};
#endif