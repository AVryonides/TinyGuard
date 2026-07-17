#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_psram.h"
#include "esp_timer.h"
#include "esp_system.h"

static const char *TAG = "TinyGuardBench";

#define INT_BENCH_ITERATIONS      10000000
#define FLOAT_BENCH_ITERATIONS    5000000
#define MEMORY_BUFFER_SIZE_BYTES  (512 * 1024)
#define MEMORY_PASSES             50

static double elapsed_seconds(int64_t start_us, int64_t end_us)
{
    return (double)(end_us - start_us) / 1000000.0;
}

static void run_integer_benchmark(void)
{
    volatile int32_t a = 123;
    volatile int32_t b = 456;
    volatile int32_t c = 0;

    int64_t start = esp_timer_get_time();

    for (int i = 0; i < INT_BENCH_ITERATIONS; i++) {
        c += a;
        c ^= b;
        c += i;
        c *= 3;
        c -= b;
    }

    int64_t end = esp_timer_get_time();

    double seconds = elapsed_seconds(start, end);
    double operations = (double)INT_BENCH_ITERATIONS * 5.0;
    double mops = operations / seconds / 1000000.0;

    printf("\nInteger benchmark\n");
    printf("-----------------\n");
    printf("Iterations: %d\n", INT_BENCH_ITERATIONS);
    printf("Approx operations: %.0f\n", operations);
    printf("Elapsed time: %.6f seconds\n", seconds);
    printf("Approx throughput: %.2f MOPS\n", mops);
    printf("Final value guard: %" PRId32 "\n", c);
}

static void run_float_benchmark(void)
{
    volatile float a = 1.0001f;
    volatile float b = 0.9999f;
    volatile float c = 1.0f;

    int64_t start = esp_timer_get_time();

    for (int i = 0; i < FLOAT_BENCH_ITERATIONS; i++) {
        c += a;
        c *= b;
        c -= 0.0001f;
        c /= a;
    }

    int64_t end = esp_timer_get_time();

    double seconds = elapsed_seconds(start, end);
    double operations = (double)FLOAT_BENCH_ITERATIONS * 4.0;
    double mflops = operations / seconds / 1000000.0;

    printf("\nFloating-point benchmark\n");
    printf("------------------------\n");
    printf("Iterations: %d\n", FLOAT_BENCH_ITERATIONS);
    printf("Approx operations: %.0f\n", operations);
    printf("Elapsed time: %.6f seconds\n", seconds);
    printf("Approx throughput: %.2f MFLOPS\n", mflops);
    printf("Final value guard: %.6f\n", c);
}

static void run_memory_write_benchmark(const char *label, uint8_t *buffer, size_t size_bytes)
{
    int64_t start = esp_timer_get_time();

    for (int pass = 0; pass < MEMORY_PASSES; pass++) {
        memset(buffer, pass & 0xFF, size_bytes);
    }

    int64_t end = esp_timer_get_time();

    double seconds = elapsed_seconds(start, end);
    double total_bytes = (double)size_bytes * (double)MEMORY_PASSES;
    double mb_per_s = total_bytes / seconds / (1024.0 * 1024.0);

    printf("\n%s write benchmark\n", label);
    printf("----------------------\n");
    printf("Buffer size: %zu bytes\n", size_bytes);
    printf("Passes: %d\n", MEMORY_PASSES);
    printf("Total written: %.2f MB\n", total_bytes / (1024.0 * 1024.0));
    printf("Elapsed time: %.6f seconds\n", seconds);
    printf("Write bandwidth: %.2f MB/s\n", mb_per_s);
}

static void run_memory_read_benchmark(const char *label, uint8_t *buffer, size_t size_bytes)
{
    volatile uint32_t checksum = 0;

    for (size_t i = 0; i < size_bytes; i++) {
        buffer[i] = (uint8_t)(i & 0xFF);
    }

    int64_t start = esp_timer_get_time();

    for (int pass = 0; pass < MEMORY_PASSES; pass++) {
        for (size_t i = 0; i < size_bytes; i++) {
            checksum += buffer[i];
        }
    }

    int64_t end = esp_timer_get_time();

    double seconds = elapsed_seconds(start, end);
    double total_bytes = (double)size_bytes * (double)MEMORY_PASSES;
    double mb_per_s = total_bytes / seconds / (1024.0 * 1024.0);

    printf("\n%s read benchmark\n", label);
    printf("---------------------\n");
    printf("Buffer size: %zu bytes\n", size_bytes);
    printf("Passes: %d\n", MEMORY_PASSES);
    printf("Total read: %.2f MB\n", total_bytes / (1024.0 * 1024.0));
    printf("Elapsed time: %.6f seconds\n", seconds);
    printf("Read bandwidth: %.2f MB/s\n", mb_per_s);
    printf("Checksum guard: %" PRIu32 "\n", checksum);
}

static void run_memory_benchmarks(void)
{
    printf("\nMemory benchmark setup\n");
    printf("----------------------\n");

    printf("Internal free heap before allocation: %zu bytes\n",
           heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));

    printf("PSRAM free heap before allocation: %zu bytes\n",
           heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    uint8_t *internal_buffer = heap_caps_malloc(
        MEMORY_BUFFER_SIZE_BYTES,
        MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
    );

    if (internal_buffer == NULL) {
        printf("\nInternal RAM allocation failed for %d bytes.\n",
               MEMORY_BUFFER_SIZE_BYTES);
        printf("Trying smaller internal buffer: 128 KB.\n");

        internal_buffer = heap_caps_malloc(
            128 * 1024,
            MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
        );

        if (internal_buffer != NULL) {
            run_memory_write_benchmark("Internal RAM", internal_buffer, 128 * 1024);
            run_memory_read_benchmark("Internal RAM", internal_buffer, 128 * 1024);
            free(internal_buffer);
        } else {
            printf("Internal RAM allocation failed even at 128 KB.\n");
        }
    } else {
        run_memory_write_benchmark("Internal RAM", internal_buffer, MEMORY_BUFFER_SIZE_BYTES);
        run_memory_read_benchmark("Internal RAM", internal_buffer, MEMORY_BUFFER_SIZE_BYTES);
        free(internal_buffer);
    }

    if (esp_psram_is_initialized()) {
        uint8_t *psram_buffer = heap_caps_malloc(
            MEMORY_BUFFER_SIZE_BYTES,
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        );

        if (psram_buffer != NULL) {
            run_memory_write_benchmark("PSRAM", psram_buffer, MEMORY_BUFFER_SIZE_BYTES);
            run_memory_read_benchmark("PSRAM", psram_buffer, MEMORY_BUFFER_SIZE_BYTES);
            free(psram_buffer);
        } else {
            printf("PSRAM allocation failed for %d bytes.\n", MEMORY_BUFFER_SIZE_BYTES);
        }
    } else {
        printf("PSRAM not initialized. Skipping PSRAM memory benchmark.\n");
    }

    printf("\nMemory after benchmark\n");
    printf("----------------------\n");
    printf("Internal free heap after benchmark: %zu bytes\n",
           heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));

    printf("PSRAM free heap after benchmark: %zu bytes\n",
           heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void app_main(void)
{
    printf("\n");
    printf("============================================================\n");
    printf(" TinyGuard CPU and Memory Benchmark\n");
    printf("============================================================\n");

    printf("Free heap at start: %" PRIu32 " bytes\n", esp_get_free_heap_size());
    printf("Internal free heap: %zu bytes\n",
           heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    printf("PSRAM initialized: %s\n",
           esp_psram_is_initialized() ? "yes" : "no");
    printf("PSRAM free heap: %zu bytes\n",
           heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    run_integer_benchmark();
    run_float_benchmark();
    run_memory_benchmarks();

    printf("\n");
    printf("Benchmark complete.\n");
    printf("Save the output into experiments/hardware_profile/cpu_memory_benchmark_run_01.md\n");
    printf("============================================================\n");

    while (1) {
        ESP_LOGI(TAG, "Heartbeat: CPU/memory benchmark complete");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}