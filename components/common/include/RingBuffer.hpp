#pragma once

#include <cstdint>
#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace common {

/**
 * @brief Thread-safe ringbuffer for audio streaming
 *
 * Used to decouple HTTP reader (variable rate) from MP3 decoder (fixed rate).
 * - Writer: HTTP task pushes chunks
 * - Reader: MP3 decoder pops frames
 * - Safe for concurrent access from multiple tasks
 */
class RingBuffer {
   public:
    /**
     * @brief Create a ringbuffer of given size (bytes)
     * @param size Buffer capacity in bytes
     */
    explicit RingBuffer(size_t size)
        : mCapacity(size), mBuffer(new uint8_t[size]), mWritePos(0), mReadPos(0) {
        mMutex = xSemaphoreCreateMutex();
        mNotEmpty = xSemaphoreCreateBinary();
    }

    ~RingBuffer() {
        if (mMutex) {
            vSemaphoreDelete(mMutex);
        }
        if (mNotEmpty) {
            vSemaphoreDelete(mNotEmpty);
        }
        delete[] mBuffer;
    }

    /**
     * @brief Write data to ringbuffer (blocking if full)
     *
     * @param data Pointer to data to write
     * @param len Number of bytes to write
     * @return Number of bytes actually written
     *
     * @note If buffer is full, waits (with timeout) for space to free up
     */
    size_t write(const uint8_t* data, size_t len) {
        if (!data || len == 0 || !mMutex || !mNotEmpty) {
            return 0;
        }

        size_t bytesWritten = 0;

        while (bytesWritten < len) {
            if (xSemaphoreTake(mMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
                break;  // Timeout—stop writing
            }

            size_t available = getAvailableWrite();
            if (available == 0) {
                xSemaphoreGive(mMutex);
                vTaskDelay(pdMS_TO_TICKS(10));  // Wait for reader to consume
                continue;
            }

            size_t toWrite = (len - bytesWritten) > available ? available : (len - bytesWritten);
            writeInternal(data + bytesWritten, toWrite);
            bytesWritten += toWrite;

            xSemaphoreGive(mMutex);
            xSemaphoreGive(mNotEmpty);  // Signal that data is available
        }

        return bytesWritten;
    }

    /**
     * @brief Read data from ringbuffer (blocking if empty)
     *
     * @param data Pointer to buffer where data will be read into
     * @param len Maximum number of bytes to read
     * @param timeoutMs Timeout in milliseconds (0 = no wait, -1 = infinite)
     * @return Number of bytes actually read
     *
     * @note If buffer is empty, waits (with timeout) for data to arrive
     */
    size_t read(uint8_t* data, size_t len, int timeoutMs = 100) {
        if (!data || len == 0 || !mMutex || !mNotEmpty) {
            return 0;
        }

        // Wait for data to be available
        TickType_t timeout = (timeoutMs < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs);
        if (xSemaphoreTake(mNotEmpty, timeout) != pdTRUE) {
            return 0;  // Timeout—no data available
        }

        if (xSemaphoreTake(mMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
            return 0;  // Couldn't acquire lock
        }

        size_t available = getAvailableRead();
        size_t toRead = len > available ? available : len;
        readInternal(data, toRead);

        xSemaphoreGive(mMutex);

        return toRead;
    }

    /**
     * @brief Get number of bytes available to read (without waiting)
     */
    size_t available() const {
        if (xSemaphoreTake(mMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
            return 0;
        }
        size_t avail = getAvailableRead();
        xSemaphoreGive(mMutex);
        return avail;
    }

    /**
     * @brief Get number of bytes available to write (without waiting)
     */
    size_t space() const {
        if (xSemaphoreTake(mMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
            return 0;
        }
        size_t sp = getAvailableWrite();
        xSemaphoreGive(mMutex);
        return sp;
    }

    /**
     * @brief Clear buffer (reset read/write pointers)
     */
    void clear() {
        if (xSemaphoreTake(mMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            mReadPos = 0;
            mWritePos = 0;
            xSemaphoreGive(mMutex);
        }
    }

    /**
     * @brief Get total capacity
     */
    size_t capacity() const {
        return mCapacity;
    }

   private:
    size_t mCapacity;
    uint8_t* mBuffer;
    size_t mWritePos;
    size_t mReadPos;
    SemaphoreHandle_t mMutex;
    SemaphoreHandle_t mNotEmpty;

    size_t getAvailableRead() const {
        if (mWritePos >= mReadPos) {
            return mWritePos - mReadPos;
        } else {
            return mCapacity - mReadPos + mWritePos;
        }
    }

    size_t getAvailableWrite() const {
        return mCapacity - getAvailableRead() - 1;  // -1 to avoid full==empty ambiguity
    }

    void writeInternal(const uint8_t* data, size_t len) {
        if (mWritePos + len <= mCapacity) {
            std::memcpy(mBuffer + mWritePos, data, len);
            mWritePos += len;
        } else {
            size_t firstPart = mCapacity - mWritePos;
            std::memcpy(mBuffer + mWritePos, data, firstPart);
            std::memcpy(mBuffer, data + firstPart, len - firstPart);
            mWritePos = len - firstPart;
        }
    }

    void readInternal(uint8_t* data, size_t len) {
        if (mReadPos + len <= mCapacity) {
            std::memcpy(data, mBuffer + mReadPos, len);
            mReadPos += len;
        } else {
            size_t firstPart = mCapacity - mReadPos;
            std::memcpy(data, mBuffer + mReadPos, firstPart);
            std::memcpy(data + firstPart, mBuffer, len - firstPart);
            mReadPos = len - firstPart;
        }

        // Wrap around if at end
        if (mReadPos >= mCapacity) {
            mReadPos = 0;
        }
        if (mWritePos >= mCapacity) {
            mWritePos = 0;
        }
    }
};

}  // namespace common
