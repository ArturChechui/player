#pragma once

typedef int esp_err_t;

#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_STATE -2
#define ESP_RETURN_ON_ERROR(x, tag, msg)                                       \
  if ((x) != ESP_OK)                                                           \
    return (x);
