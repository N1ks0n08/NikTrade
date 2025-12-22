#include <fmt/core.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include "utils/file_logger.hpp"
#include "../../core/flatbuffers/Binance/binance_bookticker_generated.h"
#include "../../core/BBO.hpp"

BBO decodeToBBO(
    const std::vector<uint8_t>& latestCryptoMessage,
    FileLogger& logger
);
