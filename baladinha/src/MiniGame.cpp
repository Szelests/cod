#include "MiniGame.hpp"
#include <Arduino.h>

MiniGame::MiniGame(LedManager& leds, DisplayManager& display, PWMSpeaker& speaker, SoundStack& soundStack) :
    _leds(leds), _display(display), _speaker(speaker), _soundStack(soundStack),
    _currentState(GameState::INACTIVE) {}

bool MiniGame::isRunning() const { return _currentState != GameState::INACTIVE; }

void MiniGame::start() {
    if (_soundStack.getCount() < 4) {
        _currentState = GameState::INACTIVE;
        return;
    }
    _generateSequence();
    _startPreview();
}

void MiniGame::update() {
    if (_currentState == GameState::INACTIVE) return;
    
    _leds.update();
    unsigned long now = millis();
    if (now < _nextStepTime) return;

    switch (_currentState) {
        case GameState::PREVIEW:
            if (_previewStep < 4) {
                SoundData data = _soundStack.getSoundDataAt(_previewStep);
                _leds.displayColorOnOne(_previewStep, data.r, data.g, data.b, 250);
                _speaker.playTone(data.frequency, 250);
                _previewStep++;
                _nextStepTime = now + 500;
            } else {
                _previewStep = 0; _previewCount++;
                if (_previewCount >= 4) {
                    _currentState = GameState::PROMPT_TO_PLAY;
                    _display.showGamePrompt(); // << CORREÇÃO
                } else {
                    _nextStepTime = now + 700;
                }
            }
            break;

        case GameState::CPU_TURN:
            if (_previewStep < _currentRound) {
                uint8_t seqIndex = _sequence[_previewStep];
                SoundData data = _soundStack.getSoundDataAt(seqIndex);
                _leds.displayColorOnOne(seqIndex, data.r, data.g, data.b, 350);
                _speaker.playTone(data.frequency, 350);
                _previewStep++;
                _nextStepTime = now + 700;
            } else {
                _currentState = GameState::PLAYER_TURN;
                _display.showGameYourTurn(); // << CORREÇÃO
            }
            break;
            
        default: break;
    }
}

void MiniGame::handleButtonPress(ButtonAction action) {
    if (_currentState == GameState::PROMPT_TO_PLAY) {
        if (action == ButtonAction::VOLUME_UP) { _startCpuTurn(); } 
        else if (action == ButtonAction::VOLUME_DOWN) { 
            _display.showRealtimeMode();
            _currentState = GameState::INACTIVE; 
        }
    } else if (_currentState == GameState::PLAYER_TURN) {
        int soundIndex = -1;
        if(action >= ButtonAction::PLAY_SOUND_1 && action <= ButtonAction::PLAY_SOUND_4) {
            soundIndex = (int)action - (int)ButtonAction::PLAY_SOUND_1;
        } else { return; }

        if (soundIndex == _sequence[_playerInputStep]) { _playerCorrect(); } 
        else { _playerWrong(); }
    }
}

void MiniGame::_generateSequence() {
    for(int i = 0; i < 4; i++) _sequence[i] = i;
    for(int i = 0; i < 4; i++) {
        int j = random(i, 4);
        uint8_t temp = _sequence[i]; _sequence[i] = _sequence[j]; _sequence[j] = temp;
    }
}

void MiniGame::_startPreview() {
    _currentState = GameState::PREVIEW; _previewCount = 0; _previewStep = 0;
    _nextStepTime = millis(); _display.showGameMemorize();
}

void MiniGame::_startCpuTurn() {
    _currentState = GameState::CPU_TURN; _playerInputStep = 0; _previewStep = 0;
    _nextStepTime = millis() + 1000; _display.showGameMemorize();
}

void MiniGame::_playerCorrect() {
    uint8_t seqIndex = _sequence[_playerInputStep];
    SoundData data = _soundStack.getSoundDataAt(seqIndex);
    _leds.displayColorOnOne(seqIndex, data.r, data.g, data.b, 150);
    _speaker.playTone(data.frequency, 150);
    
    _playerInputStep++;
    if (_playerInputStep >= _currentRound) {
        if (_currentRound >= 4) { _playerWin(); } 
        else { _currentRound++; _startCpuTurn(); }
    }
}

void MiniGame::_playerWrong() {
    _currentState = GameState::LOSE; _display.showGameLose();
    _speaker.playTone(150, 500); delay(2000);
    _display.showRealtimeMode(); _currentState = GameState::INACTIVE;
}

void MiniGame::_playerWin() {
    _currentState = GameState::WIN; _display.showGameWin();
    _speaker.playTone(880, 100); delay(150); _speaker.playTone(988, 100); delay(150); _speaker.playTone(1047, 300);
    delay(2000); _display.showRealtimeMode(); _currentState = GameState::INACTIVE;
}