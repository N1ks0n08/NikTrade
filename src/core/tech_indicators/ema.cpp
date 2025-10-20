#include "ema.hpp"
#include <fmt/core.h>

// EMA_t = a * P_t + (1 - a) * EMA_t-1
// where:
// EMA_t = current EMA
// P_t = current price (or current vector index value)
// a = smoothing factor (2 / (N + 1))
// N = lookback perod
// EMA_t-1 = previous EMA

// function that applies the EMA formula and pushes the current iteration's calculation to the EMA array
void ema_point_adder(std::vector<double>& ema_pionts, const std::vector<Tick>& ticker_data, float& smoothing_factor, size_t iteration) {
    ema_pionts.push_back(
        (smoothing_factor * ticker_data[iteration].close) 
        + 
        (1 - smoothing_factor) * (ema_pionts.back())
    );
}

// GENERALIZED FOR OTHER CALCULATIONS
// function that applies the EMA formula and pushes the current iteration's calculation to the EMA array
void ema_point_adder(std::vector<double>& ema_pionts, const std::vector<double>& initial_vector, float& smoothing_factor, size_t iteration) {
    ema_pionts.push_back(
        (smoothing_factor * initial_vector[iteration]) 
        + 
        (1 - smoothing_factor) * (ema_pionts.back())
    );
}

std::vector<double> emaCalc(int ema_interval, const std::vector<Tick>& ticker_data) {
    std::vector<double> ema_points; // holds EMA points at each new corresponding price
    // chcek for edge cases
    if (ema_interval == 0) {
        fmt::print("Error: EMA interval of 0 not allowed!");
        return ema_points;
    }
    if (ema_interval > ticker_data.size()) {
        fmt::print("Error: EMA interval is greater than data size!");
        return ema_points;
    }
    // reserve memory
    ema_points.reserve(ticker_data.size() - ema_interval + 1);

    float smoothing_factor = float(2) / (float(ema_interval) + 1);

    // very first point of an EMA is the SMA of hte first interval
    ema_points.push_back(0);
    for (size_t iterator = 0; iterator < ema_interval; iterator++) {
        ema_points[0] += ticker_data[iterator].close;
    }
    ema_points[0] = ema_points[0] / ema_interval;

    // chcck if the ema interval matches the current data size
    if (ticker_data.size() == ema_interval) {
        return ema_points;
    }
    // call a funciton that adds to the EMA array
    // make sure to start at the index after which the first EMA point was calculated
    for (size_t iterator = ema_interval; iterator < ticker_data.size(); iterator++) {
        ema_point_adder(ema_points, ticker_data, smoothing_factor, iterator);
    }

    return ema_points;
}

// GENERALIZED FOR OTHER CALCULATIONS
std::vector<double> emaCalc(int ema_interval, const std::vector<double>& initial_vector) {
    std::vector<double> ema_points; // holds EMA points at each new corresponding price
    // chcek for edge cases
    if (ema_interval == 0) {
        fmt::print("Error: EMA interval of 0 not allowed!");
        return ema_points;
    }
    if (ema_interval > initial_vector.size()) {
        fmt::print("Error: EMA interval is greater than data size!");
        return ema_points;
    }
    // reserve memory
    ema_points.reserve(initial_vector.size() - ema_interval + 1);

    float smoothing_factor = float(2) / (float(ema_interval) + 1);

    // very first point of an EMA is the SMA of hte first interval
    ema_points.push_back(0);
    for (size_t iterator = 0; iterator < ema_interval; iterator++) {
        ema_points[0] += initial_vector[iterator];
    }
    ema_points[0] = ema_points[0] / ema_interval;

    // chcck if the ema interval matches the current data size
    if (initial_vector.size() == ema_interval) {
        return ema_points;
    }
    // call a funciton that adds to the EMA array
    // make sure to start at the index after which the first EMA point was calculated
    for (size_t iterator = ema_interval; iterator < initial_vector.size(); iterator++) {
        ema_point_adder(ema_points, initial_vector, smoothing_factor, iterator);
    }

    return ema_points;
}