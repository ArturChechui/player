#pragma once

#include <driver/i2s_std.h>

#include "II2sBus.hpp"

namespace adapters {

class EspI2sBus : public II2sBus {
   public:
    explicit EspI2sBus(const int& port);
    ~EspI2sBus() override;

    // Prevent copying
    EspI2sBus(const EspI2sBus&) = delete;
    EspI2sBus& operator=(const EspI2sBus&) = delete;

    bool init() override;
    void deinit() override;
    bool write(const uint8_t* data, const size_t& size, size_t& bytesWritten,
               const uint32_t& timeoutMs) override;
    bool isAvailable() const override;

   private:
    int mPort;
    i2s_chan_handle_t mTxHandle;
};

}  // namespace adapters
