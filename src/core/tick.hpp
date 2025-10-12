#pragma once
#include <string>

// create a stock ticker data type
struct Tick {
    std::string date;
    double open, high, low, close;
    int volume;
};