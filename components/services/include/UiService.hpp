#pragma once

#include "IDisplay.hpp"
#include "StationRepository.hpp"
#include "types.hpp"

namespace services {

class UiService {
public:
  explicit UiService(adapters::IDisplay &display,
                     StationRepository &stationRepo);
  bool init();
  void onEvent(const core::UiEvent &e);

private:
  adapters::IDisplay &mDisplay;
  StationRepository &mStationRepo;
};

} // namespace services
