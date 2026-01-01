#pragma once

// Core
#include "AppController.hpp"
#include "UiTask.hpp"

// Adapters
#include "OledSsd1306Display.hpp"

// Services
#include "StationRepository.hpp"
#include "UiService.hpp"

namespace core {

class AppContext {
public:
  explicit AppContext();
  bool init();

public:
  AppController mAppController;
  services::StationRepository mStationRepository;
  services::UiService mUiService;
  adapters::OledSsd1306Display mOledDisplay;
  UiTask mUiTask;
};

} // namespace core