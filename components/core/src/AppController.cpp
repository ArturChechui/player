#include "AppController.hpp"
#include "AppContext.hpp"

namespace core {

AppController::AppController(AppContext &ctx) : mAppContext(ctx) {}

bool AppController::init() {

  UiEvent event;
  event.type = UiEvent::Type::RENDER_STATIONS;
  event.selectedIndex = 0;

  mAppContext.mUiTask->post(event);

  return true;
}

} // namespace core