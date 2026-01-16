#include "AppContext.hpp"

// IDF
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "app_main";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "=== ESP Radio App Starting ===");

    core::AppContext appContext;

    if (!appContext.init()) {
        ESP_LOGE(TAG, "Failed to initialize AppContext");

        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_restart();
        return;
    }

    ESP_LOGI(TAG, "Application initialized successfully");

    // TODO: Create a proper health check mechanism + shutdown handling
    int loopCount = 0;
    while (true) {
        loopCount++;

        if (loopCount % 10 == 0) {
            ESP_LOGI(TAG, "App alive (loop %d)", loopCount);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
