#pragma once

#include <memory>

#include "BoardConfig.hpp"
#include "driver/i2s_std.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "IPlayer.hpp"

namespace adapters {

class EspI2sPlayer : public IPlayer {
   public:
    EspI2sPlayer();
    ~EspI2sPlayer() override;

    bool init() override;
    void deinit() override;
    bool play(const std::string& url) override;
    bool stop() override;
    void setStatusCallback(common::PlayerStatusCallback cb) override;

   private:
    static constexpr const char* TAG = "EspI2sPlayer";

    i2s_chan_handle_t mI2sTxHandle = nullptr;
    TaskHandle_t mPlayerTask = nullptr;
    common::PlayerStatusCallback mStatusCb = nullptr;
    bool mIsPlaying = false;
    std::string mCurrentUrl;

    bool initI2s();
    void deinitI2s();

    static void playerTaskFn(void* arg);
    void playerLoop();
};

}  // namespace adapters
