#pragma once

#include <gmock/gmock.h>

#include "IUiSink.hpp"
#include "UiTypes.hpp"

namespace core {
class MockUiTask : public IUiSink {
   public:
    MOCK_METHOD(void, post, (const common::UiEvent &), (override));
};

}  // namespace core
