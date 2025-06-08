#include <unity.h>
#include "ArduinoFake.h"
#include "ButtonManager.hpp" // Usa o caminho relativo correto para encontrar o .hpp

// Variáveis de teste
static ButtonAction lastActionReceived;
static bool callbackWasCalled;

// Callback de teste
void testButtonCallback(ButtonAction action) {
    callbackWasCalled = true;
    lastActionReceived = action;
}

// Funções de setup/teardown do Unity
void setUp(void) {
    resetArduinoFakeState();
    callbackWasCalled = false;
    lastActionReceived = ButtonAction::NONE;
}
void tearDown(void) {}

// Teste para o botão de captura
void test_capture_button_triggers_correct_action(void) {
    ButtonManager bm;
    bm.onButtonPressed(testButtonCallback);
    
    const uint8_t CAPTURE_PIN = 2;
    bm.addButton(CAPTURE_PIN, ButtonAction::CAPTURE_COLOR);

    setDigitalReadResult(CAPTURE_PIN, HIGH);
    bm.update();
    
    setDigitalReadResult(CAPTURE_PIN, LOW);
    bm.update();

    advanceMillis(60); 
    bm.update();

    TEST_ASSERT_TRUE(callbackWasCalled);
    TEST_ASSERT_EQUAL(ButtonAction::CAPTURE_COLOR, lastActionReceived);
}

// Ponto de entrada para o test runner
void process() {
    UNITY_BEGIN();
    RUN_TEST(test_capture_button_triggers_correct_action);
    UNITY_END();
}

#ifdef ARDUINO
// Este bloco não será usado em testes nativos
void setup() { process(); }
void loop() {}
#else
// Este bloco será usado para o teste nativo no seu PC
int main(int argc, char **argv) {
    process();
    return 0;
}
#endif