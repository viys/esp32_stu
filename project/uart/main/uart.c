#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
// #include "freertos/queue.h"
// #include "freertos/semphr.h"
#include "freertos/task.h"
#include "portmacro.h"
#include "driver/uart.h"
#include "driver/gpio.h"

// #define UART_ASY

#ifdef UART_ASY
static QueueHandle_t uart_queue;
#endif

uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
};

static uint8_t rxBuffer[1024];


/**
 * @brief 任务 A
 * 
 * @param param 
 * 
 * @note 串口阻塞接收转发
 */
void task_A(void* param) {
    int recvLen = 0;
    while (1) {
        recvLen = uart_read_bytes(UART_NUM_1, rxBuffer, 1024, pdMS_TO_TICKS(50));
        if (recvLen) {
            uart_write_bytes(UART_NUM_1, rxBuffer, recvLen);
        }
    }
}

#ifdef UART_ASY

/**
 * @brief 任务 B
 * 
 * @param param 
 * 
 * @note 串口异步接收转发
 */
void task_B(void* param) {
    uart_event_t uart_ev;

    while (1) {
        if (pdTRUE == xQueueReceive(uart_queue, &uart_ev, portMAX_DELAY)) {
            switch (uart_ev.type) {
                case UART_DATA:
                    ESP_LOGI("uart1", "Uart1 recv len: %i", uart_ev.size);
                    uart_read_bytes(UART_NUM_1, rxBuffer, uart_ev.size,
                                    pdMS_TO_TICKS(1000));
                    uart_write_bytes(UART_NUM_1, rxBuffer, uart_ev.size);
                    break;
                case UART_BUFFER_FULL:
                    /* code */
                    break;
                case UART_FIFO_OVF:
                    /* code */
                    break;
                default:
                    break;
            }
        }
    }
}
#endif

void app_main(void)
{
    ESP_LOGW("main", "hello\r\n");

    uart_param_config(UART_NUM_1, &uart_config);

    uart_set_pin(UART_NUM_1, GPIO_NUM_4, GPIO_NUM_5, -1, -1);

#ifdef UART_ASY
    uart_driver_install(UART_NUM_1, 1024, 1024, 20, &uart_queue, 0);
    xTaskCreatePinnedToCore(task_B, "TaskB", 2048, NULL, 4, NULL, 0);
#else
    uart_driver_install(UART_NUM_1, 1024, 1024, 0, NULL, 0);
    xTaskCreatePinnedToCore(task_A, "TaskA", 2048, NULL, 4, NULL, 0);
#endif
}
