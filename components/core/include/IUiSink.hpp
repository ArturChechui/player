#pragma once

#include "types.hpp"

namespace core {

class IUiSink {
public:
  virtual ~IUiSink() = default;

  virtual void post(const UiEvent &e) = 0;
};

} // namespace core
