#include <stdio.h>
#include "esp_err.h"
#include "nvs_flash.h"
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define NAME_SPACE_WIFI1    "wifi1"
#define NAME_SPACE_WIFI2    "wifi2"

#define NVS_SSID_KEY        "ssid"
#define NVS_PASSWORD_KEY    "password"

void nvs_blob_read(const char* namespace, const char* key, void* buffer, int maxlen) {
    nvs_handle_t nvs_handle;
    size_t length = 0;

    nvs_open(namespace, NVS_READONLY, &nvs_handle);
    nvs_get_blob(nvs_handle, key, NULL, &length);
    if (length && length <= maxlen) {
        nvs_get_blob(nvs_handle, key, buffer, &length);
    }
    nvs_close(nvs_handle);
}

void app_main(void) {
    nvs_handle_t nvs_handle1;
    nvs_handle_t nvs_handle2;

    esp_err_t ret = nvs_flash_init();

    if (ret != ESP_OK) {
        nvs_flash_erase();
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    nvs_open(NAME_SPACE_WIFI1, NVS_READWRITE, &nvs_handle1);
    
    nvs_set_blob(nvs_handle1, NVS_SSID_KEY, "wifi_esp32", strlen("wifi_esp32"));
    nvs_set_blob(nvs_handle1, NVS_PASSWORD_KEY, "123456", strlen("123456"));
    nvs_commit(nvs_handle1);
    nvs_close(nvs_handle1);
    
    nvs_open(NAME_SPACE_WIFI2, NVS_READWRITE, &nvs_handle2);
    nvs_set_blob(nvs_handle2, NVS_SSID_KEY, "wifi_hello", strlen("wifi_hello"));
    nvs_set_blob(nvs_handle2, NVS_PASSWORD_KEY, "123456", strlen("123456"));
    nvs_commit(nvs_handle2);
    nvs_close(nvs_handle2);

    vTaskDelay(pdMS_TO_TICKS(1000));

    char buff[64];
    
    memset(buff, 0, sizeof(buff));
    nvs_blob_read(NAME_SPACE_WIFI1, NVS_SSID_KEY, buff, sizeof(buff));
    ESP_LOGI("nvs", "%s.%s:%s", NAME_SPACE_WIFI1, NVS_SSID_KEY, buff);
    
    memset(buff, 0, sizeof(buff));
    nvs_blob_read(NAME_SPACE_WIFI1, NVS_PASSWORD_KEY, buff, sizeof(buff));
    ESP_LOGI("nvs", "%s.%s:%s", NAME_SPACE_WIFI1, NVS_PASSWORD_KEY, buff);

    memset(buff, 0, sizeof(buff));
    nvs_blob_read(NAME_SPACE_WIFI2, NVS_SSID_KEY, buff, sizeof(buff));
    ESP_LOGI("nvs", "%s.%s:%s", NAME_SPACE_WIFI2, NVS_SSID_KEY, buff);

    memset(buff, 0, sizeof(buff));
    nvs_blob_read(NAME_SPACE_WIFI2, NVS_PASSWORD_KEY, buff, sizeof(buff));
    ESP_LOGI("nvs", "%s.%s:%s", NAME_SPACE_WIFI2, NVS_PASSWORD_KEY, buff);
}
