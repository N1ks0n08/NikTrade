#include "core/backtest_engines/macd_vwap_optimizer.hpp"
#include <fmt/core.h>

// As of now, focus on:
// - Total PnL
// - Win Rate
// - Average Trade PnL
// - and Max Drawdown

// For now, execute buy/sell orders ONLY when both signals are bullish

// Bulllish MACD signal: MACD line crosses above the Signal line
// Bullish VWAP signal: price crosses above the VWAP line (buyers acceptingp rices )
bool bullishMACDSignal (MACDResult& macd_values) {
    // compare hte previous day's values to current day's values
    // to determine if there is a bullish sign:
    return true;
}

bool bullishVWAPSignal (std::vector<double>& vwap_values) {
    // compare hte previous day's values to current day's values
    // to determine if there is a bullish sign:
    return true;
}

// Bearish MACD signal: MACD line crosses below the Signal line
// Bearish VWAP signal: price crosses below the VWAP line (sellers accepting prices lower than VWAP)

bool bearishMACDSignal (MACDResult& macd_values) {
    // compare hte previous day's values to current day's values
    // to determine if there is a bearish sign:
    return true;
}

bool bearishVWAPSignal (std::vector<double>& vwap_values) {
    // compare hte previous day's values to current day's values
    // to determine if there is a bearish sign:
    return true;
}


optimizer_result getOptimizerResult (int& fastEMAPeriod, int& slowEMAPeriod, int& signalPeriod, std::vector<Tick>& tickerData) {
    optimizer_result optimizer_result;

    // test edge cases:
    if (fastEMAPeriod > slowEMAPeriod) {
        // fast period = smaller period
        fmt::print("Error: fast EMA period of the MACD cannot be bigger than the slow EMA period!");
        return optimizer_result;
    } else if (tickerData.size() == 0) {
        // CHECKS IF tickerdata IS EMPTY
        fmt::print("Error: ticker data is empty!");
        return optimizer_result;
    }
    // get the MACD and VWAP vectors:
    MACDResult macd_values = macdCalc(fastEMAPeriod, slowEMAPeriod, signalPeriod, tickerData);
    std::vector<double> vwap_values = vwapCalc(tickerData);

    // BACKTEST LOGIC 1:
    // This engine starts on the same day ALL vectors are first available,
    // which means the start of the slowEMA period as it starts the latest.

    // BACKTEST LOGIC 2:
    // In order to execute trades,
    // (ssuming current_day is the iteration through the slowEMA period,
    // tickerData[current_day + 1] MUST EXIST
    // This is due to the fact that EMAs are calculated END OF THE CURRENT DAY
    // and we cannot BUY/SELL the CLOSE price of THE END OFCURRENT DAY,
    // ONLY the OPEN price of THE NEXT DAY
    // therefore, the signal occurs on current_day
    // and the trade MUST EXECUTE ON current_day + 1

    // BACKTEST LOGIC 3:
    // In addition to the previous logic, persistent bullish signals must also be held.
    // For instance, MACD may be bullish on day 50, but VWAP may not be bullish until day 53
    // (vice versa; VWAP may be bullish on day 50, but MACD may not be bullish until day 53)
    // The same is true for bearish signals.
    // Hence, I must account for this delay between bullish signals
    // THE SAME APPLIES FOR BEARISH CIRCUMSTANCES AS WELL

    // macd.size() is already of size slowEMAperiod 
    for (int current_day = 1; current_day < macd_values.macd.size(); current_day++) {
        fmt::print("Lol");
    }

    return optimizer_result;
}

double totalPnL() {
    return 0.0;
}

double winRate() {
    return 0.0;
}

double averageTradePnL() {
    return 0.0;
}

double maxDrawDown() {
    return 0.0;
}