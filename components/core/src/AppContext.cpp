#include "AppContext.hpp"

// Common
#include "BoardConfig.hpp"

namespace core {
AppContext::AppContext()
    : mI2cBus(std::make_unique<adapters::EspI2cBus>(common::I2C_PORT)),
      mI2sBus(std::make_unique<adapters::EspI2sBus>(common::I2S_PORT)),
      mEspI2sPlayer(std::make_unique<adapters::EspI2sPlayer>()),
      mOledDisplay(std::make_unique<adapters::OledSsd1306Display>(*mI2cBus)),
      mPlayerService(std::make_unique<services::PlayerService>(*mEspI2sPlayer)),
      mStationRepository(std::make_unique<services::StationRepository>()),
      mUiService(std::make_unique<services::UiService>(*mOledDisplay, *mStationRepository)),
      mUiTask(std::make_unique<UiTask>(*mUiService)),
      mAppController(std::make_unique<AppController>(*mUiTask)) {}

bool AppContext::init() {
    mI2cBus->init();
    mOledDisplay->init();
    mStationRepository->init();
    mUiService->init();
    mUiTask->init();
    mAppController->init();

    return true;
}

}  // namespace core
