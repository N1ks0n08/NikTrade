#pragma once
#include <cstdint>

struct KlineData {
    uint64_t open_time;
    double open;
    double high;
    double low;
    double close;
    double volume;
    uint64_t close_time;
};