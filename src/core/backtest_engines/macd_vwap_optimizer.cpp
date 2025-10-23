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
bool bullishMACDSignal (MACDResult& macd_values, size_t iteration) {
    // compare hte previous day's values to current day's values
    // to determine if there is a bullish sign:
    if (macd_values.macd[iteration - 1] <= macd_values.signal[iteration - 1]
            && macd_values.macd[iteration] > macd_values.signal[iteration]) {
                return true;
    } else {
        return false;
    }
}

bool bullishVWAPSignal (std::vector<double>& vwap_values, std::vector<Tick>& ticker_data, size_t iteration) {
    // compare hte previous day's values to current day's values
    // to determine if there is a bullish sign:
    if (ticker_data[iteration - 1].close <= vwap_values[iteration - 1]
            && ticker_data[iteration].close > vwap_values[iteration]) {
        return true;
    } else {
        return false;
    }
}

// Bearish MACD signal: MACD line crosses below the Signal line
// Bearish VWAP signal: price crosses below the VWAP line (sellers accepting prices lower than VWAP)

bool bearishMACDSignal (MACDResult& macd_values, size_t iteration) {
    // compare hte previous day's values to current day's values
    // to determine if there is a bearish sign:
    if (macd_values.macd[iteration - 1] >= macd_values.signal[iteration - 1]
            && macd_values.macd[iteration] < macd_values.signal[iteration]) {
                return true;
    } else {
        return false;
    }
}

bool bearishVWAPSignal (std::vector<double>& vwap_values, std::vector<Tick>& ticker_data, size_t iteration) {
    // compare hte previous day's values to current day's values
    // to determine if there is a bearish sign:
    if (ticker_data[iteration - 1].close >= vwap_values[iteration - 1]
            && ticker_data[iteration].close < vwap_values[iteration]) {
        return true;
    } else {
        return false;
    }
}


std::vector<Trade> MACD_VWAPBacktestResultCalc (int& fastEMAPeriod, int& slowEMAPeriod, int& signalPeriod
    , double starting_capital, std::vector<Tick>& tickerData) {
    std::vector<Trade> macd_vwap_backtest_result;
    // test edge cases:
    if (fastEMAPeriod > slowEMAPeriod) {
        // fast period = smaller period
        fmt::print("Error: fast EMA period of the MACD cannot be bigger than the slow EMA period!");
        return macd_vwap_backtest_result;
    } else if (tickerData.size() == 0) {
        // CHECKS IF tickerdata IS EMPTY
        fmt::print("Error: ticker data is empty!");
        return macd_vwap_backtest_result;
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
    
    // Set the signals
    bool bullish_MACD;
    bool bullish_VWAP;

    bool bearish_MACD;
    bool bearish_VWAP;

    // Keep track of available shares, unrealized/realized PnL, and buying power
    int available_shares = 0;
    double unrealized_PnL = 0;
    double realized_PnL = 0;
    double buying_power = starting_capital;

    // macd.size() is already of size slowEMAperiod  and trimmed to fit with the singal line
    // iteration starts at 1 to check "previous day" for crossover
    // OFFSET THE UPPER CEILING BY 1 TO PREVENT GOING FOR NEXT DAY'S DATA TO TRADE
    // WHICH MAY BE OUT OF BOUNDS
    // starting iteration is set by the documentation within core/tech_indicators/macd.cpp for starting day
    for (int iteration = slowEMAPeriod + signalPeriod - 1; iteration < macd_values.macd.size() - 1; iteration++) {
        Trade trade;
        bullish_MACD = bullishMACDSignal(macd_values, iteration);
        bullish_VWAP = bullishVWAPSignal(vwap_values, tickerData, iteration);
        if (bullish_MACD && bullish_VWAP) {
            // make sure there is enough buying power to make a trade
            // NOTE: check the price of next day's open
            if (buying_power >= tickerData[iteration + 1].open) {
                // set the trade values
                trade.execution_date = tickerData[iteration + 1].date;
                trade.order_type = "BUY";
                trade.strike_price = tickerData[iteration + 1].open;
                trade.shares = buying_power / trade.strike_price;
                trade.held_shares += trade.shares;

                available_shares += buying_power / tickerData[iteration + 1].open;
                // update buying power
                buying_power -= available_shares * tickerData[iteration + 1].open;
            }
        } else {
            bearish_MACD = bearishMACDSignal(macd_values, iteration);
            bearish_VWAP = bearishVWAPSignal(vwap_values, tickerData, iteration);
            if (bearish_MACD && bearish_VWAP) {

            }
            else {

            }
        }

        // update the trade vector
        macd_vwap_backtest_result.push_back(trade);
    }
    
    return macd_vwap_backtest_result;
}
