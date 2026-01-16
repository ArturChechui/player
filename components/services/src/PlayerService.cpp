#include "PlayerService.hpp"

#include <esp_log.h>

namespace services {

PlayerService::PlayerService(adapters::IPlayer& player) : mPlayer(player) {
    ESP_LOGI(TAG, "Creating PlayerService");
}

bool PlayerService::init() {
    ESP_LOGI(TAG, "Initializing PlayerService");

    // Register status callback before initializing player
    mPlayer.setStatusCallback(
        [this](common::PlayerStatus status) { onPlayerStatusChanged(status); });

    if (!mPlayer.init()) {
        ESP_LOGE(TAG, "Failed to initialize player adapter");
        return false;
    }

    mStatus = common::PlayerStatus::IDLE;
    mCurrentUrl.clear();

    ESP_LOGI(TAG, "PlayerService initialized");
    return true;
}

bool PlayerService::playStation(const std::string& stationUrl) {
    if (stationUrl.empty()) {
        ESP_LOGE(TAG, "Empty station URL provided");
        return false;
    }

    if (mStatus == common::PlayerStatus::PLAYING || mStatus == common::PlayerStatus::BUFFERING) {
        ESP_LOGW(TAG, "Already playing %s, ignoring new request", mCurrentUrl.c_str());
        return false;
    }

    ESP_LOGI(TAG, "Requesting playback: %s", stationUrl.c_str());

    if (!mPlayer.play(stationUrl)) {
        ESP_LOGE(TAG, "Failed to start playback");
        mStatus = common::PlayerStatus::ERROR;
        return false;
    }

    mCurrentUrl = stationUrl;
    // Status will be updated via callback (BUFFERING → PLAYING)
    return true;
}

bool PlayerService::stop() {
    if (mStatus == common::PlayerStatus::IDLE || mStatus == common::PlayerStatus::STOPPED) {
        ESP_LOGW(TAG, "Not playing, ignoring stop request");
        return true;  // Not an error
    }

    ESP_LOGI(TAG, "Stopping playback of %s", mCurrentUrl.c_str());

    if (!mPlayer.stop()) {
        ESP_LOGE(TAG, "Failed to stop player");
        return false;
    }

    // Status will be updated via callback (STOPPED)
    mCurrentUrl.clear();
    return true;
}

common::PlayerStatus PlayerService::getStatus() const {
    return mStatus;
}

std::string PlayerService::getCurrentUrl() const {
    return mCurrentUrl;
}

void PlayerService::onPlayerStatusChanged(common::PlayerStatus status) {
    if (mStatus == status) {
        return;  // No change
    }

    ESP_LOGI(TAG, "Player status changed: %d → %d", static_cast<int>(mStatus),
             static_cast<int>(status));

    mStatus = status;

    // TODO: Post UiEvent to UiTask when playback state changes
    // e.g., UiEvent::PLAYBACK_BUFFERING, UiEvent::PLAYBACK_PLAYING, UiEvent::PLAYBACK_ERROR
    // For now, just log status updates
}

}  // namespace services
