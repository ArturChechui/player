#include "EspI2sPlayer.hpp"

#include <cstring>

#include "driver/i2s_std.h"
#include "esp_http_client.h"
#include "esp_log.h"

namespace adapters {

EspI2sPlayer::EspI2sPlayer() {
    ESP_LOGI(TAG, "EspI2sPlayer created");
}

EspI2sPlayer::~EspI2sPlayer() {
    deinit();
}

bool EspI2sPlayer::init() {
    if (!initI2s()) {
        ESP_LOGE(TAG, "Failed to initialize I2S");
        return false;
    }
    ESP_LOGI(TAG, "EspI2sPlayer initialized");
    return true;
}

void EspI2sPlayer::deinit() {
    stop();
    deinitI2s();
    ESP_LOGI(TAG, "EspI2sPlayer deinitialized");
}

bool EspI2sPlayer::play(const std::string& url) {
    if (mIsPlaying) {
        ESP_LOGW(TAG, "Already playing, ignoring play request");
        return false;
    }

    if (url.empty()) {
        ESP_LOGE(TAG, "Empty URL");
        return false;
    }

    mCurrentUrl = url;
    mIsPlaying = true;

    ESP_LOGI(TAG, "Starting playback: %s", url.c_str());

    if (mStatusCb) {
        mStatusCb(common::PlayerStatus::BUFFERING);
    }

    // Create player task
    if (xTaskCreate(playerTaskFn, "PlayerTask", 4096, this, 5, &mPlayerTask) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create player task");
        mIsPlaying = false;
        if (mStatusCb) {
            mStatusCb(common::PlayerStatus::ERROR);
        }
        return false;
    }

    return true;
}

bool EspI2sPlayer::stop() {
    if (!mIsPlaying) {
        return true;
    }

    ESP_LOGI(TAG, "Stopping playback");
    mIsPlaying = false;

    if (mPlayerTask) {
        vTaskDelete(mPlayerTask);
        mPlayerTask = nullptr;
    }

    if (mStatusCb) {
        mStatusCb(common::PlayerStatus::STOPPED);
    }

    return true;
}

void EspI2sPlayer::setStatusCallback(common::PlayerStatusCallback cb) {
    mStatusCb = cb;
}

bool EspI2sPlayer::initI2s() {
    // I2S configuration for S3 SuperMini → external DAC
    i2s_std_config_t i2sConfig = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg =
            I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg =
            {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = static_cast<gpio_num_t>(common::I2S_BCLK_GPIO),
                .ws = static_cast<gpio_num_t>(common::I2S_LRCK_GPIO),
                .dout = static_cast<gpio_num_t>(common::I2S_DOUT_GPIO),
                .din = I2S_GPIO_UNUSED,
                .invert_flags =
                    {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv = false,
                    },
            },
    };

    esp_err_t ret = i2s_channel_init_std_mode(mI2sTxHandle, &i2sConfig);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S: %s", esp_err_to_name(ret));
        return false;
    }

    ret = i2s_channel_enable(mI2sTxHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S: %s", esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG, "I2S initialized (BCLK=%d, LRCK=%d, DOUT=%d)", common::I2S_BCLK_GPIO,
             common::I2S_LRCK_GPIO, common::I2S_DOUT_GPIO);
    return true;
}

void EspI2sPlayer::deinitI2s() {
    if (mI2sTxHandle) {
        i2s_channel_disable(mI2sTxHandle);
        mI2sTxHandle = nullptr;
        ESP_LOGI(TAG, "I2S deinitialized");
    }
}

void EspI2sPlayer::playerTaskFn(void* arg) {
    auto* player = static_cast<EspI2sPlayer*>(arg);
    player->playerLoop();
    vTaskDelete(nullptr);
}

void EspI2sPlayer::playerLoop() {
    ESP_LOGI(TAG, "Player loop started for: %s", mCurrentUrl.c_str());

    // TODO FR-03: Implement HTTP streaming + MP3 decoding
    // For now, simulate playback
    if (mStatusCb) {
        mStatusCb(common::PlayerStatus::PLAYING);
    }

    // Simulate 10 seconds of playback
    for (int i = 0; i < 100 && mIsPlaying; i++) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (mIsPlaying) {
        ESP_LOGI(TAG, "Playback completed");
        mIsPlaying = false;
        if (mStatusCb) {
            mStatusCb(common::PlayerStatus::STOPPED);
        }
    }

    ESP_LOGI(TAG, "Player loop exiting");
}

}  // namespace adapters
