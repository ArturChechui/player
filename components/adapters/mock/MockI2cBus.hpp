#pragma once

#include <gmock/gmock.h>

#include "II2cBus.hpp"

namespace adapters {
class MockI2cBus : public II2cBus {
   public:
    MOCK_METHOD(bool, init, (), (override));
    MOCK_METHOD(bool, writeBytes,
                (const uint8_t& deviceAddr, const uint8_t* data, const size_t& len,
                 const uint32_t& timeoutMs),
                (override));
    MOCK_METHOD(bool, readBytes,
                (const uint8_t& deviceAddr, uint8_t* data, const size_t& len,
                 const uint32_t& timeoutMs),
                (override));
};

}  // namespace adapters
