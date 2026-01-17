#include "PlayerService.hpp"

#include "esp_log.h"

namespace services {

PlayerService::PlayerService(adapters::IPlayer& player) : mPlayer(player) {
    ESP_LOGI(TAG, "PlayerService created");
}

bool PlayerService::init() {
    ESP_LOGI(TAG, "Initializing PlayerService");

    mPlayer.setStatusCallback(
        [this](common::PlayerStatus status) { onPlayerStatusChanged(status); });

    if (!mPlayer.init()) {
        ESP_LOGE(TAG, "Failed to initialize player");
        return false;
    }

    mStatus = common::PlayerStatus::IDLE;
    ESP_LOGI(TAG, "PlayerService initialized");
    return true;
}

bool PlayerService::playStation(const std::string& url) {
    if (url.empty()) {
        ESP_LOGE(TAG, "Empty URL");
        return false;
    }

    if (mStatus == common::PlayerStatus::PLAYING || mStatus == common::PlayerStatus::BUFFERING) {
        ESP_LOGW(TAG, "Already playing %s", mCurrentUrl.c_str());
        return false;
    }

    ESP_LOGI(TAG, "Playing: %s", url.c_str());

    if (!mPlayer.play(url)) {
        ESP_LOGE(TAG, "Failed to start playback");
        mStatus = common::PlayerStatus::ERROR;
        return false;
    }

    mCurrentUrl = url;
    return true;
}

bool PlayerService::stop() {
    if (mStatus == common::PlayerStatus::IDLE || mStatus == common::PlayerStatus::STOPPED) {
        return true;
    }

    ESP_LOGI(TAG, "Stopping playback");
    return mPlayer.stop();
}

common::PlayerStatus PlayerService::getStatus() const {
    return mStatus;
}

void PlayerService::onPlayerStatusChanged(common::PlayerStatus status) {
    if (mStatus == status)
        return;

    ESP_LOGI(TAG, "Status: %d → %d", static_cast<int>(mStatus), static_cast<int>(status));
    mStatus = status;

    // TODO FR-04: Post UiEvent to display playback status
}

}  // namespace services
