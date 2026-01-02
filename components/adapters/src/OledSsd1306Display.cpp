#include "OledSsd1306Display.hpp"

#include "BoardConfig.hpp"
#include "esp_check.h"
#include "esp_log.h"

namespace adapters {

static const char *TAG = "OledSsd1306Display";

OledSsd1306Display::OledSsd1306Display() : mReady(false) {
  ESP_LOGI(TAG, "Crating OledSsd1306Display");
}

esp_err_t OledSsd1306Display::init() {
  const esp_err_t e1 = initI2c();
  if (e1 != ESP_OK) {
    ESP_LOGE(TAG, "I2C init failed: %s", esp_err_to_name(e1));
    return e1;
  }

  const esp_err_t e2 = ping();
  if (e2 != ESP_OK) {
    ESP_LOGE(TAG, "OLED not responding on I2C addr 0x%02X: %s",
             board::OLED_I2C_ADDR, esp_err_to_name(e2));
    return e2;
  }

  mReady = true;

  // Step 2 (later): send SSD1306 init command sequence and clear screen.
  ESP_LOGI(TAG, "OLED I2C OK (addr=0x%02X). Ready.", board::OLED_I2C_ADDR);
  return ESP_OK;
}

void OledSsd1306Display::showBoot() {
  if (!mReady) {
    ESP_LOGW(TAG, "ShowBoot called before Init()");
    return;
  }
  ESP_LOGI(TAG, "[OLED] Boot screen");
  // Step 2 (later): render Booting… to OLED.
}

void OledSsd1306Display::showStatus(const core::UiStatus &s) {
  if (!mReady) {
    ESP_LOGW(TAG, "ShowStatus called before Init()");
    return;
  }
  ESP_LOGI(TAG, "[OLED] Status kind=%u line1='%s' line2='%s'",
           static_cast<unsigned>(s.kind), s.line1.c_str(), s.line2.c_str());
  // Step 2 (later): render status lines.
}

void OledSsd1306Display::showStations(
    const std::vector<core::StationData> &stations, int selected) {
  if (!mReady) {
    ESP_LOGW(TAG, "ShowStations called before Init()");
    return;
  }

  ESP_LOGI(TAG, "[OLED] Stations (count=%u) selected=%d",
           static_cast<unsigned>(stations.size()), selected);

  // Step 2 (later): render list + highlight selected.
}

esp_err_t OledSsd1306Display::initI2c() {
  i2c_config_t conf{};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = static_cast<gpio_num_t>(board::I2C_SDA_GPIO);
  conf.scl_io_num = static_cast<gpio_num_t>(board::I2C_SCL_GPIO);
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = board::I2C_FREQ_HZ;

  ESP_RETURN_ON_ERROR(
      i2c_param_config(static_cast<i2c_port_t>(board::I2C_PORT), &conf), TAG,
      "i2c_param_config failed");
  // If driver already installed, this can fail; that’s OK if you handle it
  // centrally later.
  esp_err_t err = i2c_driver_install(static_cast<i2c_port_t>(board::I2C_PORT),
                                     conf.mode, 0, 0, 0);
  if (err == ESP_ERR_INVALID_STATE) {
    ESP_LOGW(TAG, "I2C driver already installed (port=%d). Continuing.",
             board::I2C_PORT);
    err = ESP_OK;
  }
  return err;
}

esp_err_t OledSsd1306Display::ping() const {
  // simplest "is device there?" check: try to start+stop a transaction
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (board::OLED_I2C_ADDR << 1) | I2C_MASTER_WRITE,
                        true);
  i2c_master_stop(cmd);

  const esp_err_t ret = i2c_master_cmd_begin(
      static_cast<i2c_port_t>(board::I2C_PORT), cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  return ret;
}

} // namespace adapters
