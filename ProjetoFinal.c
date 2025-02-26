#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

#define JOY_Y_PIN 27
#define JOY_X_PIN 26
#define SW_PIN 22
#define BUZZER_PIN 21

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// New functions for buzzer audio
void play_tone(unsigned int frequency, unsigned int duration_ms) {
    unsigned int period_us = 1000000 / frequency;
    unsigned int half_period = period_us / 2;
    unsigned int cycles = (duration_ms * 1000) / period_us;
    for (unsigned int i = 0; i < cycles; i++) {
        gpio_put(BUZZER_PIN, 1);
        sleep_us(half_period);
        gpio_put(BUZZER_PIN, 0);
        sleep_us(half_period);
    }
}

void playBlindingLights() {
    // Notas corrigidas baseadas na imagem
    int melody[][2] = {
        {440, 300}, // A4
        {440, 300}, // A4
        {392, 100}, // G4
        {440, 200}, // A4
        {494, 400}, // B4
        {330, 200}, // E4
        {392, 200}, // G4
        {330, 200}, // E4
        {440, 200}, // A4
        {440, 200}, // A4
        {392, 200}, // G4
        {440, 200}, // A4
        {494, 200}, // B4
        {330, 200}, // E4
        {392, 200}, // G4
    };
    
    int length = sizeof(melody) / sizeof(melody[0]);
    for (int i = 0; i < length; i++) {
        play_tone(melody[i][0], melody[i][1]);
        sleep_ms(50); // Pequena pausa entre as notas
    }
}



int main()
{
    stdio_init_all();
    adc_init();

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();

    adc_gpio_init(JOY_X_PIN);
    adc_gpio_init(JOY_Y_PIN);

    // Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Initialize buzzer pin
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    // Play "Blinding Lights" melody
    playBlindingLights();

restart:

    while (true)
    {
        adc_select_input(0);
        int x = adc_read();
        adc_select_input(1);
        int y = adc_read();

        printf("x: %d, y: %d\n", x, y);
        if (x > 2048) {
            ssd1306_draw_string(ssd, 0, 0, "Projeto Final");
            render_on_display(ssd, &frame_area);
            
            playBlindingLights();
            
        }
        sleep_ms(10);

    }

    return 0;
}
