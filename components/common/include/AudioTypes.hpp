#pragma once

#include <cstdint>
#include <functional>

namespace common {

enum class PlayerStatus : uint8_t { IDLE, BUFFERING, PLAYING, PAUSED, STOPPED, ERROR };

using PlayerStatusCallback = std::function<void(PlayerStatus)>;

}  // namespace common
