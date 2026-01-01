#pragma once

#include "adapters/include/IDisplay.hpp"
#include "core/include/types.hpp"

namespace services {

class UiService {
public:
  explicit UiService(const adapters::IDisplay &display);
  bool init();
  void render(const core::AppModel &model);

private:
  const adapters::IDisplay &mDisplay;
};

} // namespace services
