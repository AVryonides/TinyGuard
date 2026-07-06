#include <stdio.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_idf_version.h"

static const char *TAG = "TinyGuard";

static const char *reset_reason_to_string(esp_reset_reason_t reason)
{
    switch (reason) {
        case ESP_RST_UNKNOWN:
            return "Unknown";
        case ESP_RST_POWERON:
            return "Power-on reset";
        case ESP_RST_EXT:
            return "External reset";
        case ESP_RST_SW:
            return "Software reset";
        case ESP_RST_PANIC:
            return "Exception/panic reset";
        case ESP_RST_INT_WDT:
            return "Interrupt watchdog reset";
        case ESP_RST_TASK_WDT:
            return "Task watchdog reset";
        case ESP_RST_WDT:
            return "Other watchdog reset";
        case ESP_RST_DEEPSLEEP:
            return "Deep sleep reset";
        case ESP_RST_BROWNOUT:
            return "Brownout reset";
        case ESP_RST_SDIO:
            return "SDIO reset";
        case ESP_RST_USB:
            return "USB peripheral reset";
        case ESP_RST_JTAG:
            return "JTAG reset";
        case ESP_RST_EFUSE:
            return "eFuse reset";
        case ESP_RST_PWR_GLITCH:
            return "Power glitch reset";
        case ESP_RST_CPU_LOCKUP:
            return "CPU lockup reset";
        default:
            return "Unrecognized reset reason";
    }
}

static void print_chip_info(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf("\n");
    printf("============================================================\n");
    printf(" TinyGuard Hardware Probe\n");
    printf("============================================================\n");

    printf("ESP-IDF version: %s\n", esp_get_idf_version());

    printf("Chip model: ");
    switch (chip_info.model) {
        case CHIP_ESP32:
            printf("ESP32\n");
            break;
        case CHIP_ESP32S2:
            printf("ESP32-S2\n");
            break;
        case CHIP_ESP32S3:
            printf("ESP32-S3\n");
            break;
        case CHIP_ESP32C3:
            printf("ESP32-C3\n");
            break;
        case CHIP_ESP32C2:
            printf("ESP32-C2\n");
            break;
        case CHIP_ESP32C6:
            printf("ESP32-C6\n");
            break;
        case CHIP_ESP32H2:
            printf("ESP32-H2\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }

    printf("Chip revision: v%d.%d\n",
           chip_info.revision / 100,
           chip_info.revision % 100);

    printf("CPU cores: %d\n", chip_info.cores);

    printf("Features:");
    if (chip_info.features & CHIP_FEATURE_WIFI_BGN) {
        printf(" Wi-Fi");
    }
    if (chip_info.features & CHIP_FEATURE_BT) {
        printf(" Bluetooth");
    }
    if (chip_info.features & CHIP_FEATURE_BLE) {
        printf(" BLE");
    }
    if (chip_info.features & CHIP_FEATURE_EMB_FLASH) {
        printf(" Embedded-Flash");
    }
    if (chip_info.features & CHIP_FEATURE_EMB_PSRAM) {
        printf(" Embedded-PSRAM");
    }
    printf("\n");
}

static void print_memory_info(void)
{
    printf("\n");
    printf("Memory information\n");
    printf("------------------\n");

    printf("Free heap: %" PRIu32 " bytes\n",
           esp_get_free_heap_size());

    printf("Minimum free heap since boot: %" PRIu32 " bytes\n",
           esp_get_minimum_free_heap_size());

    printf("Internal 8-bit capable free heap: %zu bytes\n",
           heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));

    printf("Internal DMA-capable free heap: %zu bytes\n",
           heap_caps_get_free_size(MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));

    printf("Largest free internal block: %zu bytes\n",
           heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));

    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t largest_psram_block = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);

        if (free_psram > 0) {
            printf("PSRAM: detected and available\n");
            printf("Free PSRAM: %zu bytes\n", free_psram);
            printf("Largest free PSRAM block: %zu bytes\n", largest_psram_block);
        } else {
            printf("PSRAM: not detected or not initialized\n");
        }
}

static void print_flash_info(void)
{
    uint32_t flash_size = 0;
    esp_err_t ret = esp_flash_get_size(NULL, &flash_size);

    printf("\n");
    printf("Flash information\n");
    printf("-----------------\n");

    if (ret == ESP_OK) {
        printf("Flash size: %" PRIu32 " bytes (%.2f MB)\n",
               flash_size,
               flash_size / (1024.0 * 1024.0));
    } else {
        printf("Flash size: failed to read, error code: %s\n",
               esp_err_to_name(ret));
    }
}

static void print_runtime_info(void)
{
    printf("\n");
    printf("Runtime information\n");
    printf("-------------------\n");

    printf("Reset reason: %s\n",
           reset_reason_to_string(esp_reset_reason()));

    printf("Time since boot: %" PRId64 " us\n",
           esp_timer_get_time());
}

void app_main(void)
{
    print_chip_info();
    print_memory_info();
    print_flash_info();
    print_runtime_info();

    printf("\n");
    printf("Hardware probe complete.\n");
    printf("Next step: save this output into docs/hardware_notes.md\n");
    printf("============================================================\n");

    while (1) {
        ESP_LOGI(TAG, "Heartbeat: hardware probe running");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}