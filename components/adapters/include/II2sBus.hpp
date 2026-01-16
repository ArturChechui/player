#pragma once

#include <cstddef>
#include <cstdint>

namespace adapters {

/**
 * @brief I2S bus interface for audio output
 *
 * Abstraction over ESP-IDF I2S driver for speaker/codec communication.
 * Follows same ownership pattern as II2cBus.
 */
class II2sBus {
   public:
    virtual ~II2sBus() = default;

    /**
     * @brief Initialize I2S hardware with configured pins and sample rate
     * @return true if initialization succeeded, false otherwise
     */
    virtual bool init() = 0;

    /**
     * @brief Clean up I2S resources
     */
    virtual void deinit() = 0;

    /**
     * @brief Write audio samples to I2S output
     * @param data Pointer to audio sample buffer
     * @param size Number of bytes to write
     * @param bytesWritten Output: actual bytes written
     * @param timeoutMs Timeout in milliseconds
     * @return true if write succeeded, false on timeout/error
     */
    virtual bool write(const uint8_t* data, size_t size, size_t& bytesWritten,
                       uint32_t timeoutMs) = 0;

    /**
     * @brief Check if I2S is initialized and ready
     * @return true if bus is available for writes
     */
    virtual bool isAvailable() const = 0;
};

}  // namespace adapters
