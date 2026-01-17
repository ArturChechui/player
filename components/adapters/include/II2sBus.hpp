#pragma once

#include <cstddef>
#include <cstdint>

namespace adapters {

class II2sBus {
   public:
    virtual ~II2sBus() = default;

    virtual bool init() = 0;
    virtual void deinit() = 0;
    virtual bool write(const uint8_t* data, const size_t& size, size_t& bytesWritten,
                       const uint32_t& timeoutMs) = 0;
    virtual bool isAvailable() const = 0;
};

}  // namespace adapters
