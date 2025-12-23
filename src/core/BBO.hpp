#pragma once
#include <string>

// Normalized data-agnostic BBO (Best Bid and Offer) data structure
struct BBO {
    std::string symbol;
    double bid_price;
    double bid_quantity;
    double ask_price;
    double ask_quantity;
    std::string error;
};
