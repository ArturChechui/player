#pragma once

#include <gmock/gmock.h>

#include "IDisplay.hpp"

namespace adapters {
class MockDisplay : public IDisplay {
   public:
    MOCK_METHOD(bool, init, (), (override));
    MOCK_METHOD(void, showFramebuffer, (const uint8_t*, const size_t&), (override));
};

}  // namespace adapters
