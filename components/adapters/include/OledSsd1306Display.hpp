#pragma once

#include "IDisplay.hpp"
#include "Ssd1306Controller.hpp"
#include <memory>

namespace adapters {

class OledSsd1306Display final : public IDisplay {
public:
  OledSsd1306Display();
  esp_err_t init();

  // IDisplay
  void showBoot() override;
  void showStatus(const core::UiStatus &s) override;
  void showStations(const std::vector<core::StationData> &stations,
                    int selected) override;

private:
  void clear();
  void flush();
  void drawChar(uint8_t x, uint8_t y, char c, bool inverse = false);
  void drawText(uint8_t x, uint8_t y, const char *txt, bool inverse = false);
  void drawLine(uint8_t line, const char *txt, bool inverse = false);

  std::unique_ptr<Ssd1306Controller> mCtrl;
  bool mReady;
};

} // namespace adapters