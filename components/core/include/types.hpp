#pragma once

#include <cstdint>
#include <stdint.h>
#include <string>

namespace core {

struct StationData {
  // TODO: std::string_view?
  std::string id;   // e.g., "radio1_aac_h"
  std::string name; // e.g., "Radio 1 (AAC High)"
  std::string url;  // streaming URL
};

struct AppModel {
  int selectedStationIndex;
  // TODO: Add other runtime state here (playback status, volume, etc.)
};

struct UiEvent {
  enum class Type { RENDER_STATIONS, RENDER_STATUS, RENDER_BOOT };

  Type type;
  int selectedIndex = 0; // Current selection for RENDER_STATIONS

  // TODO: union? variants? for other event data
};

// TODO: correct the status kinds
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

} // namespace core