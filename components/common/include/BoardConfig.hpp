#pragma once

#include <cstdint>

namespace common {

// I2C Configuration
static constexpr int I2C_PORT = 0;      // I2C_NUM_0
static constexpr int I2C_SCL_GPIO = 1;  // GP1
static constexpr int I2C_SDA_GPIO = 2;  // GP2
static constexpr uint32_t I2C_FREQ_HZ = 400000;
static constexpr uint8_t OLED_I2C_ADDR = 0x3C;
static constexpr int OLED_WIDTH = 128;
static constexpr int OLED_HEIGHT = 64;

// I2S Configuration (audio output)
static constexpr int I2S_PORT = 0;
static constexpr uint8_t I2S_BCLK_GPIO = 26;  // Bit clock
static constexpr uint8_t I2S_LRCK_GPIO = 25;  // Word select (left/right)
static constexpr uint8_t I2S_DOUT_GPIO = 22;  // Data out to speaker/codec
static constexpr uint8_t I2S_MCLK_GPIO = 0;   // Master clock (set to 0 if unused)
static constexpr uint32_t I2S_SAMPLE_RATE = 44100;
static constexpr uint8_t I2S_BITS_PER_SAMPLE = 16;

}  // namespace common
