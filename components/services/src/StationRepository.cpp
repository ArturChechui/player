#include "StationRepository.hpp"

// IDF
#include <esp_log.h>

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
    mStations = {{"radio1_aac_h", "Radio 1 (AAC High)",
                  "https://playerservices.streamtheworld.com/api/livestream-redirect/"
                  "RADIO_1AAC_H.aac"},
                 {"radio1_aac_m", "Radio 1 (AAC Med)",
                  "https://playerservices.streamtheworld.com/api/livestream-redirect/"
                  "RADIO_1AAC_M.aac"},
                 {"example_mp3", "Example MP3 Station", "http://example.com/stream.mp3"}};

    ESP_LOGI(TAG, "Loaded %d stations", static_cast<int>(mStations.size()));

    mInitialized = true;
    return true;
}

const std::vector<common::StationData> &StationRepository::getStations() const {
    if (!mInitialized) {
        ESP_LOGW(TAG, "Not initialized yet");
    }

    return mStations;
}

}  // namespace services
