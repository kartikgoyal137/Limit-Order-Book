#include "OrderBook.hpp"

int32_t OrderBook::matchOrder(bool isBuy, int32_t shares, int32_t limitPrice) {
    AVLTree& contra       = isBuy ? asks_ : bids_;
    auto&    contraLevels = isBuy ? askLevels_ : bidLevels_;

    while (shares > 0 && !contra.empty()) {
        PriceLevel* best = isBuy ? contra.min() : contra.max();

        if (isBuy  && best->price > limitPrice) break;
        if (!isBuy && best->price < limitPrice) break;

        while (shares > 0 && best->head) {
            Order* order = best->head;
            if (order->shares <= shares) {
                shares -= order->shares;
                best->removeOrder(order);
                orders_.erase(order->id);
                orderPool_.deallocate(order);
            } else {
                order->shares      -= shares;
                best->totalVolume  -= shares;
                shares = 0;
            }
        }

        if (best->orderCount == 0) {
            contra.remove(best);
            contraLevels.erase(best->price);
            levelPool_.deallocate(best);
        }
    }
    return shares;
}

void OrderBook::removeLevelIfEmpty(PriceLevel* level, bool isBuy) {
    if (level->orderCount > 0) return;
    auto& tree   = isBuy ? bids_ : asks_;
    auto& levels = isBuy ? bidLevels_ : askLevels_;
    tree.remove(level);
    levels.erase(level->price);
    levelPool_.deallocate(level);
}

void OrderBook::addOrder(int32_t id, bool isBuy, int32_t shares, int32_t price) {
    shares = matchOrder(isBuy, shares, price);
    if (shares <= 0) return;

    auto& levels = isBuy ? bidLevels_ : askLevels_;
    auto& tree   = isBuy ? bids_ : asks_;

    PriceLevel** existing = levels.find_ptr(price);
    PriceLevel* level;
    if (!existing) {
        level = levelPool_.allocate();
        level->price = price;
        levels[price] = level;
        tree.insert(level);
    } else {
        level = *existing;
    }

    Order* order = orderPool_.allocate();
    order->id     = id;
    order->price  = price;
    order->shares = shares;
    order->isBuy  = isBuy;

    level->appendOrder(order);
    orders_[id] = order;
}

void OrderBook::cancelOrder(int32_t id) {
    Order** ptr = orders_.find_ptr(id);
    if (!ptr) return;

    Order* order       = *ptr;
    PriceLevel* level  = order->level;
    bool isBuy         = order->isBuy;

    level->removeOrder(order);
    orders_.erase(id);
    orderPool_.deallocate(order);

    removeLevelIfEmpty(level, isBuy);
}

void OrderBook::modifyOrder(int32_t id, int32_t newShares, int32_t newPrice) {
    Order** ptr = orders_.find_ptr(id);
    if (!ptr) return;

    Order* order      = *ptr;
    PriceLevel* level = order->level;
    bool isBuy        = order->isBuy;

    level->removeOrder(order);
    orders_.erase(id);
    orderPool_.deallocate(order);
    removeLevelIfEmpty(level, isBuy);

    addOrder(id, isBuy, newShares, newPrice);
}

Order* OrderBook::findOrder(int32_t id) const {
    Order** ptr = orders_.find_ptr(id);
    return ptr ? *ptr : nullptr;
}

PriceLevel* OrderBook::findLevel(int32_t price, bool isBuy) const {
    auto& levels = isBuy ? bidLevels_ : askLevels_;
    PriceLevel** ptr = levels.find_ptr(price);
    return ptr ? *ptr : nullptr;
}
