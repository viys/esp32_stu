#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/ledc.h"

#define LED_GPIO GPIO_NUM_48

#define FULL_EVENT_BIT  BIT0
#define EMPTY_EVENT_BIT BIT1

static EventGroupHandle_t ledc_event_handle;

bool IRAM_ATTR ledc_callback(const ledc_cb_param_t *param, void *user_arg) {
    BaseType_t taskWoken;

    if (param->duty) {
        xEventGroupSetBitsFromISR(ledc_event_handle, FULL_EVENT_BIT, &taskWoken);
    } else {
        xEventGroupSetBitsFromISR(ledc_event_handle, EMPTY_EVENT_BIT, &taskWoken);
    }

    return taskWoken;
}

void led_run_task(void* param) {
    EventBits_t ev;

    while (1)
    {
        ev = xEventGroupWaitBits(ledc_event_handle, FULL_EVENT_BIT | EMPTY_EVENT_BIT, pdTRUE, pdFALSE, pdMS_TO_TICKS(5000));
        if (ev & FULL_EVENT_BIT) {
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 2000);
            ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);        
        }

        if (ev & EMPTY_EVENT_BIT) {
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 8191, 2000);
            ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);  
        }

        ledc_cbs_t cbs = {
            .fade_cb = ledc_callback,
        };
    
        ledc_cb_register(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, &cbs, NULL);

    }
}

void app_main(void)
{
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    gpio_config(&led_cfg);

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
        .freq_hz = 5000,
        .duty_resolution = LEDC_TIMER_13_BIT,
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .gpio_num = LED_GPIO,
        .duty = 0,
        .intr_type = LEDC_INTR_DISABLE,
    };

    ledc_channel_config(&ledc_channel);

    ledc_fade_func_install(0);

    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 8191, 2000);

    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);

    ledc_event_handle = xEventGroupCreate();

    ledc_cbs_t cbs = {
        .fade_cb = ledc_callback,
    };

    ledc_cb_register(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, &cbs, NULL);
    
    xTaskCreatePinnedToCore(led_run_task, "led", 2048, NULL, 3, NULL, 1);
}
