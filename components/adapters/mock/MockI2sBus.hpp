#pragma once

#include <gmock/gmock.h>

#include "II2sBus.hpp"

namespace adapters {
class MockI2sBus : public II2sBus {
   public:
    MOCK_METHOD(bool, init, (), (override));
    MOCK_METHOD(void, deinit, (), (override));
    MOCK_METHOD(bool, write, (const uint8_t*, const size_t&, size_t&, const uint32_t&), (override));
    MOCK_METHOD(bool, isAvailable, (), (const, override));
};

}  // namespace adapters
