#pragma once

#include "types.hpp"

namespace core {
class AppContext;
class AppController {
public:
  AppController(AppContext &ctx);
  bool init();

private:
  AppModel mAppModel;
  AppContext &mAppContext;
};

} // namespace core
