#pragma once

#include <string>

#include "BoardConfig.hpp"
#include "driver/i2s_std.h"
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

    // I2S
    i2s_chan_handle_t mI2sTxHandle = nullptr;

    // Task
    TaskHandle_t mPlayerTask = nullptr;

    // State
    common::PlayerStatusCallback mStatusCb = nullptr;
    volatile bool mIsPlaying = false;
    std::string mCurrentUrl;

   private:
    bool initI2s();
    void deinitI2s();

    static void playerTaskFn(void* arg);
    void playerLoop();

    void writeTone440Hz();
};

}  // namespace adapters
