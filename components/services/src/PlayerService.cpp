#include "PlayerService.hpp"

#include "esp_log.h"

namespace services {

PlayerService::PlayerService(adapters::IPlayer& player) : mPlayer(player) {
    ESP_LOGI(TAG, "PlayerService created");
}

bool PlayerService::init() {
    ESP_LOGI(TAG, "Initializing PlayerService");

    // Set our callback as the player's status callback
    mPlayer.setStatusCallback(
        [this](common::PlayerStatus status) { onPlayerStatusChanged(status); });

    // Initialize player hardware
    if (!mPlayer.init()) {
        ESP_LOGE(TAG, "Failed to initialize player");
        mStatus = common::PlayerStatus::ERROR;
        return false;
    }

    mStatus = common::PlayerStatus::IDLE;
    ESP_LOGI(TAG, "PlayerService initialized successfully");
    return true;
}

bool PlayerService::playStation(const std::string& url) {
    if (url.empty()) {
        ESP_LOGE(TAG, "Empty URL");
        return false;
    }

    if (mStatus == common::PlayerStatus::PLAYING || mStatus == common::PlayerStatus::BUFFERING) {
        ESP_LOGW(TAG, "Already playing: %s", mCurrentUrl.c_str());
        return false;
    }

    ESP_LOGI(TAG, "Playing station: %s", url.c_str());

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
        ESP_LOGW(TAG, "Not playing, nothing to stop");
        return true;
    }

    ESP_LOGI(TAG, "Stopping playback");
    bool result = mPlayer.stop();

    if (result) {
        mStatus = common::PlayerStatus::STOPPED;
        mCurrentUrl.clear();
    }

    return result;
}

common::PlayerStatus PlayerService::getStatus() const {
    return mStatus;
}

std::string PlayerService::getCurrentUrl() const {
    return mCurrentUrl;
}

void PlayerService::setStatusCallback(common::PlayerStatusCallback cb) {
    mStatusCb = cb;
}

void PlayerService::onPlayerStatusChanged(common::PlayerStatus status) {
    if (mStatus == status) {
        return;  // No change
    }

    ESP_LOGI(TAG, "Status changed: %d → %d", static_cast<int>(mStatus), static_cast<int>(status));
    mStatus = status;

    // Invoke user callback if set
    if (mStatusCb) {
        mStatusCb(status);
    }

    // TODO FR-04: Post UiEvent to display playback status
    // Example:
    // mUiEventQueue.post(UiEvent::PLAYBACK_STATUS_CHANGED{
    //     .url = mCurrentUrl,
    //     .status = status
    // });
}

}  // namespace services
