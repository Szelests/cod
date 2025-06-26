#ifndef SOUND_STACK_HPP
#define SOUND_STACK_HPP

#include <stddef.h>
#include <stdint.h>

struct SoundData {
    uint16_t frequency = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

class SoundStack {
public:
    SoundStack();
    void push(const SoundData& newData);
    void reset();
    SoundData getSoundDataAt(size_t index) const;
    size_t getCount() const;
private:
    static const size_t STACK_SIZE = 4;
    SoundData _notes[STACK_SIZE];
    size_t _count;
};
#endif