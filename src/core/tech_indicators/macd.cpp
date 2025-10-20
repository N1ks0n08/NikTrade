#include "macd.hpp"
#include "ema.hpp"
#include <fmt/core.h>

// MACD (Moving Average Convergence Divergence) 
// MACD line: fast EMA - slow EMA
// Signal Line: EMA of MACD vector (given the period for the signal line)
// Histogram: MACD line - Signal line

MACDResult defaultMacdResult() {
    return MACDResult{};
}

// Calculate the histogram vector
std::vector<double> histogramCalculator(std::vector<double>& macd_line, std::vector<double>& signal_line) {
    std::vector<double> histogram_vector;
    // Reserve the histogram vector for performance
    size_t hist_size = std::min(macd_line.size(), signal_line.size());
    histogram_vector.reserve(hist_size);

    // Account for the difference in periods:
    // e.g.:
    // smaller period vector = 5
    // bigger period vector = 10
    // smaller period vector position start index = bigger period vector - 1
    // bigger period vector position start index = 0
    // bigger size = smaller period, smaller size = bigger period

    for (size_t i = 0; i < hist_size; ++i) {
        // safer: always subtract corresponding signal value from macd value
        histogram_vector.push_back(macd_line[i] - signal_line[i]);
    }

    return histogram_vector;
}

MACDResult macdCalc(int fast_EMA_period, int slow_EMA_period, int signal_period, const std::vector<Tick>& ticker_data) {
    MACDResult macd_result;
    // check if any of the periods are zeroes
    if (fast_EMA_period == 0 || slow_EMA_period == 0 || signal_period == 0) {
        fmt::print("Error: Any EMA period cannot be equal to zero!\n");
        return defaultMacdResult();
    }
    // check if the short EMA has bigger period than long EMA
    if (fast_EMA_period > slow_EMA_period) {
        fmt::print("Error: Short EMA period cannot be larger than long EMA period!\n");
        return defaultMacdResult();
    }
    // check if dataset size is smaller than any of the periods
    if (fast_EMA_period > ticker_data.size() || slow_EMA_period > ticker_data.size() || signal_period > ticker_data.size()) {
        fmt::print("Error: EMA periods exceed dataset size!\n");
        return defaultMacdResult();
    }

    // get the fast EMA vector:
    std::vector<double> fastEMA = emaCalc(fast_EMA_period, ticker_data);
    // get the slow EMA vector:
    std::vector<double> slowEMA = emaCalc(slow_EMA_period, ticker_data);
    // Reserve MACD vector (slower EMA used due to the fitting size of MACD)
    macd_result.macd.reserve(slowEMA.size());

    // Calculate MACD line and set it to the MACDResult object:
    // get fast and slow EMA vector difference:
    size_t startIdx = slow_EMA_period - 1; // make sure to use the same starting point for the fast EMA as the slow EMA
    for (size_t iterator = 0; iterator < slowEMA.size(); ++iterator) {
        // safety check to prevent out-of-range access
        if (startIdx + iterator >= fastEMA.size()) break;
        // e.g.:
        // if fast EMA period = 12
        // and slow EMA period = 24
        // fast EMA starting point: slow EMA period - 1
        // slow EMA starting point: 0
        macd_result.macd.push_back(fastEMA[startIdx + iterator] - slowEMA[iterator]);
    }

    // set the Signal line into the MACDResult object, using the MACD line:
    if (!macd_result.macd.empty()) {
        macd_result.signal = emaCalc(signal_period, macd_result.macd);
    }

    // Reserve and set the histogram vector into the MACDResult object:
    macd_result.histogram = histogramCalculator(macd_result.macd, macd_result.signal);

    return macd_result;
}