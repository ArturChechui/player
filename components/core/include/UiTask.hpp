#pragma once

// TODO: Add forward declarations to reduce dependencies
#include "IUiSink.hpp"
#include "UiService.hpp"
#include "types.hpp"
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "types.hpp"

namespace core {

class UiTask final : public IUiSink {
public:
  explicit UiTask(services::UiService &ui);

  bool init();

  // IUiSink
  void post(const UiEvent &e) override;

  // Run the task loop (called as FreeRTOS task)
  void runLoop();

  // Static wrapper for FreeRTOS xTaskCreate
  static void taskEntry(void *pvParameters);

private:
  static constexpr uint32_t QUEUE_LENGTH = 5;
  static constexpr uint32_t QUEUE_ITEM_SIZE = sizeof(UiEvent);
  static constexpr uint32_t TASK_STACK_SIZE = 4096;
  static constexpr uint32_t TASK_PRIORITY = 5;

  QueueHandle_t mUiQueue;
  services::UiService &mUiService;
};

} // namespace core
