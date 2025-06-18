#include <stdlib.h>         // 标准库，包含内存管理函数如 malloc/free
#include <stdio.h>          // 标准输入输出，如 printf、fopen、fclose 等
#include <string.h>         // 字符串处理函数，如 strchr、strcmp 等
#include "esp_vfs.h"        // ESP-IDF 虚拟文件系统接口
#include "esp_vfs_fat.h"    // FAT 文件系统支持（包括 SPI flash 的挂载等）
#include "sdkconfig.h"      // 项目配置文件，定义 CONFIG_ 系列宏

static const char *TAG = "fatfs";  // 日志标签，用于 ESP_LOGx 系列函数中区分模块

// 指定 FAT 文件系统的挂载路径
const char *base_path = "/spiflash";

// SPI flash 的 wear leveling 句柄
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

void app_main(void)
{
    ESP_LOGI(TAG, "Mounting FAT filesystem"); // 打印挂载开始的日志

    // 定义挂载配置
    const esp_vfs_fat_mount_config_t mount_config = {
        .max_files = 4,                    // 最多同时打开的文件数
        .format_if_mount_failed = true,   // 挂载失败时是否尝试格式化
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE, // 分配单元大小（影响读写性能和空间效率）
        .use_one_fat = false,             // 是否只使用一个 FAT 表，true 可节省内存但降低容错
    };

    // 挂载 FATFS 文件系统（读写模式），使用 wear leveling，位于名称为 "storage" 的分区上
    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(base_path, "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err)); // 打印错误信息
        return;
    }

    ESP_LOGI(TAG, "Filesystem mounted"); // 成功挂载日志

    ESP_LOGI(TAG, "Opening file"); // 打开文件日志

    const char *filename = "/spiflash/example.txt"; // 要创建的文件路径

    // 以二进制写入模式打开文件（如果不存在将创建）
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        perror("fopen"); // 标准输出错误原因（仅限 stderr）
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    fprintf(f, "Hello World!\n"); // 写入一行数据
    fclose(f);                    // 关闭文件

    ESP_LOGI(TAG, "File written"); // 写入成功日志

    // 打开同一个文件用于读取
    ESP_LOGI(TAG, "Reading file");

    f = fopen(filename, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    char line[128];              // 定义读取缓冲区

    fgets(line, sizeof(line), f); // 读取一行内容
    fclose(f);                    // 关闭文件

    // 去除行末的换行符
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }

    ESP_LOGI(TAG, "Read from file: '%s'", line); // 打印读取内容

    // 卸载 FATFS 文件系统，释放资源
    ESP_LOGI(TAG, "Unmounting FAT filesystem");

    ESP_ERROR_CHECK(esp_vfs_fat_spiflash_unmount_rw_wl(base_path, s_wl_handle));

    ESP_LOGI(TAG, "Done"); // 程序结束日志
}
