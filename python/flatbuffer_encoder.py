from pathlib import Path
import sys

# Add FlatBuffers path
flatbuffers_path = Path(__file__).parent.parent / "src/core/flatbuffers"
sys.path.append(str(flatbuffers_path))

import flatbuffers
from Binance import BookTicker  # Generated FlatBuffers Python module

def encode_bookticker(payload: dict) -> bytes:
    builder = flatbuffers.Builder(1024)

    # Strings must be FlatBuffers string offsets
    symbol = builder.CreateString(payload.get("s", "UNKNOWN"))
    best_bid = builder.CreateString(payload.get("b", "0.0"))
    bid_qty = builder.CreateString(payload.get("B", "0.0"))
    best_ask = builder.CreateString(payload.get("a", "0.0"))
    ask_qty = builder.CreateString(payload.get("A", "0.0"))
    update_id = int(payload.get("u", 0))

    BookTicker.BookTickerStart(builder)
    BookTicker.BookTickerAddUpdateId(builder, update_id)
    BookTicker.BookTickerAddSymbol(builder, symbol)
    BookTicker.BookTickerAddBestBid(builder, best_bid)
    BookTicker.BookTickerAddBidQty(builder, bid_qty)
    BookTicker.BookTickerAddBestAsk(builder, best_ask)
    BookTicker.BookTickerAddAskQty(builder, ask_qty)
    fb_obj = BookTicker.BookTickerEnd(builder)
    builder.Finish(fb_obj)

    return bytes(builder.Output())
