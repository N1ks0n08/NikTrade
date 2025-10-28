import asyncio
import websockets
import orjson
from pathlib import Path
import sys

BASE_URL = "wss://stream.binance.us:9443/ws"

async def book_ticker_stream(symbol: str):
    url = f"{BASE_URL}/{symbol}@bookTicker"
    async with websockets.connect(url) as ws:
        print(f"Connected to {symbol} bookTicker stream!")
        try:
            while True:
                msg = await ws.recv()
                payload = orjson.loads(msg)
                yield payload
        except websockets.ConnectionClosed:
            print("Connection closed.")
        except asyncio.CancelledError:
            print("\nStreaming cancelled by user.")
        except Exception as e:
            print("Connection failed:", e)


if __name__ == "__main__":
    async def main():
        async for payload in book_ticker_stream("bnbusdt"):
            print(payload)

    asyncio.run(main())
