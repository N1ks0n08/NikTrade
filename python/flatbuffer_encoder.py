from pathlib import Path
import sys

# Add FlatBuffers path
flatbuffers_path = Path(__file__).parent.parent / "src/core/flatbuffers"
sys.path.append(str(flatbuffers_path))

import flatbuffers
from Binance import BookTicker  # Generated FlatBuffers Python module for BookTicker stream
from Binance import Klines, Kline # Generated FlatBuffers Python module for Kline stream

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


def encode_klines(candle_list: list[dict]) -> bytes:
    """
    Encode a list of historical kline dictionaries into a FlatBuffers Klines object.
    
    Args:
        candle_list: list of dicts, each dict with keys:
            open_time, open_price, high_price, low_price, close_price,
            volume, close_time, quote_asset_volume, number_of_trades,
            taker_buy_base, taker_buy_quote, ignore
    Returns:
        Serialized FlatBuffer bytes
    """
    builder = flatbuffers.Builder(1024)

    # First, encode each KlineData table and collect their offsets
    kline_offsets = []
    for candle in candle_list:
        open_price = builder.CreateString(str(candle.get("open_price", "0.0")))
        high_price = builder.CreateString(str(candle.get("high_price", "0.0")))
        low_price = builder.CreateString(str(candle.get("low_price", "0.0")))
        close_price = builder.CreateString(str(candle.get("close_price", "0.0")))
        volume = builder.CreateString(str(candle.get("volume", "0.0")))
        quote_volume = builder.CreateString(str(candle.get("quote_asset_volume", "0.0")))
        taker_base = builder.CreateString(str(candle.get("taker_buy_base", "0.0")))
        taker_quote = builder.CreateString(str(candle.get("taker_buy_quote", "0.0")))
        ignore = builder.CreateString(str(candle.get("ignore", "0")))

        Kline.KlineStart(builder)
        Kline.KlineAddOpenTime(builder, int(candle.get("open_time", 0)))
        Kline.KlineAddOpenPrice(builder, open_price)
        Kline.KlineAddHighPrice(builder, high_price)
        Kline.KlineAddLowPrice(builder, low_price)
        Kline.KlineAddClosePrice(builder, close_price)
        Kline.KlineAddVolume(builder, volume)
        Kline.KlineAddCloseTime(builder, int(candle.get("close_time", 0)))
        Kline.KlineAddQuoteAssetVolume(builder, quote_volume)
        Kline.KlineAddNumberOfTrades(builder, int(candle.get("number_of_trades", 0)))
        Kline.KlineAddTakerBuyBase(builder, taker_base)
        Kline.KlineAddTakerBuyQuote(builder, taker_quote)
        Kline.KlineAddIgnore(builder, ignore)
        kline_offsets.append(Kline.KlineEnd(builder))

    # Create a vector of KlineData
    Klines.KlinesStartKlinesVector(builder, len(kline_offsets))
    for offset in reversed(kline_offsets):
        builder.PrependUOffsetTRelative(offset)
    klines_vector = builder.EndVector(len(kline_offsets))

    # Wrap in Klines table
    Klines.KlinesStart(builder)
    Klines.KlinesAddKlines(builder, klines_vector)
    fb_obj = Klines.KlinesEnd(builder)
    builder.Finish(fb_obj)

    return bytes(builder.Output())