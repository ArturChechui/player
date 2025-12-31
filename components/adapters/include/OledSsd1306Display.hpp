#pragma once

#include "IDisplay.hpp"

#include <cstdint>

#include "driver/i2c.h"
#include "esp_err.h"

namespace display {

// TODO: remove and reuse BoardConfig
struct OledSsd1306Config {
  i2c_port_t port = I2C_NUM_0;
  gpio_num_t sda = GPIO_NUM_2; // your GP2
  gpio_num_t scl = GPIO_NUM_1; // your GP1
  uint32_t clk_hz = 400000;    // 400kHz usually OK
  uint8_t addr = 0x3C;         // common SSD1306 I2C address
};

class OledSsd1306Display final : public IDisplay {
public:
  explicit OledSsd1306Display(OledSsd1306Config cfg);

  // Call once during boot.
  esp_err_t Init();

  // IDisplay
  void ShowBoot() override;
  void ShowStatus(const UiStatus &s) override;
  void ShowStations(const StationsModel &m, int selected) override;

private:
  esp_err_t initI2c_();
  esp_err_t ping_() const;

  OledSsd1306Config cfg_;
  bool ready_{false};
};

} // namespace display
