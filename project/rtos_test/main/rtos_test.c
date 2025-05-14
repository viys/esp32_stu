#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "portmacro.h"

#define NUM0_BIT BIT0
#define NUM1_BIT BIT1

EventGroupHandle_t test_event;
TaskHandle_t taskH_handle;
TaskHandle_t taskI_handle;

QueueHandle_t queue_handle = NULL;
SemaphoreHandle_t sem_handle = NULL;
SemaphoreHandle_t mutex_handle = NULL;

typedef struct {
    int value;
} queue_data_t;

void task_A(void* param) {
    while (1) {
        ESP_LOGI("main", "hello task A");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void task_B(void* param) {
    queue_data_t data;

    while (1) {
        if (pdPASS == xQueueReceive(queue_handle, &data, portMAX_DELAY)) {
            ESP_LOGI("mian", "massage: %d", data.value);
        }
    }
}

void task_C(void* param) {
    queue_data_t data = {.value = 0};

    while (1) {
        xQueueSend(queue_handle, &data, 100);
        vTaskDelay(pdMS_TO_TICKS(1000));
        data.value++;
    }
}

void task_D(void* param) {

    while (1) {
        xSemaphoreGive(sem_handle);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void task_E(void* param) {

    while (1) {
        if (pdTRUE == xSemaphoreTake(sem_handle, portMAX_DELAY)) {
            ESP_LOGI("main", "Task E take sem suc");
        }
    }
}

void task_F(void* param) {

    while (1) {
        xSemaphoreTake(mutex_handle, portMAX_DELAY);
        // opt

        xSemaphoreGive(mutex_handle);
    }
}

void task_G(void* param) {

    while (1) {
        xSemaphoreTake(mutex_handle, portMAX_DELAY);
        // opt

        xSemaphoreGive(mutex_handle);
    }
}

// void task_H(void* param) {

//     while (1) {
//         xEventGroupSetBits(test_event, NUM0_BIT);
//         vTaskDelay(pdMS_TO_TICKS(1000));
//         xEventGroupSetBits(test_event, NUM1_BIT);
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

// void task_I(void* param) {
//     EventBits_t ev;

//     while (1) {
//         ev = xEventGroupWaitBits(test_event, NUM0_BIT | NUM1_BIT, pdTRUE,
//                                  pdFALSE, pdMS_TO_TICKS(5000));
//         if (ev & NUM0_BIT) {
//             ESP_LOGI("ev", "Get BIT0 event");
//         }
//         if (ev & NUM1_BIT) {
//             ESP_LOGI("ev", "Get BIT1 event");
//         }
//     }
// }

void task_H(void* param) {
    uint32_t value = 0;

    vTaskDelay(pdMS_TO_TICKS(200));

    while (1) {
        xTaskNotify(taskI_handle, value, eSetValueWithoutOverwrite);
        vTaskDelay(pdMS_TO_TICKS(1000));
        value++;
    }
}

void task_I(void* param) {
    uint32_t value = 0;

    while (1) {
        xTaskNotifyWait(0x00, ULONG_MAX, &value, portMAX_DELAY);
        ESP_LOGI("ev", "Notify wait avlue:%lu", value);
    }
}

void app_main(void) {

    queue_handle = xQueueCreate(10, sizeof(queue_data_t));
    sem_handle = xSemaphoreCreateBinary();
    mutex_handle = xSemaphoreCreateMutex();
    test_event = xEventGroupCreate();

    xTaskCreatePinnedToCore(task_A, "Task A", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(task_B, "Task B", 2048, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(task_C, "Task C", 2048, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(task_D, "Task D", 2048, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(task_E, "Task E", 2048, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(task_H, "Task H", 2048, NULL, 4, &taskH_handle, 1);
    xTaskCreatePinnedToCore(task_I, "Task H", 2048, NULL, 4, &taskI_handle, 1);
}
