#pragma once

#include <cstdint>
#include <vector>

#include "IDisplay.hpp"

namespace adapters {

class II2cBus;

class OledSsd1306Display final : public IDisplay {
   public:
    explicit OledSsd1306Display(II2cBus &i2cBus);
    ~OledSsd1306Display() override;

    // IDisplay
    bool init() override;
    void showFramebuffer(const uint8_t *framebuffer, const size_t &len) override;

   private:
    void sendInitSequence();
    void writeCommand(const uint8_t *cmd, const uint16_t &len);
    void writeData(const uint8_t *data, const uint16_t &len);

    II2cBus &mI2cBus;
    uint8_t mI2cAddr;
    bool mReady;
};

}  // namespace adapters
