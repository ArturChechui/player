#pragma once

// Core
#include "AppController.hpp"
#include "UiTask.hpp"

// Adapters
#include "OledSsd1306Display.hpp"

// Services
#include "StationRepository.hpp"
#include "UiService.hpp"

#include <memory>

namespace core {

class AppContext {
public:
  AppContext() = default;
  bool init();

public:
  std::unique_ptr<AppController> mAppController;
  std::unique_ptr<services::StationRepository> mStationRepository;
  std::unique_ptr<services::UiService> mUiService;
  std::unique_ptr<adapters::OledSsd1306Display> mOledDisplay;
  std::unique_ptr<UiTask> mUiTask;
};

} // namespace core