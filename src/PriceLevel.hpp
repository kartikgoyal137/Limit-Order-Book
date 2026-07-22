#pragma once

#include <cstdint>
#include "Order.hpp"

struct PriceLevel {
    int32_t price       = 0;
    int32_t orderCount  = 0;
    int32_t totalVolume = 0;
    int32_t height      = 1;

    PriceLevel* left  = nullptr;
    PriceLevel* right = nullptr;
    Order*      head  = nullptr;
    Order*      tail  = nullptr;

    void appendOrder(Order* order) {
        order->prev  = tail;
        order->next  = nullptr;
        order->level = this;

        if (tail) tail->next = order;
        else      head = order;
        tail = order;

        orderCount++;
        totalVolume += order->shares;
    }

    void removeOrder(Order* order) {
        if (order->prev) order->prev->next = order->next;
        else             head = order->next;

        if (order->next) order->next->prev = order->prev;
        else             tail = order->prev;

        totalVolume -= order->shares;
        orderCount--;

        order->prev = order->next = nullptr;
        order->level = nullptr;
    }
};
