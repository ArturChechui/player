#pragma once

#include "UiTypes.hpp"

namespace adapters {

class IDisplay {
public:
  virtual ~IDisplay() = default;

  virtual void showBoot() = 0;
  virtual void showStatus(const UiStatus &s) = 0;
  virtual void showStations(const StationsModel &m, int selected) = 0;
};

} // namespace adapters
