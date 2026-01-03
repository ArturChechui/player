#include "Ssd1306Controller.hpp"
#include "esp_check.h"
#include "esp_log.h"
#include <algorithm>

namespace adapters {

static const char *TAG = "Ssd1306Ctrl";

Ssd1306Controller::Ssd1306Controller(i2c_port_t port, uint8_t addr)
    : mPort(port), mAddr(addr), mFb(WIDTH * PAGES, 0) {}

esp_err_t Ssd1306Controller::init() {
  ESP_RETURN_ON_ERROR(sendInitSequence(), TAG, "init seq failed");
  clear();
  return display();
}

esp_err_t Ssd1306Controller::sendInitSequence() {
  // Minimal, common SSD1306 128x64 init sequence
  const uint8_t init[] = {
      0xAE,       // display off
      0xD5, 0x80, // clock divide
      0xA8, 0x3F, // multiplex 1/64
      0xD3, 0x00, // display offset
      0x40,       // start line
      0x8D, 0x14, // charge pump on
      0x20, 0x00, // memory mode: horizontal
      0xA1,       // segment remap
      0xC8,       // COM scan dec
      0xDA, 0x12, // COM pins
      0x81, 0x7F, // contrast
      0xD9, 0xF1, // pre-charge
      0xDB, 0x20, // VCOM detect
      0xA4,       // display from RAM
      0xA6,       // normal display
      0xAF        // display on
  };
  return writeCommand(init, sizeof(init));
}

esp_err_t Ssd1306Controller::writeCommand(const uint8_t *cmd, uint16_t len) {
  i2c_cmd_handle_t c = i2c_cmd_link_create();
  i2c_master_start(c);
  i2c_master_write_byte(c, (mAddr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(c, 0x00, true); // command control byte
  i2c_master_write(c, const_cast<uint8_t *>(cmd), len, true);
  i2c_master_stop(c);
  esp_err_t ret = i2c_master_cmd_begin(mPort, c, pdMS_TO_TICKS(100));
  i2c_cmd_link_delete(c);
  return ret;
}

esp_err_t Ssd1306Controller::writeData(const uint8_t *data, uint16_t len) {
  i2c_cmd_handle_t c = i2c_cmd_link_create();
  i2c_master_start(c);
  i2c_master_write_byte(c, (mAddr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(c, 0x40, true); // data control byte
  i2c_master_write(c, const_cast<uint8_t *>(data), len, true);
  i2c_master_stop(c);
  esp_err_t ret = i2c_master_cmd_begin(mPort, c, pdMS_TO_TICKS(100));
  i2c_cmd_link_delete(c);
  return ret;
}

void Ssd1306Controller::clear() { std::fill(mFb.begin(), mFb.end(), 0x00); }

void Ssd1306Controller::drawPixel(uint8_t x, uint8_t y, bool on) {
  if (x >= WIDTH || y >= HEIGHT)
    return;
  const uint16_t idx = (y / 8) * WIDTH + x;
  const uint8_t bit = y & 0x07;
  if (on)
    mFb[idx] |= (1u << bit);
  else
    mFb[idx] &= ~(1u << bit);
}

esp_err_t Ssd1306Controller::display() {
  const uint8_t pageCmd[] = {0x22, 0x00, 0x07};
  const uint8_t colCmd[] = {0x21, 0x00, 0x7F};
  ESP_RETURN_ON_ERROR(writeCommand(pageCmd, sizeof(pageCmd)), TAG, "page cmd");
  ESP_RETURN_ON_ERROR(writeCommand(colCmd, sizeof(colCmd)), TAG, "col cmd");
  return writeData(mFb.data(), mFb.size());
}

} // namespace adapters