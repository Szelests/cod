#ifndef SOUND_STACK_HPP
#define SOUND_STACK_HPP

#include <stddef.h> // Para usar size_t, um tipo padrão para tamanhos e índices
#include <stdint.h> // Para usar uint16_t para as frequências

class SoundStack {
public:
    /**
     * @brief Construtor. Cria uma pilha de sons vazia.
     */
    SoundStack();

    /**
     * @brief Adiciona uma nova nota ao topo da pilha (posição 0).
     * Se a pilha estiver cheia, a nota mais antiga (na última posição) é descartada.
     * @param newNote A frequência da nova nota a ser adicionada.
     */
    void push(uint16_t newNote);
    
    void reset();

    /**
     * @brief Obtém a nota em um índice específico da pilha.
     * O índice 0 é a nota mais recente.
     * @param index A posição na pilha (0 a 7).
     * @return A frequência da nota, ou 0 se o índice for inválido ou a posição estiver vazia.
     */
    uint16_t getNoteAt(size_t index) const;

    /**
     * @brief Obtém o número atual de notas válidas na pilha.
     * @return O número de notas (de 0 a 8).
     */
    size_t getCount() const;

    /**
     * @brief Obtém o tamanho máximo da pilha.
     * @return O tamanho máximo (sempre 8).
     */
    size_t getMaxSize() const;

private:
    static const size_t STACK_SIZE = 8; // Define o tamanho fixo da pilha
    uint16_t _notes[STACK_SIZE];         // Array interno para armazenar as notas
    size_t _count;                       // Contador de quantas notas válidas existem
};

#endif // SOUND_STACK_HPP