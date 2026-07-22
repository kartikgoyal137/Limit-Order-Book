#pragma once

#include <cstdint>
#include <cstddef>

template<typename V, size_t Capacity = 65536>
class FlatHashMap {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
    static constexpr int32_t Empty = INT32_MIN;
    static constexpr size_t  Mask  = Capacity - 1;

    struct Slot {
        int32_t key = Empty;
        V       value{};
    };

    Slot   slots_[Capacity];
    size_t size_ = 0;

public:
    V& operator[](int32_t key) {
        size_t idx = static_cast<size_t>(key) & Mask;
        while (true) {
            if (slots_[idx].key == key)
                return slots_[idx].value;
            if (slots_[idx].key == Empty) {
                slots_[idx].key = key;
                ++size_;
                return slots_[idx].value;
            }
            idx = (idx + 1) & Mask;
        }
    }

    V* find_ptr(int32_t key) const {
        size_t idx = static_cast<size_t>(key) & Mask;
        while (true) {
            if (slots_[idx].key == key)
                return const_cast<V*>(&slots_[idx].value);
            if (slots_[idx].key == Empty)
                return nullptr;
            idx = (idx + 1) & Mask;
        }
    }

    bool erase(int32_t key) {
        size_t idx = static_cast<size_t>(key) & Mask;
        while (true) {
            if (slots_[idx].key == Empty)
                return false;
            if (slots_[idx].key == key)
                break;
            idx = (idx + 1) & Mask;
        }

        --size_;

        size_t cur = idx;
        while (true) {
            size_t next = (cur + 1) & Mask;
            if (slots_[next].key == Empty)
                break;
            size_t natural = static_cast<size_t>(slots_[next].key) & Mask;
            if (((next - natural) & Mask) == 0)
                break;
            slots_[cur] = slots_[next];
            cur = next;
        }
        slots_[cur].key   = Empty;
        slots_[cur].value = V{};
        return true;
    }

    size_t size() const { return size_; }
};
