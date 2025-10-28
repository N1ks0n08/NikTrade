from pathlib import Path
import sys

# Add FlatBuffers path
flatbuffers_path = Path(__file__).parent.parent / "src/core/flatbuffers"
sys.path.append(str(flatbuffers_path))

import flatbuffers
from Binance import BookTicker  # Generated FlatBuffers Python module

def encode_bookticker(payload: dict) -> bytes:
    """
    Convert raw Binance bookTicker JSON into FlatBuffers bytes.
    """
    builder = flatbuffers.Builder(0)

    # Prepare strings
    symbol = builder.CreateString(payload["s"])

    # Start building FlatBuffer
    BookTicker.BookTickerStart(builder)
    BookTicker.BookTickerAddUpdateId(builder, int(payload["u"]))
    BookTicker.BookTickerAddSymbol(builder, symbol)
    BookTicker.BookTickerAddBestBid(builder, float(payload["b"]))
    BookTicker.BookTickerAddBidQty(builder, float(payload["B"]))
    BookTicker.BookTickerAddBestAsk(builder, float(payload["a"]))
    BookTicker.BookTickerAddAskQty(builder, float(payload["A"]))
    fb_obj = BookTicker.BookTickerEnd(builder)
    builder.Finish(fb_obj)

    return bytes(builder.Output())
