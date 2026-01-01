#pragma once

#include "IUiSink.hpp"
#include "UiService.hpp"
#include "types.hpp"

#include <cstdint>

#include "queue.h"

namespace core {

class UiTask final : public IUiSink {
public:
  explicit UiTask(services::UiService &ui);

  bool init();

  // IUiSink
  void post(const UiEvent &e) override;

private:
  void runLoop();

  QueueHandle_t mUiQueue;

  const services::UiService &mUiService;
};

} // namespace core
