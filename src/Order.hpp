#pragma once

#include <cstdint>

struct PriceLevel;

struct Order {
    int32_t id       = 0;
    int32_t price    = 0;
    int32_t shares   = 0;
    bool    isBuy    = false;

    Order*      next  = nullptr;
    Order*      prev  = nullptr;
    PriceLevel* level = nullptr;
};
