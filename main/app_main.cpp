#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
  ESP_LOGI("APP", "Hello from ESP32-S3");
  while (true) {
    ESP_LOGI("APP", "tick");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
