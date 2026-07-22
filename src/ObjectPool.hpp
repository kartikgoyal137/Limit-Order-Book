#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <vector>

template<typename T, size_t BlockSize = 4096>
class ObjectPool {
    struct FreeNode { FreeNode* next; };
    static_assert(sizeof(T) >= sizeof(FreeNode), "T must be at least pointer-sized");

    std::vector<void*> blocks_;
    FreeNode* free_ = nullptr;

    void grow() {
        void* block = ::operator new(BlockSize * sizeof(T), std::align_val_t{alignof(T)});
        blocks_.push_back(block);

        auto* ptr = static_cast<char*>(block);
        for (size_t i = 0; i < BlockSize; ++i) {
            auto* node = reinterpret_cast<FreeNode*>(ptr + i * sizeof(T));
            node->next = free_;
            free_ = node;
        }
    }

public:
    ObjectPool() { grow(); }

    ~ObjectPool() {
        for (void* block : blocks_)
            ::operator delete(block, std::align_val_t{alignof(T)});
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    template<typename... Args>
    T* allocate(Args&&... args) {
        if (!free_) [[unlikely]] grow();
        void* ptr = free_;
        free_ = free_->next;
        return new (ptr) T{std::forward<Args>(args)...};
    }

    void deallocate(T* ptr) {
        ptr->~T();
        auto* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = free_;
        free_ = node;
    }
};
