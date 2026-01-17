#pragma once

#include <gmock/gmock.h>

#include "IPlayer.hpp"

namespace adapters {
class MockPlayer : public IPlayer {
   public:
    MOCK_METHOD(bool, init, (), (override));
    MOCK_METHOD(void, deinit, (), (override));
    MOCK_METHOD(bool, play, (const std::string&), (override));
    MOCK_METHOD(bool, stop, (), (override));
    MOCK_METHOD(void, setStatusCallback, (common::PlayerStatusCallback), (override));
};

}  // namespace adapters
