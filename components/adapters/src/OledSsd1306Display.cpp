#include "OledSsd1306Display.hpp"

#include "esp_log.h"

namespace display {

static const char *TAG = "OledSsd1306Display";

OledSsd1306Display::OledSsd1306Display(OledSsd1306Config cfg) : cfg_(cfg) {}

esp_err_t OledSsd1306Display::Init() {
  const esp_err_t e1 = initI2c_();
  if (e1 != ESP_OK) {
    ESP_LOGE(TAG, "I2C init failed: %s", esp_err_to_name(e1));
    return e1;
  }

  const esp_err_t e2 = ping_();
  if (e2 != ESP_OK) {
    ESP_LOGE(TAG, "OLED not responding on I2C addr 0x%02X: %s", cfg_.addr,
             esp_err_to_name(e2));
    return e2;
  }

  ready_ = true;

  // Step 2 (later): send SSD1306 init command sequence and clear screen.
  ESP_LOGI(TAG, "OLED I2C OK (addr=0x%02X). Ready.", cfg_.addr);
  return ESP_OK;
}

void OledSsd1306Display::ShowBoot() {
  if (!ready_) {
    ESP_LOGW(TAG, "ShowBoot called before Init()");
    return;
  }
  ESP_LOGI(TAG, "[OLED] Boot screen");
  // Step 2 (later): render Booting… to OLED.
}

void OledSsd1306Display::ShowStatus(const UiStatus &s) {
  if (!ready_) {
    ESP_LOGW(TAG, "ShowStatus called before Init()");
    return;
  }
  ESP_LOGI(TAG, "[OLED] Status kind=%u line1='%s' line2='%s'",
           static_cast<unsigned>(s.kind), s.line1.c_str(), s.line2.c_str());
  // Step 2 (later): render status lines.
}

void OledSsd1306Display::ShowStations(const StationsModel &m, int selected) {
  if (!ready_) {
    ESP_LOGW(TAG, "ShowStations called before Init()");
    return;
  }

  ESP_LOGI(TAG, "[OLED] Stations (count=%u) selected=%d",
           static_cast<unsigned>(m.stations.size()), selected);

  // Step 2 (later): render list + highlight selected.
}

esp_err_t OledSsd1306Display::initI2c_() {
  i2c_config_t conf{};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = cfg_.sda;
  conf.scl_io_num = cfg_.scl;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = cfg_.clk_hz;

  ESP_RETURN_ON_ERROR(i2c_param_config(cfg_.port, &conf), TAG,
                      "i2c_param_config failed");
  // If driver already installed, this can fail; that’s OK if you handle it
  // centrally later.
  esp_err_t err = i2c_driver_install(cfg_.port, conf.mode, 0, 0, 0);
  if (err == ESP_ERR_INVALID_STATE) {
    ESP_LOGW(TAG, "I2C driver already installed (port=%d). Continuing.",
             static_cast<int>(cfg_.port));
    err = ESP_OK;
  }
  return err;
}

esp_err_t OledSsd1306Display::ping_() const {
  // simplest "is device there?" check: try to start+stop a transaction
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (cfg_.addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_stop(cmd);

  const esp_err_t ret = i2c_master_cmd_begin(cfg_.port, cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  return ret;
}

} // namespace display
