idf_component_register(SRCS "cpu_memory_benchmark_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES freertos heap log esp_psram esp_timer esp_system)
