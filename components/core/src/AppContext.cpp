#include "AppContext.hpp"

namespace core {

bool AppContext::init() {
  // TODO: move to constructor initialization list?
  mAppController = std::make_unique<AppController>(*this);
  mOledDisplay = std::make_unique<adapters::OledSsd1306Display>();
  mStationRepository = std::make_unique<services::StationRepository>();
  mUiService =
      std::make_unique<services::UiService>(*mOledDisplay, *mStationRepository);
  mUiTask = std::make_unique<UiTask>(*mUiService);

  mOledDisplay->init();
  mUiService->init();
  mStationRepository->init();
  mUiTask->init();

  // send the event
  mAppController->init();

  return true;
}

} // namespace core
