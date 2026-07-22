#pragma once

#include "OrderBook.hpp"
#include "SPSCQueue.hpp"

#include <atomic>
#include <cstdint>

struct OrderMessage {
    enum class Type : uint8_t { Add, Cancel, Modify };
    Type    type;
    int32_t orderId;
    bool    isBuy;
    int32_t shares;
    int32_t price;
};

class MatchingEngine {
    OrderBook book_;
    SPSCQueue<OrderMessage, 65536> inbox_;
    alignas(CacheLineSize) std::atomic<bool> running_{true};

    void processMessage(const OrderMessage& msg);

public:
    bool submit(const OrderMessage& msg) { return inbox_.push(msg); }
    void stop() { running_.store(false, std::memory_order_release); }
    void run();

    const OrderBook& book() const { return book_; }
};
