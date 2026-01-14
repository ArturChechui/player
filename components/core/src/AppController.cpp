#include "AppController.hpp"

#include "IUiSink.hpp"
#include "UiTypes.hpp"

// IDF
#include <esp_log.h>

namespace core {
constexpr const char* TAG = "AppController";

AppController::AppController(IUiSink& uiSink) : mUiSink(uiSink) {}

bool AppController::init() {
    ESP_LOGI(TAG, "Initializing AppController");

    common::UiEvent event;
    event.type = common::UiEvent::Type::RENDER_STATIONS;
    event.selectedIndex = 0;

    mUiSink.post(event);

    return true;
}
}  // namespace core
