#include "OledSsd1306Display.hpp"

#include <cstring>

#include "BoardConfig.hpp"
#include "II2cBus.hpp"

// IDF
#include <esp_log.h>

namespace adapters {
static const char *TAG = "OledSsd1306Display";

OledSsd1306Display::OledSsd1306Display(II2cBus &i2cBus)
    : mI2cBus(i2cBus), mI2cAddr(common::OLED_I2C_ADDR), mReady(false) {
    ESP_LOGI(TAG, "Creating OledSsd1306Display");
}

OledSsd1306Display::~OledSsd1306Display() {}

bool OledSsd1306Display::init() {
    sendInitSequence();

    mReady = true;
    ESP_LOGI(TAG, "OLED ready");

    return true;
}

void OledSsd1306Display::showFramebuffer(const uint8_t *framebuffer, const size_t &len) {
    if (!mReady) {
        ESP_LOGW(TAG, "Display not ready");
        return;
    }

    const uint8_t pageCmd[] = {0x22, 0x00, 0x07};
    const uint8_t colCmd[] = {0x21, 0x00, 0x7F};

    writeCommand(pageCmd, sizeof(pageCmd));
    writeCommand(colCmd, sizeof(colCmd));
    writeData(framebuffer, len);
}

void OledSsd1306Display::sendInitSequence() {
    const uint8_t initCmd[] = {0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D,
                               0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12, 0x81, 0x7F,
                               0xD9, 0xF1, 0xDB, 0x20, 0xA4, 0xA6, 0xAF};

    writeCommand(initCmd, sizeof(initCmd));
}

void OledSsd1306Display::writeCommand(const uint8_t *cmd, const uint16_t &len) {
    ESP_LOGI(TAG, "Writing command to OLED %d bytes", len);  // debug logs for testing
    std::vector<uint8_t> buf(len + 1U);
    buf[0] = 0x00;  // control byte: command
    std::memcpy(&buf[1], cmd, len);

    mI2cBus.writeBytes(mI2cAddr, buf.data(), buf.size());
}

void OledSsd1306Display::writeData(const uint8_t *data, const uint16_t &len) {
    ESP_LOGI(TAG, "Writing data to OLED %d bytes", len);  // debug logs for testing
    std::vector<uint8_t> buf(len + 1U);
    buf[0] = 0x40;  // control byte: data
    std::memcpy(&buf[1], data, len);

    mI2cBus.writeBytes(mI2cAddr, buf.data(), buf.size());
}

}  // namespace adapters
