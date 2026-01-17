#pragma once

// Core
#include "AppController.hpp"
#include "UiTask.hpp"

// Adapters
#include "EspI2cBus.hpp"
#include "EspI2sBus.hpp"
#include "EspI2sPlayer.hpp"
#include "OledSsd1306Display.hpp"

// Services
#include "PlayerService.hpp"
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
    std::unique_ptr<adapters::EspI2sBus> mI2sBus;
    std::unique_ptr<adapters::EspI2sPlayer> mEspI2sPlayer;

    std::unique_ptr<adapters::OledSsd1306Display> mOledDisplay;
    std::unique_ptr<services::PlayerService> mPlayerService;
    std::unique_ptr<services::StationRepository> mStationRepository;
    std::unique_ptr<services::UiService> mUiService;
    std::unique_ptr<UiTask> mUiTask;
    std::unique_ptr<AppController> mAppController;
};

}  // namespace core
