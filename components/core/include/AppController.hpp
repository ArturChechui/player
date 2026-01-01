#pragma once

#include "AppContext.hpp"
#include "types.hpp"

namespace core {

class AppController {
public:
  explicit AppController(const AppContext &ctx);
  bool init();

private:
  AppModel mAppModel;
  const AppContext &mAppContext;
};

} // namespace core
