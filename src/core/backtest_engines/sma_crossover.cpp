#include "sma_crossover.hpp"
#include <fmt/core.h>

// SMA Crossover Backtest concept:
/*
    The SMA Crossover Backtest is a trend following strategy using 2 SMAs of different periods
    Fast SMA (e.g. 10-period SMA)
    Slow EMA (e.g. 50-peroid SMA)

    Buy Signal: When the fast SMA crosses ABOVE the slow SMA
    (indicates potential upward trend)
    Sell Signal: When the fast SMA crosses BELOW the slow SMA
    (indicates potential downward trend)
*/
std::vector<Trade> sma_crossover_result(int& fastSMAPeriod, int& slowSMAPeriod, double& startingCapital, std::vector<Tick>& ticker_data) {
    std::vector<Trade> sma_crossover_trades;
    // Make sure fast SMA period is smaller than the long SMA period
    if (fastSMAPeriod > slowSMAPeriod) {
        fmt::print("Error: fast SMA period is greater than slow SMA period!");
        return sma_crossover_trades;
    }

    // SMA calculations will handle the ticker data size edge case
    std::vector<double> fastSMA = smaCalc(fastSMAPeriod, ticker_data);
    std::vector<double> slowSMA = smaCalc(slowSMAPeriod, ticker_data);

    // Offset the index starting points between the 2 SMAs
    // The fast SMA has a shorter period therefore a higher vector size
    // The slow SMA has a longer period therefore a lower vector size

    // ALSO, the fast SMA has a required period of data
    // before starting therefore a lower vector size,
    // therefore it has a smaller vector size than the actual ticker data size

    // Assuming iteration = 0:
    // ALL ARE SHIFTED TO THE RIGHT ONE INDEX to match the slow SMA's index start
    // The offset for ticker data index start: fast SMA period + slow EMA period + iteration
    // The offset for fast SMA index start: slow SMA period + iteration
    // The offset for slow SMA index start: 1 + iteration

    // Start at the slow SMA's index + 1 (offset for checking actual cross of SMAs)
    // end at the slow SMA's index - 1 (offset for buhying the next open)
    // NOTE: ONLY buy/sell the day AFTER the SMA crossovers (Next day's open price)
    int available_shares = 0;
    double last_bought_strike_price = 0;
    // upper bound is slowSMA.size() - 1
    // because of taking one less day of SMA values to
    // account for next day's open price which will be bought
    for (size_t iterator = 0; iterator < slowSMA.size() - 1; iterator++) {
        size_t current_day_slowSMA = iterator + 1;
        size_t previous_day_slowSMA = current_day_slowSMA - 1;
        size_t current_day_fastSMA = slowSMAPeriod + iterator;
        size_t prevoius_day_fastSMA = current_day_fastSMA - 1;
        size_t current_day_tickerData = fastSMAPeriod + slowSMAPeriod + iterator;
        size_t prevoius_day_tickerData = current_day_tickerData - 1;
        Trade trade{};
        bool bullish = {
            // Check if fast SMA crossed the slow SMA in bullish fashion
            fastSMA[prevoius_day_fastSMA] <= slowSMA[previous_day_slowSMA] // previous day SMA
            &&
            fastSMA[current_day_fastSMA] > slowSMA[current_day_slowSMA] // current day SMA
        };
        bool bearish = {
            // Check if fast SMA crossed the slow SMA in bearish fashion
            fastSMA[prevoius_day_fastSMA] >= slowSMA[previous_day_slowSMA] // previous day SMA
            &&
            fastSMA[current_day_fastSMA] < slowSMA[current_day_slowSMA] // current day SMA
        };
        // Buy if fastSMA_current > slowSMA_current
        // AND fastSMA_previous < slowSMA_previous
        // AND shares are equal 0 (can't buy more if already maximized shares previously)
        if (bullish && available_shares == 0) {
            // Set trade date to next day
            trade.execution_date = ticker_data[current_day_tickerData + 1].date;
            trade.order_type = "BUY";
            // + 1 to index for getting next day's open
            trade.strike_price = ticker_data[current_day_tickerData + 1].open;
            // set the last strike price for which shares were bought
            last_bought_strike_price = trade.strike_price;
            trade.shares = static_cast<int>(std::floor(startingCapital / trade.strike_price));
            // update the available shares
            available_shares += trade.shares;
            // update starting capital (for future buying power)
            startingCapital -= available_shares * last_bought_strike_price;
            // set unrealized PnL; should be zero since this is hte beginning of the trade:
            trade.unrealizedPnL = 0;
            // calculate realized PnL: zero if initial trade, the previous realized PnL if not
            if (iterator == 0) {
                trade.realizedPnL = 0;
            } else {
                trade.realizedPnL = sma_crossover_trades[iterator - 1].realizedPnL;
            }

            sma_crossover_trades.push_back(trade);
        }
        // Sell if fastSMA < slowSMA (available shares only as of now; no short positions)
        // Selling leads to realized PnL
        else if (bearish && available_shares > 0) {
            // Set trade execution to next day at open
            trade.execution_date = ticker_data[current_day_tickerData + 1].date;
            trade.order_type = "SELL";
            // + 1 to index for getting next day's open
            trade.strike_price = ticker_data[current_day_tickerData + 1].open;
            trade.shares = available_shares;
            // reset urnealized PnL
            trade.unrealizedPnL = 0;
            // finalize realized PnL
            trade.realizedPnL = (trade.strike_price - last_bought_strike_price) * available_shares;
            // reset the available shares
            available_shares -= trade.shares;
            // Update starting capital
            startingCapital += trade.realizedPnL;
            // Calculate current total equity value:
            trade.totalEquityValue = startingCapital;

            sma_crossover_trades.push_back(trade);
        } else { // Current protfolio status as of date
            // Set hold date to next day:
            trade.execution_date = ticker_data[current_day_tickerData + 1].date;
            trade.order_type = "HOLD";
            // No trade therefore no shares traded
            trade.shares = 0;
            // Use strike price from next day
            trade.strike_price = ticker_data[current_day_tickerData + 1].open;
            // Calculate unrealized PnL
            // If the first trade and no trades were made:
            // unrealized PnL = 0
            if (sma_crossover_trades.size() == 0) {
                trade.realizedPnL = 0;
            } else {
                // Realized PnL is the same as before
                trade.realizedPnL = sma_crossover_trades[iterator - 1].realizedPnL;
            }
            // Calculate unrealized PnL
            trade.unrealizedPnL = available_shares * (last_bought_strike_price - ticker_data[iterator].open);
            // Calculate current total equity value:
            trade.totalEquityValue = startingCapital + trade.unrealizedPnL + trade.realizedPnL;

            sma_crossover_trades.push_back(trade);
        }
    }

    return sma_crossover_trades;
}