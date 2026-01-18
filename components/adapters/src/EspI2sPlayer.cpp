#include "EspI2sPlayer.hpp"

#include <cmath>
#include <cstring>

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

    BaseType_t ok = xTaskCreate(playerTaskFn, "PlayerTask", 4096, this, 5, &mPlayerTask);

    if (ok != pdPASS) {
        ESP_LOGE(TAG, "Failed to create player task");
        mIsPlaying = false;
        mPlayerTask = nullptr;
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

    ESP_LOGI(TAG, "Stopping playback...");
    mIsPlaying = false;

    // Wait until player task exits by itself
    while (mPlayerTask != nullptr) {
        vTaskDelay(pdMS_TO_TICKS(10));
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
    if (mI2sTxHandle != nullptr) {
        ESP_LOGW(TAG, "I2S already initialized");
        return true;
    }

    // 1) Create I2S TX channel first ✅
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);

    esp_err_t ret = i2s_new_channel(&chan_cfg, &mI2sTxHandle, nullptr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_new_channel failed: %s", esp_err_to_name(ret));
        return false;
    }

    // 2) Configure I2S STD mode (Philips)
    i2s_std_config_t std_cfg = {
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

    ret = i2s_channel_init_std_mode(mI2sTxHandle, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_channel_init_std_mode failed: %s", esp_err_to_name(ret));
        i2s_del_channel(mI2sTxHandle);
        mI2sTxHandle = nullptr;
        return false;
    }

    ret = i2s_channel_enable(mI2sTxHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_channel_enable failed: %s", esp_err_to_name(ret));
        i2s_del_channel(mI2sTxHandle);
        mI2sTxHandle = nullptr;
        return false;
    }

    ESP_LOGI(TAG, "I2S initialized (BCLK=%d, LRCK=%d, DOUT=%d)", common::I2S_BCLK_GPIO,
             common::I2S_LRCK_GPIO, common::I2S_DOUT_GPIO);

    return true;
}

void EspI2sPlayer::deinitI2s() {
    if (!mI2sTxHandle) {
        return;
    }

    i2s_channel_disable(mI2sTxHandle);
    i2s_del_channel(mI2sTxHandle);
    mI2sTxHandle = nullptr;

    ESP_LOGI(TAG, "I2S deinitialized");
}

void EspI2sPlayer::playerTaskFn(void* arg) {
    auto* player = static_cast<EspI2sPlayer*>(arg);
    player->playerLoop();

    // Mark task as finished
    player->mPlayerTask = nullptr;
    vTaskDelete(nullptr);
}

void EspI2sPlayer::playerLoop() {
    ESP_LOGI(TAG, "Player loop started for: %s", mCurrentUrl.c_str());

    if (mStatusCb) {
        mStatusCb(common::PlayerStatus::PLAYING);
    }

    // For now: output real I2S audio (440Hz sine) for ~10 seconds
    writeTone440Hz();

    if (mIsPlaying) {
        // finished naturally
        mIsPlaying = false;
        if (mStatusCb) {
            mStatusCb(common::PlayerStatus::STOPPED);
        }
    }

    ESP_LOGI(TAG, "Player loop exiting");
}

void EspI2sPlayer::writeTone440Hz() {
    if (!mI2sTxHandle) {
        ESP_LOGE(TAG, "I2S not initialized");
        if (mStatusCb) {
            mStatusCb(common::PlayerStatus::ERROR);
        }
        mIsPlaying = false;
        return;
    }

    constexpr int sampleRate = 44100;
    constexpr float freq = 440.0f;
    constexpr float amplitude = 12000.0f;  // avoid clipping
    constexpr int framesPerChunk = 256;    // small & stable

    int16_t buffer[framesPerChunk * 2];  // stereo: L,R interleaved
    float phase = 0.0f;
    constexpr float twoPi = 6.28318530718f;

    // Play for ~10 seconds
    const int totalFrames = sampleRate * 10;
    int framesWritten = 0;

    while (mIsPlaying && framesWritten < totalFrames) {
        for (int i = 0; i < framesPerChunk; i++) {
            float s = sinf(phase) * amplitude;
            int16_t sample = static_cast<int16_t>(s);

            // stereo
            buffer[i * 2 + 0] = sample;
            buffer[i * 2 + 1] = sample;

            phase += twoPi * freq / static_cast<float>(sampleRate);
            if (phase >= twoPi) {
                phase -= twoPi;
            }
        }

        size_t bytesWritten = 0;
        esp_err_t ret =
            i2s_channel_write(mI2sTxHandle, buffer, sizeof(buffer), &bytesWritten, portMAX_DELAY);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "i2s_channel_write failed: %s", esp_err_to_name(ret));
            if (mStatusCb) {
                mStatusCb(common::PlayerStatus::ERROR);
            }
            mIsPlaying = false;
            return;
        }

        framesWritten += framesPerChunk;
    }
}

}  // namespace adapters
