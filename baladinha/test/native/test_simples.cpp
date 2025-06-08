#include <unity.h>

void setUp(void) {
    // Roda antes de cada teste
}

void tearDown(void) {
    // Roda depois de cada teste
}

void test_se_o_ambiente_de_teste_funciona(void) {
    // Este é um teste trivial para garantir que 2 é igual a 2.
    // Se isso passar, o test runner está funcionando.
    TEST_ASSERT_EQUAL(2, 2);
}

// Para PlatformIO com Unity, o 'main' é gerenciado.
// Apenas defina as funções de teste e o PlatformIO as encontrará
// se você chamar RUN_TEST. Para garantir, vamos adicionar um main.
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_se_o_ambiente_de_teste_funciona);
    return UNITY_END();
}