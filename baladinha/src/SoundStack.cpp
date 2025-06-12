#include "SoundStack.hpp" // Inclui a definição correta da nossa classe

SoundStack::SoundStack() : _count(0) {
    // O construtor padrão da struct SoundData já inicializa tudo com 0,
    // mas podemos garantir isso explicitamente se quisermos.
    for (size_t i = 0; i < STACK_SIZE; ++i) {
        _notes[i] = SoundData(); // Preenche com SoundData vazio
    }
}

// O método push agora aceita um objeto SoundData
void SoundStack::push(const SoundData& newData) {
    // Não adiciona "silêncio" (frequência 0) à pilha
    if (newData.frequency == 0) {
        return;
    }

    // Desloca todos os elementos existentes para a "direita"
    for (size_t i = STACK_SIZE - 1; i > 0; --i) {
        _notes[i] = _notes[i - 1];
    }
    
    // Adiciona o novo objeto SoundData no topo da pilha
    _notes[0] = newData;

    // Incrementa a contagem de elementos válidos se a pilha ainda não estava cheia
    if (_count < STACK_SIZE) {
        _count++;
    }
}

// O método getNoteAt agora retorna um objeto SoundData completo
SoundData SoundStack::getSoundDataAt(size_t index) const {
    if (index >= _count) {
        return SoundData(); // Retorna um SoundData vazio/padrão se o índice for inválido
    }
    return _notes[index];
}

void SoundStack::reset() {
    _count = 0;
    for (size_t i = 0; i < STACK_SIZE; ++i) {
        _notes[i] = SoundData(); // Limpa a pilha com SoundData vazio
    }
}

size_t SoundStack::getCount() const {
    return _count;
}

size_t SoundStack::getMaxSize() const {
    return STACK_SIZE;
}