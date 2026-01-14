#pragma once

// Core
#include "AppController.hpp"
#include "UiTask.hpp"

// Adapters
#include "EspI2cBus.hpp"
#include "OledSsd1306Display.hpp"

// Services
#include "StationRepository.hpp"
#include "UiService.hpp"

// Standard
#include <memory>

namespace core {
class AppContext {
   public:
    AppContext();
    bool init();

   private:
    std::unique_ptr<adapters::EspI2cBus> mI2cBus;
    std::unique_ptr<adapters::OledSsd1306Display> mOledDisplay;
    std::unique_ptr<services::StationRepository> mStationRepository;
    std::unique_ptr<services::UiService> mUiService;
    std::unique_ptr<UiTask> mUiTask;
    std::unique_ptr<AppController> mAppController;
};

}  // namespace core
