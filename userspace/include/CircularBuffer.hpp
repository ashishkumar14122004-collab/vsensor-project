/**
 * @file CircularBuffer.hpp
 * @brief Thread-safe fixed-capacity circular buffer.
 */

#pragma once

#include <vector>
#include <mutex>
#include <stdexcept>
#include <algorithm>

/**
 * @brief A thread-safe circular (ring) buffer with fixed capacity.
 *
 * When full, new elements overwrite the oldest. Safe for single-producer,
 * single-consumer scenarios via internal mutex.
 *
 * @tparam T Element type (must be default-constructible).
 */
template <typename T>
class CircularBuffer {
public:
    /**
     * @brief Construct a CircularBuffer with the given capacity.
     * @param capacity Maximum number of elements to store.
     */
    explicit CircularBuffer(std::size_t capacity)
        : buffer_(capacity), capacity_(capacity) {}

    /**
     * @brief Push a new element into the buffer.
     *
     * If the buffer is full, the oldest element is overwritten.
     */
    void push(const T& item) {
        std::lock_guard<std::mutex> lk(mtx_);
        buffer_[head_] = item;
        head_ = (head_ + 1) % capacity_;
        if (count_ < capacity_) {
            ++count_;
        } else {
            tail_ = (tail_ + 1) % capacity_; // overwrite: advance tail
        }
    }

    /**
     * @brief Return the most recently pushed element.
     * @throws std::underflow_error if buffer is empty.
     */
    T latest() const {
        std::lock_guard<std::mutex> lk(mtx_);
        if (count_ == 0)
            throw std::underflow_error("CircularBuffer is empty");
        std::size_t idx = (head_ + capacity_ - 1) % capacity_;
        return buffer_[idx];
    }

    /**
     * @brief Return the last @p n elements in chronological order.
     * @param n Number of elements to retrieve (clamped to current size).
     */
    std::vector<T> range(std::size_t n) const {
        std::lock_guard<std::mutex> lk(mtx_);
        n = std::min(n, count_);
        std::vector<T> result;
        result.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            std::size_t idx = (tail_ + count_ - n + i) % capacity_;
            result.push_back(buffer_[idx]);
        }
        return result;
    }

    /** @brief Return the number of elements currently stored. */
    std::size_t size() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return count_;
    }

    /** @brief Return true if the buffer holds no elements. */
    bool empty() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return count_ == 0;
    }

    /** @brief Return true if the buffer is at capacity. */
    bool full() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return count_ == capacity_;
    }

    /** @brief Clear all elements. */
    void clear() {
        std::lock_guard<std::mutex> lk(mtx_);
        head_  = 0;
        tail_  = 0;
        count_ = 0;
    }

private:
    mutable std::mutex  mtx_;
    std::vector<T>      buffer_;
    std::size_t         capacity_;
    std::size_t         head_  = 0; ///< Next write index
    std::size_t         tail_  = 0; ///< Oldest element index
    std::size_t         count_ = 0; ///< Number of valid elements
};
