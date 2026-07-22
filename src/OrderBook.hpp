#pragma once

#include "AVLTree.hpp"
#include "ObjectPool.hpp"
#include "Order.hpp"
#include "PriceLevel.hpp"
#include "FlatHashMap.hpp"

#include <cstdint>

class OrderBook {
    AVLTree bids_;
    AVLTree asks_;

    FlatHashMap<Order*>            orders_;
    FlatHashMap<PriceLevel*, 4096> bidLevels_;
    FlatHashMap<PriceLevel*, 4096> askLevels_;

    ObjectPool<Order>      orderPool_;
    ObjectPool<PriceLevel> levelPool_;

    int32_t matchOrder(bool isBuy, int32_t shares, int32_t limitPrice);
    void removeLevelIfEmpty(PriceLevel* level, bool isBuy);

public:
    OrderBook() = default;

    void addOrder(int32_t id, bool isBuy, int32_t shares, int32_t price);
    void cancelOrder(int32_t id);
    void modifyOrder(int32_t id, int32_t newShares, int32_t newPrice);

    PriceLevel* bestBid() const { return bids_.max(); }
    PriceLevel* bestAsk() const { return asks_.min(); }

    Order* findOrder(int32_t id) const;
    PriceLevel* findLevel(int32_t price, bool isBuy) const;
};
