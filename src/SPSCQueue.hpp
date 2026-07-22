#pragma once

#include <atomic>
#include <array>
#include <cstddef>

inline constexpr size_t CacheLineSize = 64;

template<typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
    static constexpr size_t Mask = Capacity - 1;

    alignas(CacheLineSize) std::atomic<size_t> head_{0};
    alignas(CacheLineSize) std::atomic<size_t> tail_{0};
    alignas(CacheLineSize) std::array<T, Capacity> buffer_;

public:
    bool push(const T& item) {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        const size_t next = (tail + 1) & Mask;
        if (next == head_.load(std::memory_order_acquire))
            return false;
        buffer_[tail] = item;
        tail_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        const size_t head = head_.load(std::memory_order_relaxed);
        if (head == tail_.load(std::memory_order_acquire))
            return false;
        item = buffer_[head];
        head_.store((head + 1) & Mask, std::memory_order_release);
        return true;
    }
};
