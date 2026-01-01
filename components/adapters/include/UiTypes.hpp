#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace adapters {

enum class UiStatusKind : uint8_t {
  Booting,
  WifiConnecting,
  WifiConnected,
  WifiError,
  Playing,
  Stopped,
  Error
};

struct UiStatus {
  UiStatusKind kind{UiStatusKind::Booting};
  std::string line1;
  std::string line2;
};

struct StationsModel {
  std::vector<std::string> stations;
};

} // namespace adapters
