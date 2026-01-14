#pragma once

#include <gmock/gmock.h>

namespace common {
struct UiEvent;
}  // namespace common

namespace services {
class MockUiService {
   public:
    MOCK_METHOD(bool, init, ());
    MOCK_METHOD(void, onEvent, (const common::UiEvent &));
};

}  // namespace services
