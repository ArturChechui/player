#pragma once

#include "types.hpp"
#include <vector>

namespace services {

class StationRepository {
public:
  explicit StationRepository();
  /*  @brief Initializes the repository. Loads stations from LittleFS (or
   * hardcoded for now). Call once during app startup */
  bool init();

  /*  @brief Get immutable reference to stations Thread-safe because stations
   * never change after init*/
  const std::vector<core::StationData> &getStations() const;

private:
  std::vector<core::StationData> mStations;
  bool mInitialized;
};

} // namespace services
