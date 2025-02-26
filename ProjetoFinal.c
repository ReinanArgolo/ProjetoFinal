#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "pico/multicore.h"
#include <stdbool.h>


// Imprta arquivos de função
#include "functions/convert.h"

// Define as portas dos perifericos
#define BNT1_PIN 5
#define JOY_Y_PIN 27
#define JOY_X_PIN 26
#define SW_PIN 22
#define BUZZER_PIN 21
#define LED_BLUE_PIN 12
#define LED_RED_PIN 13
#define LED_GREEN_PIN 11

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
    // Notas corrigidas com tempos ajustados
    int melody[][2] = {
        // {440, 300}, // A4
        // {440, 300}, // A4
        // {392, 150}, // G4
        // {440, 300}, // A4
        // {494, 400}, // B4
        // {330, 200}, // E4
        // {392, 250}, // G4
        // {330, 250}, // E4
        // {440, 300}, // A4
        // {440, 300}, // A4
        // {392, 150}, // G4
        // {440, 300}, // A4
        // {494, 400}, // B4
        // {330, 200}, // E4
        // {392, 250}, // G4

        // // Continuação conforme imagem (DBAG DBAGA) com tempos ajustados
        {293, 300}, // D4
        {247, 200}, // B3
        {220, 300}, // A3
        {196, 300}, // G3
        
        {293, 300}, // D4
        {247, 200}, // B3
        {220, 300}, // A3
        {196, 250}, // G3
        {220, 350}, // A3 (última nota um pouco mais longa)
    };
    
    int length = sizeof(melody) / sizeof(melody[0]);
    for (int i = 0; i < length; i++) {
        play_tone(melody[i][0], melody[i][1]);
        sleep_ms(50); // Pequena pausa entre as notas
    }
}

void limparDisplay(uint8_t *ssd, struct render_area *area) {
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, area);
}

void alerta(int umidade, int temperatura, uint8_t *ssd, struct render_area *frame_area) {

    char mensagem[35];
    memset(ssd, 0, ssd1306_buffer_length);
    sprintf(mensagem, "PERIGO ATIVANDO CONTROLES");
    ssd1306_draw_string(ssd, 0, 34, mensagem);
    playBlindingLights();
    render_on_display(ssd, frame_area);
    gpio_put(LED_RED_PIN, 1);
    sleep_ms(500);

    gpio_put(LED_RED_PIN, 0);
    memset(ssd, 0, ssd1306_buffer_length);
    sprintf(mensagem, "UMIDADE: %d", umidade);
    ssd1306_draw_string(ssd, 0, 34, mensagem);
    render_on_display(ssd, frame_area);
    gpio_put(LED_RED_PIN, 1);
    playBlindingLights();

    sleep_ms(500);
    gpio_put(LED_RED_PIN, 0);
    memset(ssd, 0, ssd1306_buffer_length);
    sprintf(mensagem, "TEMPERATURA: %d", temperatura);
    ssd1306_draw_string(ssd, 0, 34, mensagem);
    render_on_display(ssd, frame_area);
    gpio_put(LED_RED_PIN, 1);
    playBlindingLights();

    gpio_put(LED_RED_PIN, 0);
}

int main()
{
    stdio_init_all();
    adc_init();

    gpio_init(LED_BLUE_PIN);
    gpio_init(LED_RED_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);

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

    // Show On screen 
    ssd1306_draw_string(ssd, 10, 32, "Projeto Final");
    render_on_display(ssd, &frame_area);

    sleep_ms(200);

    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Flag Screen
    int dataOnScreen = 0;

    char mensagem[35];

    // Changed from converterDadosADC to converterJoyToUmid
    int umidade = 99;
    int temperatura = 32;

restart:

    while (true)
    {
        adc_select_input(0);
        int x = adc_read();
        adc_select_input(1);
        int y = adc_read();

        converterJoyToCelsius(x, &temperatura);
        converterJoyToUmid(y, &umidade);

        gpio_init(BNT1_PIN);
        gpio_set_dir(BNT1_PIN, GPIO_IN);
        gpio_pull_up(BNT1_PIN);

        sprintf(mensagem, "Umidade: %d", umidade);
        ssd1306_draw_string(ssd, 0, 0, mensagem);
        sprintf(mensagem, "Temperatura: %d", temperatura);
        ssd1306_draw_string(ssd, 0, 16, mensagem);
        render_on_display(ssd, &frame_area);

        static uint32_t last_press_time = 0;
        uint32_t current_time = to_ms_since_boot(get_absolute_time());

        if (gpio_get(BNT1_PIN) == 0 && (current_time - last_press_time) > 200)
        {
            last_press_time = current_time;
            dataOnScreen++;
            limparDisplay(ssd, &frame_area);
            if (dataOnScreen > 2)
            {
            dataOnScreen = 0;
            }

                switch (dataOnScreen)
            {
            case 0:
                sprintf(mensagem, "Umidade: %2d", umidade);
                ssd1306_draw_string(ssd, 0, 0, mensagem);  
                render_on_display(ssd, &frame_area);

                break;
            case 1:
                sprintf(mensagem, "Temperatura: %d", temperatura);
                ssd1306_draw_string(ssd, 0, 0, mensagem);
                render_on_display(ssd, &frame_area);

                break;
            case 2:
                sprintf(mensagem, "Umidade: %d", umidade);
                ssd1306_draw_string(ssd, 0, 0, mensagem);
                sprintf(mensagem, "Temperatura: %d", temperatura);
                ssd1306_draw_string(ssd, 0, 16, mensagem);
                render_on_display(ssd, &frame_area);

            default:
                break;
            }
        }
        
        if(temperatura > 45 ||temperatura < 20 || umidade < 75){
           
            alerta(umidade, temperatura, ssd, &frame_area);
            goto restart;


        }
        
        sleep_ms(10);

    }

    return 0;
}
