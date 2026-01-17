#include "EspI2sBus.hpp"

#include "BoardConfig.hpp"

// IDF
#include <esp_log.h>

namespace adapters {

static constexpr const char* TAG = "EspI2sBus";

EspI2sBus::EspI2sBus(const int& port) : mPort(port), mTxHandle(nullptr) {
    ESP_LOGI(TAG, "Creating I2S bus for port %d", mPort);
}

EspI2sBus::~EspI2sBus() {
    deinit();
}

bool EspI2sBus::init() {
    if (mTxHandle != nullptr) {
        ESP_LOGW(TAG, "I2S already initialized");
        return true;
    }

    // Standard TX channel config
    i2s_chan_config_t chanCfg =
        I2S_CHANNEL_DEFAULT_CONFIG(static_cast<i2s_port_t>(mPort), I2S_ROLE_MASTER);

    esp_err_t ret = i2s_new_channel(&chanCfg, &mTxHandle, nullptr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S TX channel: %s", esp_err_to_name(ret));
        return false;
    }

    // Standard mode config (Philips I2S format)
    i2s_std_config_t stdCfg = {
        .clk_cfg =
            {
                .sample_rate_hz = common::I2S_SAMPLE_RATE,
                .clk_src = I2S_CLK_SRC_DEFAULT,
                .mclk_multiple = I2S_MCLK_MULTIPLE_256,
            },
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            static_cast<i2s_data_bit_width_t>(common::I2S_BITS_PER_SAMPLE), I2S_SLOT_MODE_STEREO),
        .gpio_cfg =
            {
                .mclk = static_cast<gpio_num_t>(common::I2S_MCLK_GPIO),
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

    ret = i2s_channel_init_std_mode(mTxHandle, &stdCfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S standard mode: %s", esp_err_to_name(ret));
        i2s_del_channel(mTxHandle);
        mTxHandle = nullptr;
        return false;
    }

    ret = i2s_channel_enable(mTxHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S channel: %s", esp_err_to_name(ret));
        i2s_del_channel(mTxHandle);
        mTxHandle = nullptr;
        return false;
    }

    // ESP_LOGI(TAG, "I2S initialized (port=%d, rate=%u Hz, bits=%u, BCLK=%d, LRCK=%d, DOUT=%d)",
    //          mPort, common::I2S_SAMPLE_RATE, common::I2S_BITS_PER_SAMPLE, common::I2S_BCLK_GPIO,
    //          common::I2S_LRCK_GPIO, common::I2S_DOUT_GPIO);

    return true;
}

void EspI2sBus::deinit() {
    if (mTxHandle == nullptr) {
        return;
    }

    ESP_LOGI(TAG, "Deinitializing I2S bus");
    i2s_channel_disable(mTxHandle);
    i2s_del_channel(mTxHandle);
    mTxHandle = nullptr;
}

bool EspI2sBus::write(const uint8_t* data, const size_t& size, size_t& bytesWritten,
                      const uint32_t& timeoutMs) {
    if (mTxHandle == nullptr) {
        ESP_LOGE(TAG, "I2S not initialized");
        bytesWritten = 0;
        return false;
    }

    esp_err_t ret = i2s_channel_write(mTxHandle, data, size, &bytesWritten, timeoutMs);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "I2S write failed: %s (wrote %zu/%zu bytes)", esp_err_to_name(ret),
                 bytesWritten, size);
        return false;
    }

    return true;
}

bool EspI2sBus::isAvailable() const {
    return (mTxHandle != nullptr);
}

}  // namespace adapters
