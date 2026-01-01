#include "UiService.hpp"

namespace services {

UiService::UiService(const adapters::IDisplay &display) {}

bool UiService::init() { return false; }

void UiService::render(const core::AppModel &model) {

  // TODO: do I even need to convert core::AppModel to adapters::StationsModel??
  // Or just use vector of stationData?
  mDisplay.showStations(/* TBD: convert model.stations to StationsModel */,
                        model.selectedStationIndex);
}

} // namespace services