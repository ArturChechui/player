#pragma once

#include "IDisplay.hpp"

#include <cstdint>

#include "driver/i2c.h"
#include "esp_err.h"

namespace adapters {

class OledSsd1306Display final : public IDisplay {
public:
  explicit OledSsd1306Display();
  esp_err_t init();

  // IDisplay
  void showBoot() override;
  void showStatus(const core::UiStatus &s) override;
  void showStations(const std::vector<core::StationData> &stations,
                    int selected) override;

private:
  esp_err_t initI2c();
  esp_err_t ping() const;

  bool mReady;
};

} // namespace adapters
