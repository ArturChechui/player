#pragma once

#include <string>

#include "AudioTypes.hpp"
#include "IPlayer.hpp"

namespace services {

/**
 * @brief Manages audio playback and player state
 *
 * Wraps IPlayer adapter, tracks current station/status, and posts UI events
 * when playback state changes. Follows same service pattern as UiService.
 */
class PlayerService {
   public:
    /**
     * @brief Construct PlayerService with player adapter
     * @param player Audio player adapter (injected, owned by AppContext)
     */
    explicit PlayerService(adapters::IPlayer& player);

    /**
     * @brief Initialize player service and register status callback
     * @return true if player adapter initialized successfully
     */
    bool init();

    /**
     * @brief Start playback of a station URL
     * @param stationUrl HTTP stream URL (MP3 format)
     * @return true if playback started, false if already playing or error
     */
    bool playStation(const std::string& stationUrl);

    /**
     * @brief Stop current playback
     * @return true if stopped successfully
     */
    bool stop();

    /**
     * @brief Get current playback status
     * @return Current PlayerStatus (IDLE/BUFFERING/PLAYING/STOPPED/ERROR)
     */
    common::PlayerStatus getStatus() const;

    /**
     * @brief Get currently playing station URL
     * @return Empty string if not playing
     */
    std::string getCurrentUrl() const;

   private:
    static constexpr const char* TAG = "PlayerService";

    adapters::IPlayer& mPlayer;
    common::PlayerStatus mStatus = common::PlayerStatus::IDLE;
    std::string mCurrentUrl;

    /**
     * @brief Handle player status changes from adapter callback
     * @param status New status reported by player
     */
    void onPlayerStatusChanged(common::PlayerStatus status);
};

}  // namespace services
