#include "SoundStack.hpp" // Inclui a definição da nossa classe

SoundStack::SoundStack() : _count(0) {
    // Inicializa a pilha com 0s (silêncio)
    for (size_t i = 0; i < STACK_SIZE; ++i) {
        _notes[i] = 0;
    }
}

void SoundStack::reset() {
    _count = 0;
    for (size_t i = 0; i < STACK_SIZE; ++i) {
        _notes[i] = 0; // Preenche com silêncio
    }
}

void SoundStack::push(uint16_t newNote) {
    if (newNote == 0) {
        // Não adiciona "silêncio" à pilha
        return;
    }

    // Desloca todos os elementos existentes uma posição para a "direita"
    // para abrir espaço para o novo no início (índice 0).
    for (size_t i = STACK_SIZE - 1; i > 0; --i) {
        _notes[i] = _notes[i - 1];
    }
    
    // Adiciona a nova nota no topo da pilha (índice 0)
    _notes[0] = newNote;

    // Incrementa a contagem de elementos válidos se a pilha ainda não estava cheia
    if (_count < STACK_SIZE) {
        _count++;
    }
}

uint16_t SoundStack::getNoteAt(size_t index) const {
    // Verifica se o índice solicitado é válido e se essa posição já foi preenchida
    if (index >= _count) {
        return 0; // Retorna 0 (silêncio) para posições inválidas ou vazias
    }
    return _notes[index];
}

size_t SoundStack::getCount() const {
    return _count;
}

size_t SoundStack::getMaxSize() const {
    return STACK_SIZE;
}