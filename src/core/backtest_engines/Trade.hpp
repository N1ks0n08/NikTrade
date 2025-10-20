#pragma once
#include <string>

struct Trade {
    std::string execution_date;
    std::string order_type;
    int shares; // NO FRACTIONAL SHARES AS OF NOW
    double strike_price;
    double unrealizedPnL;
    double realizedPnL;
    double totalEquityValue;
};