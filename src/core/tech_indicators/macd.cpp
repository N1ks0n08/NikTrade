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

    // Calculate MACD line and set it to the MACDResult object:
    // NOTE: THE MAX SIZE OF MACD IS LIMITED TO THE SIZE OF THE SLOW EMA
    // Therefore, I must offset the start indexes of the other vectors as well:
    // e.g. if data size = 100,
    // slowEMA period = 26,
    // fastEMA period = 12,
    //
    // (Using current_day = 1 for iteration) 
    //
    // For visual and conceptual purposes:
    // price starts on: current_day = 1
    // slowEMA starts on: current_day = 26 (first calculation happens af the 26th day)
    // fastEMA starts on: current_day = 12 (first calculation happens on the 12th day)
    //
    // THEREFORE:
    // price index STARTS @: current_day - 1 (minus 1 due to index starting at 0)
    // slowEMA index STARTS @: current_day - 26 (subtract 26 days since the slowEMA starts on day 26)
    // fastEMA index STARTS @: current_day - 12 (subtract 12 days since the fastEMA starts on day 12)

    // Using the ticker data's size to index correctly:
    // ticker_data.size() + 1 allows for me to reach "Day 122" (index value of 121 after subtracting 1)
    // START AT current_day = SLOW EMA PERIOD TO ALLOW FOR 
    // ALL VECTORS TO BE AVAILABLE FOR MACD calculation!
    // (subtract by 1 to convert from "current day" to index positioning)
    // e.g.: Given a max ticker size of 122:
    // goes from starting at 26 ( slowEMAPeriod) to 123 -> MACD size of: 123 - 26 = 97
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
    // e.g. (Assuming singal length of 9 periods):
    // SIgnal starts on the 9th INDEX OF MACD LINE
    // 9th index = position 8 (or macd[8])
    // Therefore, we must skip index range: [0, 7]
    // [0, 7] = first 8 elements of MACD line
    // Simple calculation to find the trim:
    // trim = singal_peroid - 1
    // trim = 9 - 1
    // trim = 8, which is correct.

    size_t trimCount = signal_period - 1;
    macd_result.macd.erase(macd_result.macd.begin(), macd_result.macd.begin() + trimCount);
    // Final size of MACD line and Signal line after being cut and appropriately matched:
    // MACD line size - trim = 97 - 8 = 89
    // Reserve and set the histogram vector into the MACDResult object:
    macd_result.histogram = histogramCalculator(macd_result.macd, macd_result.signal);

    // FOR UI PURPOSES:
    // THE FINAL DAY AT WHICH MACD, SIGNAL, and HISTOGRAM VALUES ARE ALL AVAILABLE:
    // slowEMA period of MACD calculation: 26
    // therefore 25 days after day 1 of reading data, MACD starts
    // however, signal period is of preoid 9
    // and as such signal period becomes available after 8 days
    // hence:
    // starting_day = current_day + (slowEMAPeriod - 1) + (signal_Period - 1)
    return macd_result;
}