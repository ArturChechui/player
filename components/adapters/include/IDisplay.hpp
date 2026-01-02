#pragma once

// TODO: forward declarations
#include "types.hpp"
#include <vector>

namespace adapters {

class IDisplay {
public:
  virtual ~IDisplay() = default;

  virtual void showBoot() = 0;
  virtual void showStatus(const core::UiStatus &s) = 0;
  virtual void showStations(const std::vector<core::StationData> &stations,
                            int selected) = 0;
};

} // namespace adapters
