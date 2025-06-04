#ifndef TCS3200_SENSOR_HPP
#define TCS3200_SENSOR_HPP

#include <Arduino.h>

class TCS3200Sensor {
public:
    // Enum para facilitar a configuração da escala de frequência
    enum class FrequencyScaling {
        POWER_DOWN = 0, // S0=L, S1=L
        SCALE_2_PERCENT = 1,  // S0=L, S1=H
        SCALE_20_PERCENT = 2, // S0=H, S1=L
        SCALE_100_PERCENT = 3 // S0=H, S1=H
    };

    /**
     * @brief Construtor da classe TCS3200Sensor.
     * @param s0 Pino S0 do sensor.
     * @param s1 Pino S1 do sensor.
     * @param s2 Pino S2 do sensor.
     * @param s3 Pino S3 do sensor.
     * @param out Pino OUT do sensor.
     * @param numReadingsForAverage Número de leituras para calcular a média e suavizar o sinal.
     */
    TCS3200Sensor(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t out,
                    int numReadingsForAverage = 10);

    /**
     * @brief Inicializa os pinos do sensor e configura a escala de frequência inicial.
     * @param scaling Escala de frequência desejada (padrão: 20%).
     */
    void begin(FrequencyScaling scaling = FrequencyScaling::SCALE_20_PERCENT);

    /**
     * @brief Define a escala de frequência de saída do sensor.
     * @param scaling A escala de frequência desejada.
     */
    void setFrequencyScaling(FrequencyScaling scaling);

    /**
     * @brief Realiza a rotina de calibração interativa para as cores branca e preta.
     * Os valores de calibração são armazenados internamente.
     */
    void calibrate();

    /**
     * @brief Obtém os valores de cor RGB brutos (frequência) após aplicar a média de leituras.
     * @param r Referência para armazenar o valor vermelho bruto.
     * @param g Referência para armazenar o valor verde bruto.
     * @param b Referência para armazenar o valor azul bruto.
     */
    void getRawAverageRGB(int& r, int& g, int& b);

    /**
     * @brief Obtém os valores de cor RGB finais (0-255) após leitura suave, calibração e mapeamento.
     * @param r Referência para armazenar o valor R final (0-255).
     * @param g Referência para armazenar o valor G final (0-255).
     * @param b Referência para armazenar o valor B final (0-255).
     */
    void getCalibratedRGB(int& r, int& g, int& b);

    /**
     * @brief Define manualmente os valores de calibração para o branco.
     * @param r_white Valor vermelho para branco.
     * @param g_white Valor verde para branco.
     * @param b_white Valor azul para branco.
     */
    void setWhiteBalance(int r_white, int g_white, int b_white);

    /**
     * @brief Define manualmente os valores de calibração para o preto.
     * @param r_black Valor vermelho para preto.
     * @param g_black Valor verde para preto.
     * @param b_black Valor azul para preto.
     */
    void setBlackBalance(int r_black, int g_black, int b_black);


private:
    uint8_t _s0_pin, _s1_pin, _s2_pin, _s3_pin, _out_pin;
    int _numReadings;

    // Valores de calibração (frequências brutas)
    // Frequências MENORES para cores MAIS brilhantes (branco)
    // Frequências MAIORES para cores MAIS escuras (preto)
    int _calib_white_R, _calib_white_G, _calib_white_B;
    int _calib_black_R, _calib_black_G, _calib_black_B;

    /**
     * @brief Lê o valor de frequência de um canal de cor específico.
     * @param s2_val Estado do pino S2 para selecionar o filtro.
     * @param s3_val Estado do pino S3 para selecionar o filtro.
     * @return O valor de frequência lido (período do pulso).
     */
    long readColorChannel(uint8_t s2_val, uint8_t s3_val);
};

#endif // TCS3200_SENSOR_HPP