#pragma once

#include <atomic>
#include <string>

#include "BoardConfig.hpp"
#include "driver/i2s_std.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
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
    uint32_t mI2sSampleRate = 44100;

    // HTTP
    esp_http_client_handle_t mHttpClient = nullptr;
    SemaphoreHandle_t mHttpClientMutex = nullptr;

    // Task
    TaskHandle_t mPlayerTask = nullptr;

    // State
    common::PlayerStatusCallback mStatusCb = nullptr;
    std::atomic_bool mIsPlaying{false};
    std::atomic_bool mStopRequested{false};
    std::string mCurrentUrl;

    bool initI2s();
    void deinitI2s();
    bool reconfigureI2sClock(uint32_t sampleRate);

    bool openHttpStream(const std::string& url);
    void closeHttpStream();

    static void playerTaskFn(void* arg);
    void playerLoop();
};

}  // namespace adapters
