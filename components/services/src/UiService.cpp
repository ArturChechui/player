#include "UiService.hpp"

#include "esp_log.h"

namespace services {
static const char *TAG = "UiService";

UiService::UiService(adapters::IDisplay &display,
                     StationRepository &stationRepo)
    : mDisplay(display), mStationRepo(stationRepo) {}

bool UiService::init() { return false; }

void UiService::onEvent(const core::UiEvent &e) {
  switch (e.type) {
  case core::UiEvent::Type::RENDER_BOOT:
    ESP_LOGI(TAG, "Rendering boot screen");
    break;
  case core::UiEvent::Type::RENDER_STATIONS:
    ESP_LOGI(TAG, "Rendering stations");
    mDisplay.showStations(mStationRepo.getStations(), e.selectedIndex);
    break;
  case core::UiEvent::Type::RENDER_STATUS:
    ESP_LOGI(TAG, "Rendering UI status");
    // playback status rendering to be implemented
    break;
  default:
    ESP_LOGW(TAG, "Unknown UI event type");
    break;
  }
}

} // namespace services