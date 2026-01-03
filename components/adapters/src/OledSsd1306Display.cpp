#include "OledSsd1306Display.hpp"

#include "BoardConfig.hpp"
#include "esp_check.h"
#include "esp_log.h"
#include <algorithm>
#include <cstring>

namespace adapters {

static const char *TAG = "OledSsd1306Display";

// Minimal 5x7 font (ASCII 32..127). Only a small subset shown; extend as
// needed.
static const uint8_t FONT5x7[96][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' '
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!'
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // '$'
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%'
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&'
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '''
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // '('
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ')'
                                    // ... (add the remaining glyphs as needed)
};

OledSsd1306Display::OledSsd1306Display() : mReady(false) {
  ESP_LOGI(TAG, "Creating OledSsd1306Display");
}

esp_err_t OledSsd1306Display::init() {
  // I2C is already set up via BoardConfig pins
  mCtrl = std::make_unique<Ssd1306Controller>(
      static_cast<i2c_port_t>(board::I2C_PORT), board::OLED_I2C_ADDR);

  // Ensure driver is installed
  i2c_config_t conf{};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = static_cast<gpio_num_t>(board::I2C_SDA_GPIO);
  conf.scl_io_num = static_cast<gpio_num_t>(board::I2C_SCL_GPIO);
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = board::I2C_FREQ_HZ;
  ESP_RETURN_ON_ERROR(
      i2c_param_config(static_cast<i2c_port_t>(board::I2C_PORT), &conf), TAG,
      "i2c_param_config");
  esp_err_t err = i2c_driver_install(static_cast<i2c_port_t>(board::I2C_PORT),
                                     conf.mode, 0, 0, 0);
  if (err == ESP_ERR_INVALID_STATE) {
    ESP_LOGW(TAG, "I2C driver already installed, continuing");
    err = ESP_OK;
  }
  ESP_RETURN_ON_ERROR(err, TAG, "i2c_driver_install");

  ESP_RETURN_ON_ERROR(mCtrl->init(), TAG, "SSD1306 init failed");

  mReady = true;
  ESP_LOGI(TAG, "OLED ready");
  return ESP_OK;
}

void OledSsd1306Display::clear() {
  if (mCtrl)
    mCtrl->clear();
}

void OledSsd1306Display::flush() {
  if (mCtrl)
    mCtrl->display();
}

void OledSsd1306Display::showBoot() {
  if (!mReady)
    return;
  clear();
  drawLine(0, "Booting...");
  flush();
}

void OledSsd1306Display::showStatus(const core::UiStatus &s) {
  if (!mReady)
    return;
  clear();
  drawLine(0, s.line1.c_str());
  drawLine(1, s.line2.c_str());
  flush();
}

void OledSsd1306Display::showStations(
    const std::vector<core::StationData> &stations, int selected) {
  if (!mReady)
    return;
  clear();

  // Up to 8 lines (64px / 8px per text line)
  const int maxLines = 8;
  for (int i = 0; i < static_cast<int>(stations.size()) && i < maxLines; ++i) {
    const bool inv = (i == selected);
    // Truncate to ~21 chars (128px / 6px char spacing)
    std::string name = stations[i].name;
    if (name.size() > 21)
      name.resize(21);
    drawLine(static_cast<uint8_t>(i), name.c_str(), inv);
  }

  flush();
}

// TODO: add explanations and how this glyph drawing works
// 00100
// 00100
// 00100
// 00000
// 00100
// it is !
// add more glyphs, maybe custom icons? (wifi/ play status etc)
void OledSsd1306Display::drawChar(uint8_t x, uint8_t y, char c, bool inverse) {
  if (c < 32 || c > 127)
    c = '?';
  const uint8_t *glyph = FONT5x7[c - 32];
  for (uint8_t col = 0; col < 5; ++col) {
    uint8_t bits = glyph[col];
    for (uint8_t row = 0; row < 7; ++row) {
      bool on = bits & (1u << row);
      if (inverse)
        on = !on;
      mCtrl->drawPixel(x + col, y + row, on);
    }
  }
  // one column spacing after glyph
  for (uint8_t row = 0; row < 7; ++row) {
    mCtrl->drawPixel(x + 5, y + row, inverse); // spacing respects inverse bg
  }
}

void OledSsd1306Display::drawText(uint8_t x, uint8_t y, const char *txt,
                                  bool inverse) {
  uint8_t cx = x;
  while (*txt && cx + 6 <= Ssd1306Controller::WIDTH) { // 5+1 spacing
    drawChar(cx, y, *txt, inverse);
    cx += 6;
    ++txt;
  }
}

void OledSsd1306Display::drawLine(uint8_t line, const char *txt, bool inverse) {
  // Each text line is 8px tall; place glyphs at y = line*8
  drawText(0, static_cast<uint8_t>(line * 8), txt, inverse);
}

} // namespace adapters