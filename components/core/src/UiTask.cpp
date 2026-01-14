#include "UiTask.hpp"

#include "UiService.hpp"
#include "UiTypes.hpp"

// IDF
#include <esp_log.h>

namespace core {
static constexpr uint32_t QUEUE_LENGTH = 5;
static constexpr uint32_t QUEUE_ITEM_SIZE = sizeof(common::UiEvent);
static constexpr uint32_t TASK_STACK_SIZE = 4096;
static constexpr uint32_t TASK_PRIORITY = 5;

static const char *TAG = "UiTask";

UiTask::UiTask(services::UiService &ui) : mUiQueue(nullptr), mUiService(ui) {
    ESP_LOGI(TAG, "UiTask::UiTask created");
}

bool UiTask::init() {
    // Create the queue
    // xQueueCreate(length, item_size)
    mUiQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

    if (mUiQueue == nullptr) {
        ESP_LOGE(TAG, "Failed to create UI queue");
        return false;
    }

    // Create the FreeRTOS task
    // xTaskCreate(function, name, stack_size, params, priority, handle)
    BaseType_t result = xTaskCreate(UiTask::taskEntry,  // Task function
                                    "UiTask",           // Task name
                                    TASK_STACK_SIZE,    // Stack size in bytes
                                    this,  // Parameter passed to task (our UiTask instance)
                                    TASK_PRIORITY,  // Priority
                                    nullptr         // Task handle (optional)
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UI task");
        return false;
    }

    ESP_LOGI(TAG, "UI task initialized");
    return true;
}

// Static wrapper that FreeRTOS calls
void UiTask::taskEntry(void *pvParameters) {
    auto *pThis = static_cast<UiTask *>(pvParameters);
    pThis->runLoop();

    // If runLoop exits, delete the task
    vTaskDelete(nullptr);
}

void UiTask::post(const common::UiEvent &e) {
    if (mUiQueue == nullptr) {
        ESP_LOGE(TAG, "Queue not initialized");
        return;
    }

    // xQueueSend(queue, ptr_to_item, ticks_to_wait)
    // pdMS_TO_TICKS converts milliseconds to FreeRTOS ticks
    // portMAX_DELAY means wait forever if queue is full
    // Timeout: 100ms
    BaseType_t result = xQueueSend(mUiQueue, &e, pdMS_TO_TICKS(100));

    if (result != pdPASS) {
        ESP_LOGW(TAG, "Failed to post UI event (queue full?)");
        // TODO: handle this properly
    }
}

void UiTask::runLoop() {
    ESP_LOGI(TAG, "UI task loop started");

    common::UiEvent event;

    while (true) {
        // Wait for event from queue
        // xQueueReceive blocks until item arrives or timeout
        BaseType_t result = xQueueReceive(mUiQueue, &event, pdMS_TO_TICKS(1000));

        if (result == pdTRUE) {
            ESP_LOGI(TAG, "Received UI event, type=%d", (int)event.type);

            mUiService.onEvent(event);
        }
    }
}

}  // namespace core
