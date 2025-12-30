#include "BinanceBookTickerDecoder.hpp"

// Convert flatbuffers to BBO struct
BBO decodeToBBO(
    const std::vector<uint8_t>& latestFlatbufferMessage,
    FileLogger& logger
) {
    BBO bbo;
    if (!latestFlatbufferMessage.empty()) {
        logger.logInfo("Message successfully received for display.");
        const Binance::BookTicker* ticker = Binance::GetBookTicker(latestFlatbufferMessage.data());
        if (ticker) {
            auto safe = [](const flatbuffers::String* s) -> double {
                if (!s || s->str().empty()) return 0.0;
                try { return std::stod(s->str()); } catch (...) { return 0.0; }
            };

            bbo.symbol = ticker->symbol() ? ticker->symbol()->str() : "[null]";
            logger.logInfo(fmt::format("Displaying ticker for symbol: {}", bbo.symbol));
            bbo.bid_price = safe(ticker->best_bid());
            bbo.bid_quantity = safe(ticker->bid_qty());
            bbo.ask_price = safe(ticker->best_ask());
            bbo.ask_quantity = safe(ticker->ask_qty());
            bbo.error = "";
        } else {
            bbo.error = "Invalid FlatBuffer data received.";
        }
    } else {
        bbo.error = "No data received yet.";
        /* logger.logInfo(fmt::format(
            "No flatbuffer message received yet for symbol '{}'.",
            win.desiredSymbol
        )); */
    }
    return bbo;
}
