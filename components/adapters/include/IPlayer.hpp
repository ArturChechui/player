#pragma once

#include <string>

#include "AudioTypes.hpp"

namespace adapters {

/**
 * @brief Audio player interface for internet radio streaming
 *
 * Manages playback pipeline (HTTP stream -> decoder -> I2S output).
 * Follows same lifecycle pattern as I2C/I2S adapters.
 */
class IPlayer {
   public:
    virtual ~IPlayer() = default;

    /**
     * @brief Initialize audio pipeline elements
     * @return true if pipeline setup succeeded
     */
    virtual bool init() = 0;

    /**
     * @brief Clean up pipeline resources
     */
    virtual void deinit() = 0;

    /**
     * @brief Start streaming from URL
     * @param url HTTP stream URL (MP3 format)
     * @return true if playback started, false if already playing or error
     */
    virtual bool play(const std::string& url) = 0;

    /**
     * @brief Stop current playback
     * @return true if stopped successfully
     */
    virtual bool stop() = 0;

    /**
     * @brief Register callback for playback status changes
     * @param cb Callback invoked on BUFFERING/PLAYING/STOPPED/ERROR
     */
    virtual void setStatusCallback(common::PlayerStatusCallback cb) = 0;
};

}  // namespace adapters
