#pragma once

#include <driver/i2s_std.h>

#include "II2sBus.hpp"

namespace adapters {

/**
 * @brief ESP-IDF I2S bus implementation
 *
 * Wraps ESP-IDF v5.x I2S standard mode driver for audio output.
 * Pins and sample rate configured from BoardConfig.hpp.
 */
class EspI2sBus : public II2sBus {
   public:
    /**
     * @brief Construct I2S bus for specified port
     * @param port I2S port number (I2S_NUM_0 or I2S_NUM_1)
     */
    explicit EspI2sBus(int port);
    ~EspI2sBus() override;

    // Prevent copying
    EspI2sBus(const EspI2sBus&) = delete;
    EspI2sBus& operator=(const EspI2sBus&) = delete;

    bool init() override;
    void deinit() override;
    bool write(const uint8_t* data, size_t size, size_t& bytesWritten, uint32_t timeoutMs) override;
    bool isAvailable() const override;

   private:
    static constexpr const char* TAG = "EspI2sBus";

    int mPort;
    i2s_chan_handle_t mTxHandle = nullptr;
};

}  // namespace adapters
