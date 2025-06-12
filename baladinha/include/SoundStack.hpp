#ifndef SOUND_STACK_HPP
#define SOUND_STACK_HPP

#include <stddef.h>
#include <stdint.h>

// Struct para guardar a frequência E a cor juntas.
struct SoundData {
    uint16_t frequency = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

class SoundStack {
public:
    SoundStack();

    // Recebe a struct completa para adicionar à pilha
    void push(const SoundData& newData);

    // Reseta a pilha
    void reset();

    // Retorna a struct completa de uma posição
    SoundData getSoundDataAt(size_t index) const;

    size_t getCount() const;
    size_t getMaxSize() const;

private:
    // O tamanho da pilha agora é 4 para corresponder aos 4 botões de som/LEDs
    static const size_t STACK_SIZE = 4;
    
    // O array interno agora armazena objetos SoundData
    SoundData _notes[STACK_SIZE];
    
    size_t _count;
};

#endif // SOUND_STACK_HPP