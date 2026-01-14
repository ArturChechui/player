#pragma once
#include <cstdint>

namespace common {
// ---- I2C (OLED + AHT20) ----
static constexpr int I2C_PORT = 0;      // I2C_NUM_0
static constexpr int I2C_SCL_GPIO = 1;  // GP1
static constexpr int I2C_SDA_GPIO = 2;  // GP2
static constexpr uint32_t I2C_FREQ_HZ = 400000;

// ---- OLED SSD1306 ----
static constexpr uint8_t OLED_I2C_ADDR = 0x3C;
static constexpr int OLED_WIDTH = 128;
static constexpr int OLED_HEIGHT = 64;

// TBD: Not used with I2C
static constexpr int OLED_RESET_GPIO = -1;

}  // namespace common
