#include <core/tick.hpp>
#include <vector>
//#include <string>
#include <fmt/core.h>

// SMA of n = (price_1 + price_2 + ... + price_n) / n

std::vector<double> smaCalc(int sma_interval, const std::vector<Tick>& ticker_data) { 
    std::vector<double> sma_points; // holds SMA points at each new corresponding price
    // reserve memory for efficiency/performance
    sma_points.reserve(ticker_data.size() - sma_interval + 1);

    // check for edge case
    // Avoid divide by zero edge case or an not calculable SMA
    if (sma_interval == 0) {
        fmt::print("Error: SMA interval of 0 not allowed!");
        return sma_points;
    }
    // Avoid calculatoin of indeterminate SMAs
    if (sma_interval > ticker_data.size()) {
        fmt::print("Error: SMA interval size larger than provided data size!");
        return sma_points;
    }

    // get the first SMA dataset to calculate from
    double rolling_sum = 0; // rolling sum
    for (size_t iterator = 0; iterator < sma_interval; iterator++) {
        rolling_sum += ticker_data[iterator].close;
    }
    sma_points.push_back(rolling_sum / sma_interval);

    // check if the SMA interval is equal to the dataset size and avoid out-of-bounds error:
    if (ticker_data.size() == sma_interval) {
        return sma_points;
    }
    
    // iteratively calculate the new SMA at every new data point
    for (size_t iterator = sma_interval; iterator < ticker_data.size(); iterator++) {
        // subtract the current "head" value
        rolling_sum -= ticker_data[iterator - sma_interval].close;
        // add the current "tail" value
        rolling_sum += ticker_data[iterator].close;
        // add the current new SMA value
        sma_points.push_back(rolling_sum / sma_interval);
    }
    return sma_points;
}

