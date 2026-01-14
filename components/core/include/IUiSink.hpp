#pragma once

namespace common {
struct UiEvent;
}  // namespace common

namespace core {
class IUiSink {
   public:
    virtual ~IUiSink() = default;

    virtual void post(const common::UiEvent &e) = 0;
};

}  // namespace core
