#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define LEDC_LS_TIMER          LEDC_TIMER_0
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LED_GPIO_BLUE          (2)
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_FADE_TIME         (1800)
#define LEDC_TASK_STACK_DEPTH  (1024)
#define LEDC_MAX_DUTY          (8196)
#define LEDC_MIN_DUTY          (1366)
#define LEDC_PRIORITY          (2)

TaskHandle_t G_PWMLED_HANDLE = NULL;

bool G_RUNNING = false;
bool G_INIT_PWM = false;

ledc_channel_config_t G_LEDC_OBJ = {
    .channel    = LEDC_LS_CH0_CHANNEL,
    .duty       = 0,
    .gpio_num   = LED_GPIO_BLUE,
    .speed_mode = LEDC_LS_MODE,
    .hpoint     = 0,
    .timer_sel  = LEDC_LS_TIMER
};

void _dimPwmLED() {
    ledc_set_duty(G_LEDC_OBJ.speed_mode, G_LEDC_OBJ.channel, LEDC_MAX_DUTY);
    ledc_update_duty(G_LEDC_OBJ.speed_mode, G_LEDC_OBJ.channel);
}

void initPwmLED() {
    if (G_INIT_PWM) {
        return;
    }
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 1000,                      // frequency of PWM signal
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .timer_num = LEDC_LS_TIMER            // timer index
    };
    // Set configuration of timer0 for low speed channels
    ledc_timer_config(&ledc_timer);
    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&G_LEDC_OBJ);
    // Initialize fade service.
    ledc_fade_func_install(0);
    _dimPwmLED();
    G_INIT_PWM = true;
}

void _doStartPwmLED(void* pvParameters) {
    do {
        if (!G_RUNNING) {
          break;
        }
        printf("A: LEDC fade up to duty = %d\n", LEDC_MAX_DUTY);
        ledc_set_fade_with_time(G_LEDC_OBJ.speed_mode, G_LEDC_OBJ.channel, LEDC_MIN_DUTY, LEDC_FADE_TIME);
        ledc_fade_start(G_LEDC_OBJ.speed_mode, G_LEDC_OBJ.channel, LEDC_FADE_NO_WAIT);
        vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);

        printf("Z: LEDC fade down to duty = %d\n", LEDC_MIN_DUTY);
        ledc_set_fade_with_time(G_LEDC_OBJ.speed_mode, G_LEDC_OBJ.channel, LEDC_MAX_DUTY, LEDC_FADE_TIME);
        ledc_fade_start(G_LEDC_OBJ.speed_mode, G_LEDC_OBJ.channel, LEDC_FADE_NO_WAIT);
        vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    } while (true);
    vTaskDelete(NULL);
}

void stopPwmLED() {
    initPwmLED();
    G_RUNNING = false;
    if (G_PWMLED_HANDLE) {
        vTaskDelete(G_PWMLED_HANDLE);
        _dimPwmLED();
        G_PWMLED_HANDLE = NULL;
    }
}

void startPwmLED() {
    initPwmLED();
    stopPwmLED();
    /* Create the task, storing the handle. */
    BaseType_t taskRet = xTaskCreate(_doStartPwmLED, "_doStartPwmLED", LEDC_TASK_STACK_DEPTH, NULL, LEDC_PRIORITY, &G_PWMLED_HANDLE);
    if( taskRet == pdPASS ) {
        G_RUNNING = true;
    }
}

#ifdef __cplusplus
}
#endif
