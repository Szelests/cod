#include "SoundStack.hpp"

SoundStack::SoundStack() : _count(0) {}

void SoundStack::push(const SoundData& newData) {
    if (newData.frequency == 0) return;
    for (size_t i = STACK_SIZE - 1; i > 0; --i) {
        _notes[i] = _notes[i - 1];
    }
    _notes[0] = newData;
    if (_count < STACK_SIZE) _count++;
}

SoundData SoundStack::getSoundDataAt(size_t index) const {
    return (index >= _count) ? SoundData() : _notes[index];
}

void SoundStack::reset() {
    _count = 0;
    for (size_t i = 0; i < STACK_SIZE; ++i) _notes[i] = SoundData();
}

size_t SoundStack::getCount() const { return _count; }