#include "TCS3200Sensor.hpp"

TCS3200Sensor::TCS3200Sensor(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t out,
                             int numReadingsForAverage)
    : _s0_pin(s0), _s1_pin(s1), _s2_pin(s2), _s3_pin(s3), _out_pin(out),
      _numReadings(numReadingsForAverage) {
    // Valores de calibração padrão (podem ser ajustados via calibrate() ou set)
    // Branco (espera-se frequências baixas)
    _calib_white_R = 50;
    _calib_white_G = 50;
    _calib_white_B = 50;
    // Preto (espera-se frequências altas)
    _calib_black_R = 1000;
    _calib_black_G = 1000;
    _calib_black_B = 1000;
}

void TCS3200Sensor::begin(FrequencyScaling scaling) {
    pinMode(_s0_pin, OUTPUT);
    pinMode(_s1_pin, OUTPUT);
    pinMode(_s2_pin, OUTPUT);
    pinMode(_s3_pin, OUTPUT);
    pinMode(_out_pin, INPUT);

    setFrequencyScaling(scaling);
}

void TCS3200Sensor::setFrequencyScaling(FrequencyScaling scaling) {
    switch (scaling) {
        case FrequencyScaling::POWER_DOWN:
            digitalWrite(_s0_pin, LOW);
            digitalWrite(_s1_pin, LOW);
            break;
        case FrequencyScaling::SCALE_2_PERCENT:
            digitalWrite(_s0_pin, LOW);
            digitalWrite(_s1_pin, HIGH);
            break;
        case FrequencyScaling::SCALE_20_PERCENT:
            digitalWrite(_s0_pin, HIGH);
            digitalWrite(_s1_pin, LOW);
            break;
        case FrequencyScaling::SCALE_100_PERCENT:
            digitalWrite(_s0_pin, HIGH);
            digitalWrite(_s1_pin, HIGH);
            break;
    }
}

long TCS3200Sensor::readColorChannel(uint8_t s2_val, uint8_t s3_val) {
    digitalWrite(_s2_pin, s2_val);
    digitalWrite(_s3_pin, s3_val);
    delayMicroseconds(100); // Pequeno delay para estabilização do filtro
    // O timeout no pulseIn é importante para evitar travamentos se o sinal não vier.
    // Um valor de 100000 us (100 ms) é geralmente seguro.
    return pulseIn(_out_pin, LOW, 100000); 
}

void TCS3200Sensor::getRawAverageRGB(int& r, int& g, int& b) {
    long sum_r = 0, sum_g = 0, sum_b = 0;

    for (int i = 0; i < _numReadings; ++i) {
        sum_r += readColorChannel(LOW, LOW);   // Filtro Vermelho
        sum_g += readColorChannel(HIGH, HIGH); // Filtro Verde
        sum_b += readColorChannel(LOW, HIGH);  // Filtro Azul
        delay(10); // Delay entre conjuntos de leituras RGB
    }

    r = sum_r / _numReadings;
    g = sum_g / _numReadings;
    b = sum_b / _numReadings;
}

void TCS3200Sensor::calibrate() {
    if (!Serial) { // Garante que a Serial está ativa
        Serial.begin(115200); // Ou a taxa que você preferir
        while(!Serial); // Espera a Serial conectar (importante para placas como Leonardo/Micro)
    }

    Serial.println("\n--- Calibracao do Sensor TCS3200 ---");

    Serial.println("1. Posicione o sensor sobre uma SUPERFICIE BRANCA.");
    Serial.println("   Pressione 'B' e Enter para calibrar o branco...");
    while (true) {
        if (Serial.available() > 0) {
            char input = Serial.read();
            Serial.readStringUntil('\n'); // Limpa o buffer
            if (input == 'B' || input == 'b') {
                getRawAverageRGB(_calib_white_R, _calib_white_G, _calib_white_B);
                Serial.print("Branco calibrado: R="); Serial.print(_calib_white_R);
                Serial.print(", G="); Serial.print(_calib_white_G);
                Serial.print(", B="); Serial.println(_calib_white_B);
                break;
            }
        }
        delay(10);
    }

    delay(1000); // Tempo para o usuário

    Serial.println("\n2. Posicione o sensor sobre uma SUPERFICIE PRETA (ou escura).");
    Serial.println("   Pressione 'P' e Enter para calibrar o preto...");
    while (true) {
        if (Serial.available() > 0) {
            char input = Serial.read();
            Serial.readStringUntil('\n'); // Limpa o buffer
            if (input == 'P' || input == 'p') {
                getRawAverageRGB(_calib_black_R, _calib_black_G, _calib_black_B);
                Serial.print("Preto calibrado: R="); Serial.print(_calib_black_R);
                Serial.print(", G="); Serial.print(_calib_black_G);
                Serial.print(", B="); Serial.println(_calib_black_B);
                break;
            }
        }
        delay(10);
    }
    Serial.println("--- Calibracao Concluida ---");
}

void TCS3200Sensor::setWhiteBalance(int r_white, int g_white, int b_white) {
    _calib_white_R = r_white;
    _calib_white_G = g_white;
    _calib_white_B = b_white;
}

void TCS3200Sensor::setBlackBalance(int r_black, int g_black, int b_black) {
    _calib_black_R = r_black;
    _calib_black_G = g_black;
    _calib_black_B = b_black;
}

void TCS3200Sensor::getCalibratedRGB(int& r, int& g, int& b) {
    int raw_r, raw_g, raw_b;
    getRawAverageRGB(raw_r, raw_g, raw_b);

    // Mapeia os valores. Lembrar que MENOR frequência = MAIS luz.
    // Por isso, _calib_white_X (baixa frequência) mapeia para 255 (alto valor RGB).
    // E _calib_black_X (alta frequência) mapeia para 0 (baixo valor RGB).
    r = map(raw_r, _calib_black_R, _calib_white_R, 0, 255);
    g = map(raw_g, _calib_black_G, _calib_white_G, 0, 255);
    b = map(raw_b, _calib_black_B, _calib_white_B, 0, 255);

    // Garante que os valores fiquem no intervalo 0-255
    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);
}