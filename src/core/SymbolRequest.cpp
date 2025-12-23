#include "SymbolRequest.hpp"

void handleSymbolChangeRequest(
    const std::string& symbol,
    ZMQControlClient& controlClient,
    FileLogger& logger
) {
    std::string reply;
    bool ok = controlClient.sendControlRequest(
        fmt::format("switch_symbol {}", symbol),
        reply,
        logger,
        500
    );

    if (ok) logger.logInfo(fmt::format("[INFO] Switched symbol: {}", reply));
    else logger.logInfo("[WARN] Failed to switch symbol.");
}
