#pragma once

#include <cstdint>

#include "IUiSink.hpp"

// IDF
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace services {
class UiService;
}  // namespace services

namespace common {
struct UiEvent;
}  // namespace common

namespace core {
class UiTask final : public IUiSink {
   public:
    explicit UiTask(services::UiService &ui);
    bool init();

    // IUiSink
    void post(const common::UiEvent &e) override;

    void runLoop();
    static void taskEntry(void *pvParameters);

   private:
    QueueHandle_t mUiQueue;
    services::UiService &mUiService;
};

}  // namespace core
