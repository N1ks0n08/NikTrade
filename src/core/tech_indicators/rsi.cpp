#include "rsi.hpp"
#include <fmt/core.h>

// RSI = Relative Strength Index
// RSI_t = 100 - ( 100 / (1 + RS_t))
//
// where:  Average Gain over N periods
// RS_t =  ---------------------------
//         Average Loss over N periods
//
// Average Gain_Initial = # of gains / N
// Average Loss_Initial = # of losses / N
//
// Average Gain_t = ( (Average Gain_t-1) * (N - 1) + Gain_t ) / N
// Average Loss_t = ( (Average Loss_t-1) * (N - 1) + Gain_t ) / N
// 
// number of gains + number of losses <= N - 1 (since these are based on differences)

// push back to the rsi vector
void rsi_point_adder (std::vector<double>& rsi_points, double& average_gains, double& average_losses) {
    // add the new calculated data point
    rsi_points.push_back(
        100 - (100 / (1 + (average_gains / average_losses)))
    );
}

std::vector<double> rsiCalc (int rsi_interval, const std::vector<Tick>& ticker_data) {
    std::vector<double> rsi_points; // vector holding rsi points
    
    // check for edge cases
    // check for if interval size is 0 or bigger than dataset size:
    if (rsi_interval == 0) {
        fmt::print("Error: RSI interval cannot be zero!");
        return rsi_points;
    }
    if (rsi_interval > ticker_data.size()) {
        fmt::print("Error: RSI interval is larger than the dataset!");
        return rsi_points;
    }

    rsi_points.reserve(ticker_data.size() - rsi_interval);

    double average_gains = 0;
    double average_losses = 0;

    // calculate the first initial RSI value
    for (size_t iterator = 1; iterator < rsi_interval; iterator++) { // use rsi_interval for just the first rsi values
        double difference =  (ticker_data[iterator].close - ticker_data[iterator - 1].close);
        // handle gains or losses or neither
        if (difference > 0) {
            average_gains += difference / rsi_interval;
        } else if (difference < 0) {
            average_losses += (-1 * difference) / rsi_interval; // make sure to get the positive diff_avg
        }
    }
    rsi_points.push_back(100 - ( 100 / (1 + average_gains / average_losses)));

    // check if the RSI interval is the same as the dataset size
    if (rsi_interval == ticker_data.size()) {
        return rsi_points;
    }

    // calculate and add RSI points iteratively
    for (size_t iterator = rsi_interval; iterator < ticker_data.size(); iterator++) {
        double current_new_difference = (
            ticker_data[iterator].close - ticker_data[iterator - 1].close
        );
        // calculate the new current gain/loss/neither
        if (current_new_difference != 0) {
            // while deceptive, the average_gains on the right hand side
            // is actaully average_gains/losses_t-1 since it is the previous value
            // and hence the average_gains/losses on the left hand side is the new value
            if (current_new_difference > 0) {
                average_gains = (average_gains * (rsi_interval - 1) + current_new_difference) / rsi_interval;
            } else{
                // take the absolute value of difference
                average_losses = (average_losses * (rsi_interval - 1) + (-1 * current_new_difference)) / rsi_interval;
            }
        }

        // add current rsi point
        rsi_point_adder(rsi_points, average_gains, average_losses);
    }

    return rsi_points;
}