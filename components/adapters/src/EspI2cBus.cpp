#include "EspI2cBus.hpp"

#include "BoardConfig.hpp"

// IDF
#include <driver/i2c_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>

namespace adapters {

static const char *TAG = "EspI2cBus";

EspI2cBus::EspI2cBus(const int &port)
    : mPort(port), mBusHandle(nullptr), mInitialized(false), mFreqHz(0U) {
    ESP_LOGI(TAG, "Creating EspI2cBus on port %d", mPort);
}

EspI2cBus::~EspI2cBus() {
    ESP_LOGI(TAG, "I2C cleanup: %zu device(s), bus %s", mDeviceHandles.size(),
             mBusHandle ? "valid" : "null");

    for (auto &[addr, devHandle] : mDeviceHandles) {
        if (devHandle) {
            i2c_master_bus_rm_device(devHandle);
        }
    }
    mDeviceHandles.clear();

    if (mBusHandle) {
        i2c_del_master_bus(mBusHandle);
        mBusHandle = nullptr;
    }
}

bool EspI2cBus::init() {
    if (mInitialized) {
        ESP_LOGW(TAG, "I2C bus already configured");
        return true;
    }

    i2c_master_bus_config_t busConfig = {};
    busConfig.i2c_port = static_cast<i2c_port_num_t>(mPort);
    busConfig.sda_io_num = static_cast<gpio_num_t>(common::I2C_SDA_GPIO);
    busConfig.scl_io_num = static_cast<gpio_num_t>(common::I2C_SCL_GPIO);
    busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    busConfig.flags.enable_internal_pullup = true;

    const esp_err_t ret = i2c_new_master_bus(&busConfig, &mBusHandle);

    if (ret == ESP_OK) {
        mInitialized = true;
        mFreqHz = common::I2C_FREQ_HZ;
        ESP_LOGI(TAG, "I2C master bus configured (SDA=%d, SCL=%d, freq=%lu Hz)",
                 common::I2C_SDA_GPIO, common::I2C_SCL_GPIO, mFreqHz);

        // Probe 0x3C for OLED
        esp_err_t probe_ret = i2c_master_probe(mBusHandle, 0x3C, pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "I2C probe 0x3C: %s", esp_err_to_name(probe_ret));
    } else {
        ESP_LOGE(TAG, "Failed to configure I2C master bus: %s", esp_err_to_name(ret));
    }

    return true;
}

bool EspI2cBus::writeBytes(const uint8_t &deviceAddr, const uint8_t *data, const size_t &len,
                           const uint32_t &timeoutMs) {
    if (!mBusHandle) {
        ESP_LOGE(TAG, "I2C bus not initialized");
        return false;
    }

    i2c_master_dev_handle_t devHandle = getOrCreateDeviceHandle(deviceAddr);
    if (!devHandle) {
        return false;
    }

    const esp_err_t ret = i2c_master_transmit(devHandle, data, len, pdMS_TO_TICKS(timeoutMs));
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "I2C write failed to 0x%02X: %s", deviceAddr, esp_err_to_name(ret));
    }

    return (ret == ESP_OK);
}

bool EspI2cBus::readBytes(const uint8_t &deviceAddr, uint8_t *data, const size_t &len,
                          const uint32_t &timeoutMs) {
    if (!mBusHandle) {
        ESP_LOGE(TAG, "I2C bus not initialized");
        return false;
    }

    i2c_master_dev_handle_t devHandle = getOrCreateDeviceHandle(deviceAddr);
    if (!devHandle) {
        return false;
    }

    const esp_err_t ret = i2c_master_receive(devHandle, data, len, pdMS_TO_TICKS(timeoutMs));
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "I2C read failed from 0x%02X: %s", deviceAddr, esp_err_to_name(ret));
    }

    return (ret == ESP_OK);
}

i2c_master_dev_handle_t EspI2cBus::getOrCreateDeviceHandle(const uint8_t &deviceAddr) {
    auto it = mDeviceHandles.find(deviceAddr);
    if (it != mDeviceHandles.end()) {
        return it->second;
    }

    i2c_device_config_t devConfig = {};
    devConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    devConfig.device_address = deviceAddr;
    devConfig.scl_speed_hz = mFreqHz;

    i2c_master_dev_handle_t devHandle = nullptr;
    esp_err_t ret = i2c_master_bus_add_device(mBusHandle, &devConfig, &devHandle);

    if (ret == ESP_OK) {
        mDeviceHandles[deviceAddr] = devHandle;
        ESP_LOGI(TAG, "Created device handle for 0x%02X", deviceAddr);
    } else {
        ESP_LOGE(TAG, "Failed to create device handle for 0x%02X: %s", deviceAddr,
                 esp_err_to_name(ret));
    }

    return devHandle;
}

}  // namespace adapters
