#include <stdio.h>
#include "esp_partition.h"
#include "esp_log.h"
#include <string.h>

static const char* TAG = "partition";

#define USER_PARTITION_TYPE     0x40
#define USER_PARTITION_SUBTYPE  0x01

static const esp_partition_t* partition_ptr = NULL;

void app_main(void)
{
    partition_ptr = esp_partition_find_first(USER_PARTITION_TYPE,
                                             USER_PARTITION_SUBTYPE, NULL);

    if (partition_ptr == NULL) {
        ESP_LOGE(TAG, "Can`t find partirion");
        return;
    }

    esp_partition_erase_range(partition_ptr, 0, 0x1000);

    const char* test_string = "this is for test partition";

    esp_partition_write(partition_ptr, 0, test_string, strlen(test_string));

    char read_buf[64] = {};

    memset(read_buf, 0, sizeof(read_buf));

    esp_partition_read(partition_ptr, 0, read_buf, strlen(test_string));

    ESP_LOGI(TAG, "read partition data: %s", read_buf);

    return;
}
