#include "EspI2sPlayer.hpp"

#include <algorithm>
#include <cstring>
#include <vector>

#include "esp_crt_bundle.h"
#include "esp_err.h"
#include "esp_log.h"
#include "minimp3.h"

namespace adapters {

namespace {
constexpr int kTaskStack = 8192;
constexpr int kTaskPrio = 5;

constexpr int kReadChunk = 2048;
constexpr size_t kInputBufferSize = 32 * 1024;

// I2S DMA tuning: larger frames reduce CPU wakeups while streaming
constexpr int kDmaDescNum = 12;
constexpr int kDmaFrameNum = 256;
}  // namespace

EspI2sPlayer::EspI2sPlayer() {
    ESP_LOGI(TAG, "EspI2sPlayer created");
    mHttpClientMutex = xSemaphoreCreateMutex();
}

EspI2sPlayer::~EspI2sPlayer() {
    deinit();
    if (mHttpClientMutex) {
        vSemaphoreDelete(mHttpClientMutex);
        mHttpClientMutex = nullptr;
    }
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
    if (mIsPlaying.load()) {
        ESP_LOGW(TAG, "Already playing, ignoring play request");
        return false;
    }

    if (url.empty()) {
        ESP_LOGE(TAG, "Empty URL");
        return false;
    }

    mCurrentUrl = url;
    mStopRequested.store(false);
    mIsPlaying.store(true);

    ESP_LOGI(TAG, "Starting playback: %s", url.c_str());

    if (mStatusCb) {
        mStatusCb(common::PlayerStatus::BUFFERING);
    }

    if (xTaskCreate(playerTaskFn, "PlayerTask", kTaskStack, this, kTaskPrio, &mPlayerTask) !=
        pdPASS) {
        ESP_LOGE(TAG, "Failed to create player task");
        mIsPlaying.store(false);
        if (mStatusCb) {
            mStatusCb(common::PlayerStatus::ERROR);
        }
        return false;
    }

    return true;
}

bool EspI2sPlayer::stop() {
    if (!mIsPlaying.load()) {
        return true;
    }

    ESP_LOGI(TAG, "Stopping playback");
    mStopRequested.store(true);
    mIsPlaying.store(false);

    // Closing HTTP stream will unblock esp_http_client_read()
    closeHttpStream();

    // Let the task exit by itself (safer than vTaskDelete from another context)
    for (int i = 0; i < 50 && mPlayerTask != nullptr; ++i) {
        vTaskDelay(pdMS_TO_TICKS(20));
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
    if (mI2sTxHandle) {
        return true;
    }

    // Create TX channel
    i2s_chan_config_t chanCfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chanCfg.dma_desc_num = kDmaDescNum;
    chanCfg.dma_frame_num = kDmaFrameNum;

    esp_err_t ret = i2s_new_channel(&chanCfg, &mI2sTxHandle, nullptr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_new_channel failed: %s", esp_err_to_name(ret));
        mI2sTxHandle = nullptr;
        return false;
    }

    // Standard I2S (Philips) config for MAX98357A
    i2s_std_config_t stdCfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(mI2sSampleRate),
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

    ret = i2s_channel_init_std_mode(mI2sTxHandle, &stdCfg);
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

    // ESP_LOGI(TAG, "I2S initialized (SR=%d, BCLK=%d, LRCK=%d, DOUT=%d)", mI2sSampleRate,
    //          common::I2S_BCLK_GPIO, common::I2S_LRCK_GPIO, common::I2S_DOUT_GPIO);
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

bool EspI2sPlayer::reconfigureI2sClock(uint32_t sampleRate) {
    if (!mI2sTxHandle || sampleRate == 0 || sampleRate == mI2sSampleRate) {
        return true;
    }

    i2s_std_clk_config_t clkCfg = I2S_STD_CLK_DEFAULT_CONFIG(sampleRate);
    esp_err_t ret = i2s_channel_reconfig_std_clock(mI2sTxHandle, &clkCfg);
    if (ret != ESP_OK) {
        // ESP_LOGW(TAG, "I2S clock reconfig failed (%d -> %d): %s", mI2sSampleRate, sampleRate,
        //          esp_err_to_name(ret));
        return false;
    }

    // ESP_LOGI(TAG, "I2S clock reconfigured: %d -> %d", mI2sSampleRate, sampleRate);
    mI2sSampleRate = sampleRate;
    return true;
}

bool EspI2sPlayer::openHttpStream(const std::string& url) {
    // If user calls stop() quickly, avoid starting the HTTP session
    if (mStopRequested.load()) {
        return false;
    }

    esp_http_client_config_t cfg = {};
    cfg.url = url.c_str();
    cfg.timeout_ms = 8000;
    cfg.buffer_size = kReadChunk;
    cfg.buffer_size_tx = 1024;
    cfg.keep_alive_enable = true;
    cfg.disable_auto_redirect = false;

#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
    cfg.crt_bundle_attach = esp_crt_bundle_attach;
#endif

    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) {
        ESP_LOGE(TAG, "esp_http_client_init failed");
        return false;
    }

    // Some radios may send ICY metadata only if requested. We don't want it.
    // If server sends ICY anyway, you'd need to parse and strip metadata blocks.
    esp_http_client_set_header(client, "Icy-MetaData", "0");

    esp_err_t ret = esp_http_client_open(client, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_http_client_open failed: %s", esp_err_to_name(ret));
        esp_http_client_cleanup(client);
        return false;
    }

    int status = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "HTTP status: %d", status);

    if (status != 200) {
        // Many stream endpoints will 302-redirect; IDF should follow automatically if possible.
        // If we still don't get 200, treat as error.
        ESP_LOGW(TAG, "Unexpected status code: %d", status);
    }

    // Publish the handle so stop() can close it to unblock reads
    if (mHttpClientMutex) {
        xSemaphoreTake(mHttpClientMutex, portMAX_DELAY);
    }
    mHttpClient = client;
    if (mHttpClientMutex) {
        xSemaphoreGive(mHttpClientMutex);
    }

    return true;
}

void EspI2sPlayer::closeHttpStream() {
    if (mHttpClientMutex) {
        xSemaphoreTake(mHttpClientMutex, portMAX_DELAY);
    }

    if (mHttpClient) {
        esp_http_client_close(mHttpClient);
        esp_http_client_cleanup(mHttpClient);
        mHttpClient = nullptr;
    }

    if (mHttpClientMutex) {
        xSemaphoreGive(mHttpClientMutex);
    }
}

void EspI2sPlayer::playerTaskFn(void* arg) {
    auto* player = static_cast<EspI2sPlayer*>(arg);
    player->playerLoop();

    // Mark the task as gone
    player->mPlayerTask = nullptr;
    vTaskDelete(nullptr);
}

void EspI2sPlayer::playerLoop() {
    ESP_LOGI(TAG, "Player loop started for: %s", mCurrentUrl.c_str());

    if (!openHttpStream(mCurrentUrl)) {
        ESP_LOGE(TAG, "Failed to open HTTP stream");
        if (mStatusCb) {
            mStatusCb(common::PlayerStatus::ERROR);
        }
        mIsPlaying.store(false);
        return;
    }

    if (mStatusCb) {
        mStatusCb(common::PlayerStatus::PLAYING);
    }

    // MP3 decoder
    mp3dec_t mp3d;
    mp3dec_init(&mp3d);

    std::vector<uint8_t> inbuf(kInputBufferSize);
    size_t inbuf_len = 0;

    // One MP3 frame max samples: 1152 * 2 channels
    int16_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];

    while (mIsPlaying.load() && !mStopRequested.load()) {
        // Ensure we have some data
        if (inbuf_len < 4096) {
            // Read from HTTP into the remaining space
            size_t free_space = inbuf.size() - inbuf_len;
            if (free_space < kReadChunk) {
                // If buffer is too full, drop some oldest data to recover
                size_t drop = std::min<size_t>(inbuf_len, 2048);
                memmove(inbuf.data(), inbuf.data() + drop, inbuf_len - drop);
                inbuf_len -= drop;
                free_space = inbuf.size() - inbuf_len;
            }

            int r = esp_http_client_read(
                mHttpClient, reinterpret_cast<char*>(inbuf.data() + inbuf_len), free_space);
            if (r < 0) {
                ESP_LOGE(TAG, "HTTP read error");
                break;
            }
            if (r == 0) {
                ESP_LOGW(TAG, "HTTP stream ended");
                break;
            }
            inbuf_len += static_cast<size_t>(r);
        }

        mp3dec_frame_info_t info = {};
        int samples_per_ch =
            mp3dec_decode_frame(&mp3d, inbuf.data(), static_cast<int>(inbuf_len), pcm, &info);

        if (info.frame_bytes == 0) {
            // Not enough data for a frame yet
            continue;
        }

        // Consume bytes used by the decoder
        if (info.frame_bytes > 0 && static_cast<size_t>(info.frame_bytes) <= inbuf_len) {
            memmove(inbuf.data(), inbuf.data() + info.frame_bytes, inbuf_len - info.frame_bytes);
            inbuf_len -= static_cast<size_t>(info.frame_bytes);
        }

        if (samples_per_ch <= 0 || info.channels <= 0 || info.hz <= 0) {
            continue;
        }

        // Reconfigure I2S clock if stream uses a different sample rate
        reconfigureI2sClock(info.hz);

        // minimp3 returns samples per channel; pcm contains interleaved samples
        const int channels = info.channels;
        const int total_samples = samples_per_ch * channels;

        const int16_t* out_ptr = pcm;
        std::vector<int16_t> mono_to_stereo;

        if (channels == 1) {
            mono_to_stereo.resize(samples_per_ch * 2);
            for (int i = 0; i < samples_per_ch; ++i) {
                mono_to_stereo[i * 2 + 0] = pcm[i];
                mono_to_stereo[i * 2 + 1] = pcm[i];
            }
            out_ptr = mono_to_stereo.data();
        }

        const size_t bytes_to_write =
            (channels == 1) ? static_cast<size_t>(samples_per_ch * 2) * sizeof(int16_t)
                            : static_cast<size_t>(total_samples) * sizeof(int16_t);

        size_t written = 0;
        esp_err_t ret =
            i2s_channel_write(mI2sTxHandle, out_ptr, bytes_to_write, &written, pdMS_TO_TICKS(200));
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "i2s_channel_write failed: %s", esp_err_to_name(ret));
        }
    }

    closeHttpStream();

    if (!mStopRequested.load()) {
        // Stream ended or error
        mIsPlaying.store(false);
    }

    ESP_LOGI(TAG, "Player loop exiting");
}

}  // namespace adapters
