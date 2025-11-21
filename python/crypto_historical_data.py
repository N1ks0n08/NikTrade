import asyncio
import aiohttp
import orjson
from datetime import datetime, timedelta
from pathlib import Path

BASE_REST_URL = "https://api.binance.us/api/v3/klines"

async def fetch_historical_klines(session, symbol: str, interval: str, start_time: int = None, end_time: int = None, limit: int = 500):
    """
    Async generator to fetch historical klines from Binance.US REST API.

    Yields one candle at a time as a dictionary.
    """
    params = {
        "symbol": symbol.upper(),
        "interval": interval,
        "limit": limit
    }
    if start_time:
        params["startTime"] = start_time
    if end_time:
        params["endTime"] = end_time

    async with session.get(BASE_REST_URL, params=params) as resp:
        resp.raise_for_status()
        data = await resp.json(loads=orjson.loads)

        for c in data:
            candle_dict = {
                "open_time": c[0],
                "open_price": c[1],
                "high_price": c[2],
                "low_price": c[3],
                "close_price": c[4],
                "volume": c[5],
                "close_time": c[6],
                "quote_asset_volume": c[7],
                "number_of_trades": c[8],
                "taker_buy_base": c[9],
                "taker_buy_quote": c[10],
                "ignore": c[11],
            }
            yield candle_dict  # yield each candle one by one

async def main():
    symbol = "BNBBTC"
    interval = "1m"
    limit = 500

    async with aiohttp.ClientSession() as session:
        async for candle in fetch_historical_klines(session, symbol, interval, limit=limit):
            print(candle)

if __name__ == "__main__":
    asyncio.run(main())
