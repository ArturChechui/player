#pragma once

#include <string>

#include "AudioTypes.hpp"

namespace adapters {
class IPlayer {
   public:
    virtual ~IPlayer() = default;

    virtual bool init() = 0;
    virtual void deinit() = 0;
    virtual bool play(const std::string& url) = 0;
    virtual bool stop() = 0;
    virtual void setStatusCallback(common::PlayerStatusCallback cb) = 0;
};

}  // namespace adapters
