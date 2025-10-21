#include "sma_crossover.hpp"
#include <fmt/core.h>
#include <cmath> // for std::floor

// SMA Crossover Backtest concept:
/*
    The SMA Crossover Backtest is a trend following strategy using 2 SMAs of different periods
    Fast SMA (e.g. 10-period SMA)
    Slow SMA (e.g. 50-period SMA)

    Buy Signal: When the fast SMA crosses ABOVE the slow SMA
    (indicates potential upward trend)
    Sell Signal: When the fast SMA crosses BELOW the slow SMA
    (indicates potential downward trend)
*/
std::vector<Trade> sma_crossover_result(int& fastSMAPeriod, int& slowSMAPeriod, double& startingCapital, std::vector<Tick>& ticker_data) {
    std::vector<Trade> sma_crossover_trades;

    // Make sure fast SMA period is smaller than slow SMA period
    if (fastSMAPeriod > slowSMAPeriod) {
        fmt::print("Error: fast SMA period is greater than slow SMA period!\n");
        return sma_crossover_trades;
    }

    // Calculate SMAs
    std::vector<double> fastSMA = smaCalc(fastSMAPeriod, ticker_data);
    std::vector<double> slowSMA = smaCalc(slowSMAPeriod, ticker_data);

    // Variables for tracking shares and last bought price
    int available_shares = 0;
    double last_bought_strike_price = 0.0;

    // Iterate over slow SMA days
    for (size_t slowIndex = 1; slowIndex < slowSMA.size(); ++slowIndex) {
        // Align SMA indices with ticker_data
        size_t day = slowSMAPeriod - 1 + slowIndex;            // actual ticker_data day corresponding to this slowSMA
        size_t fastIndex = day - (fastSMAPeriod - 1);         // corresponding index in fastSMA

        // Safety check
        if (day + 1 >= ticker_data.size() || fastIndex >= fastSMA.size()) {
            // day + 1 >= ticker_data.size() checks for trying to access the open price of the next day after the last available market data
            // fastIndex >= fastSMA.size() checks for if we try to access the index over the fastSMA's maximum size
            break;
        }

        Trade trade{};

        // Check crossover
        // fastIndex - 1 and slowIndex - 1 to check for crossover from previous day to current day's close
        bool bullish = fastSMA[fastIndex - 1] <= slowSMA[slowIndex - 1] &&
                       fastSMA[fastIndex] > slowSMA[slowIndex];
        bool bearish = fastSMA[fastIndex - 1] >= slowSMA[slowIndex - 1] &&
                       fastSMA[fastIndex] < slowSMA[slowIndex];

        if (bullish && available_shares == 0) {
            // Buy on next day's open
            trade.execution_date = ticker_data[day + 1].date;
            trade.order_type = "BUY";
            trade.strike_price = ticker_data[day + 1].open;
            last_bought_strike_price = trade.strike_price;
            trade.shares = static_cast<int>(std::floor(startingCapital / trade.strike_price));
            available_shares += trade.shares;
            startingCapital -= available_shares * last_bought_strike_price;
            trade.unrealizedPnL = 0;
            trade.realizedPnL = sma_crossover_trades.empty() ? 0 : sma_crossover_trades.back().realizedPnL;
            trade.totalEquityValue = startingCapital;

            sma_crossover_trades.push_back(trade);
        }
        else if (bearish && available_shares > 0) {
            // Sell on next day's open
            trade.execution_date = ticker_data[day + 1].date;
            trade.order_type = "SELL";
            trade.strike_price = ticker_data[day + 1].open;
            trade.shares = available_shares;
            trade.unrealizedPnL = 0;
            trade.realizedPnL = (trade.strike_price - last_bought_strike_price) * available_shares;
            startingCapital += trade.realizedPnL;
            available_shares = 0;
            trade.totalEquityValue = startingCapital;

            sma_crossover_trades.push_back(trade);
        }
        else {
            // Hold: update unrealized PnL and total equity
            trade.execution_date = ticker_data[day + 1].date;
            trade.order_type = "HOLD";
            trade.shares = 0;
            trade.strike_price = ticker_data[day + 1].open;

            double unrealizedPnL = available_shares * (ticker_data[day + 1].open - last_bought_strike_price);
            trade.unrealizedPnL = unrealizedPnL;
            trade.realizedPnL = sma_crossover_trades.empty() ? 0 : sma_crossover_trades.back().realizedPnL;
            trade.totalEquityValue = startingCapital + unrealizedPnL + trade.realizedPnL;

            sma_crossover_trades.push_back(trade);
        }
    }

    return sma_crossover_trades;
}
