#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace core {

struct StationData {
  std::string name;
  std::string url;
};

struct AppModel {
  std::vector<StationData> stations;
  int selectedStationIndex;
};

enum class UiEventType : uint8_t { UiInvalidate, Render };

struct UiEvent {
  /* @brief copy of the app model at event time for the correctconcurrency
   * handling*/
  AppModel copyAppModel;

  /* @brief type of the event */
  UiEventType type;
};

} // namespace core