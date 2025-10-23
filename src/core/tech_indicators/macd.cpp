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
    // Reserve MACD vector
    // slowEMA.size() because:
    // it appears the latest in MACD calculatoin (typically)
    macd_result.macd.reserve(slowEMA.size());

    /*
    Poorly documented/cohesive code:
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
    */

    // Calculate MACD line and set it to the MACDResult object:
    // NOTE: THE MAX SIZE OF MACD IS LIMITED TO THE SIZE OF THE SLOW EMA
    // Therefore, I must offset the start indexes of the other vectors as well:
    // e.g. if data size = 100,
    // slowEMA period = 40,
    // fastEMA period = 20,
    //
    // (Using current_day = 1 for iteration) 
    //
    // For visual and conceptual purposes:
    // price starts on: current_day = 1
    // slowEMA starts on: current_day = 40 (first calculation happens af the 40th day)
    // fastEMA starts on: current_day = 20 (first calculation happens on the 20th day)
    //
    // THEREFORE:
    // price index STARTS @: current_day - 1 (minus 1 due to index starting at 0)
    // slowEMA index STARTS @: current_day - 40 (subtract 40 days since the slowEMA starts on day 40)
    // fastEMA index STARTS @: current_day - 20 (subtract 20 days since the fastEMA starts on day 20)

    // Using the ticker data's size to index correctly:
    // ticker_data.size() + 1 allows for me to reach "Day 100" (index value of 99 after subtracting 1)
    // START AT current_day = SLOW EMA PERIOD TO ALLOW FOR 
    // ALL VECTORS TO BE AVAILABLE FOR MACD calculation!
    // (subtract by 1 to convert from "current day" to index positioning)
    for (size_t current_day = slow_EMA_period; current_day < ticker_data.size() + 1; current_day++) {
        // actual array index positoins for:
        // ticker_data: current_day - 1
        // fastEMA: current_day - fastEMAPeriod
        // slowEMA: current_day - slowEMAPeriod

        macd_result.macd.push_back(fastEMA[current_day - fast_EMA_period] - slowEMA[current_day - slow_EMA_period]);
    }

    // set the Signal line into the MACDResult object, using the MACD line:
    if (!macd_result.macd.empty()) {
        macd_result.signal = emaCalc(signal_period, macd_result.macd);
    }

    // DATA NORMALIZING FOR EQUAL LENGTH VECTORS
    // Cut up the macd to align with the starting index of signal vector:
    size_t trimCount = macd_result.macd.size() - macd_result.signal.size();
    macd_result.macd.erase(macd_result.macd.begin(), macd_result.macd.begin() + trimCount);

    // Reserve and set the histogram vector into the MACDResult object:
    macd_result.histogram = histogramCalculator(macd_result.macd, macd_result.signal);

    return macd_result;
}