#pragma once

#include "driver/i2c.h"
#include "esp_err.h"
#include <cstdint>
#include <vector>

namespace adapters {

class Ssd1306Controller {
public:
  static constexpr uint8_t WIDTH = 128;
  static constexpr uint8_t HEIGHT = 64;
  static constexpr uint8_t PAGES = HEIGHT / 8; // 8 pages

  Ssd1306Controller(i2c_port_t port, uint8_t addr);
  esp_err_t init(); // send init sequence, clear, display
  void clear();     // clear framebuffer
  void drawPixel(uint8_t x, uint8_t y, bool on);
  esp_err_t display(); // push framebuffer to panel

  std::vector<uint8_t> &buffer() { return mFb; }

private:
  esp_err_t sendInitSequence();
  esp_err_t writeCommand(const uint8_t *cmd, uint16_t len);
  esp_err_t writeData(const uint8_t *data, uint16_t len);

  i2c_port_t mPort;
  uint8_t mAddr;
  std::vector<uint8_t> mFb; // 1024 bytes
};

} // namespace adapters