#pragma once

#include <memory>
#include <string>

#include "AudioTypes.hpp"
#include "IPlayer.hpp"

namespace services {

class PlayerService {
   public:
    explicit PlayerService(adapters::IPlayer& player);
    ~PlayerService() = default;

    /**
     * @brief Initialize player service
     * @return true if successful
     */
    bool init();

    /**
     * @brief Play a station URL
     * @param url Radio station URL (e.g., http://play.global.audio/radio164)
     * @return true if playback started
     */
    bool playStation(const std::string& url);

    /**
     * @brief Stop current playback
     * @return true if successful
     */
    bool stop();

    /**
     * @brief Get current playback status
     */
    common::PlayerStatus getStatus() const;

    /**
     * @brief Get currently playing URL
     */
    std::string getCurrentUrl() const;

    /**
     * @brief Set callback for playback status changes
     */
    void setStatusCallback(common::PlayerStatusCallback cb);

   private:
    static constexpr const char* TAG = "PlayerService";

    adapters::IPlayer& mPlayer;
    common::PlayerStatus mStatus = common::PlayerStatus::IDLE;
    std::string mCurrentUrl;
    common::PlayerStatusCallback mStatusCb = nullptr;

    void onPlayerStatusChanged(common::PlayerStatus status);
};

}  // namespace services
