#include "AppContext.hpp"

namespace core {
AppContext::AppContext() {}

bool AppContext::init() {
  mAppController = AppController(*this);
  mUiService = services::UiService(mOledDisplay);
  mOledDisplay = adapters::OledSsd1306Display();
  mUiTask = UiTask(mUiService);

  mOledDisplay.Init();
  mUiService.init();
  mStationRepository.init();
  mUiTask.init();

  return true;
}

} // namespace core
