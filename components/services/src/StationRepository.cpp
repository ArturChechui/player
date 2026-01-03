#include "StationRepository.hpp"
#include "esp_log.h"

namespace services {
static const char *TAG = "StationRepository";

StationRepository::StationRepository() : mStations(), mInitialized(false) {
  ESP_LOGI(TAG, "StationRepository created");
}

bool StationRepository::init() {
  if (mInitialized) {
    ESP_LOGW(TAG, "Already initialized, ignoring");
    return true;
  }

  // TODO: Load from LittleFS stations.json
  // For now, hardcoded stations for FR-01
  mStations = {
      {"radio1_aac_h", "Radio 1 (AAC High)",
       "https://playerservices.streamtheworld.com/api/livestream-redirect/"
       "RADIO_1AAC_H.aac"},
      {"radio1_aac_m", "Radio 1 (AAC Medium)",
       "https://playerservices.streamtheworld.com/api/livestream-redirect/"
       "RADIO_1AAC_M.aac"},
      {"example_mp3", "Example MP3 Station", "http://example.com/stream.mp3"}};

  ESP_LOGI(TAG, "Loaded %d stations", mStations.size());

  return true;
}

const std::vector<core::StationData> &StationRepository::getStations() const {
  if (!mInitialized) {
    ESP_LOGW(TAG, "Not initialized yet");
  }

  return mStations;
}

} // namespace services