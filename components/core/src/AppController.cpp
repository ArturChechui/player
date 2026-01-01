#include "AppController.hpp"

namespace core {

AppController::AppController(const AppContext &ctx) : mAppContext(ctx) {}

bool AppController::init() {

  mAppModel = mAppContext.mStationRepository.load();
  UiEvent event{mAppModel, UiEventType::Render};
  mAppContext.mUiTask.post(event);

  return true;
}

} // namespace core