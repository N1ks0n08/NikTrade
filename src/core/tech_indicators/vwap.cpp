#include "vwap.hpp"
#include <fmt/core.h>

// VWAP (Volume-Weighted Average Price)
//           sum of price_i * volume_i from i to n
//  VWAP =  ---------------------------------------
//               sum of volume_i from i to n
std::vector<double> vwapCalc (const std::vector<Tick>& ticker_data) {
    std::vector<double> vwap_points;
    double total_volume = 0.0;

    // check for edge cases
    if (ticker_data.size() == 0) {
        fmt::print("Error: No ticker data found!");
        return vwap_points;
    }
    // get the very first volume value checked
    if (ticker_data[0].volume == 0) {
        fmt::print("Error: Volume sum for first iteration is zero; leads to divide by zero error!");
        return vwap_points;
    }

    // reserve the vwap vector size for performance
    vwap_points.reserve(ticker_data.size());
    double volume_weighted_price_sum = 0;

    // append the VWAP at each index of ticker_data vector
    for (size_t iterator = 0; iterator < ticker_data.size(); iterator++) {
        // get total volume up to the n iteration
        total_volume += ticker_data[iterator].volume;
        volume_weighted_price_sum += ticker_data[iterator].close * ticker_data[iterator].volume;
        vwap_points.push_back(volume_weighted_price_sum / total_volume);
    }

    return vwap_points;
}