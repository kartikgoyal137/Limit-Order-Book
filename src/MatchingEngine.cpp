#include "MatchingEngine.hpp"

void MatchingEngine::processMessage(const OrderMessage& msg) {
    switch (msg.type) {
        case OrderMessage::Type::Add:
            book_.addOrder(msg.orderId, msg.isBuy, msg.shares, msg.price);
            break;
        case OrderMessage::Type::Cancel:
            book_.cancelOrder(msg.orderId);
            break;
        case OrderMessage::Type::Modify:
            book_.modifyOrder(msg.orderId, msg.shares, msg.price);
            break;
    }
}

void MatchingEngine::run() {
    OrderMessage msg;
    while (running_.load(std::memory_order_acquire)) {
        while (inbox_.pop(msg))
            processMessage(msg);
    }
    while (inbox_.pop(msg))
        processMessage(msg);
}
