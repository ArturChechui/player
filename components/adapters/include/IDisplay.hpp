#pragma once

#include "UiTypes.hpp"

namespace display {

class IDisplay {
public:
  virtual ~IDisplay() = default;

  virtual void ShowBoot() = 0;
  virtual void ShowStatus(const UiStatus &s) = 0;
  virtual void ShowStations(const StationsModel &m, int selected) = 0;
};

} // namespace display
