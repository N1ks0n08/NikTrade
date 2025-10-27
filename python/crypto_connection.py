#base endpoint:
# wss://stream.binance.us:9443
# Raw streams:
# /ws/<streamName>
# Multiple streams:
#  /stream?streams=<streamName1>/<streamName2>/<streamName3>
# data payload format:
# {"stream":"<streamName>","data":<rawPayload>}
"""Ticker Order Book Stream
Payload:

{
  "u":400900217,     // order book updateId
  "s":"BNBUSDT",     // symbol
  "b":"25.35190000", // best bid price
  "B":"31.21000000", // best bid qty
  "a":"25.36520000", // best ask price
  "A":"40.66000000"  // best ask qty
}
Pushes any update to the best bid or asks price or quantity in real-time for a specified symbol.

Stream Name: <symbol>@bookTicker"""

# NOTE: SYMBOLS MUST BE IN LOWERCASE

import asyncio
import websockets
import orjson

# base WebSocket URL:
BASE_URL = "wss://stream.binance.us:9443/ws"

# Symbol to subscribe to
SYMBOL = "bnbusdt"

async def book_ticker_stream(symbol: str):
    url = f"{BASE_URL}/{symbol}@bookTicker"

    # connectoin open attempt
    async with websockets.connect(url) as ws:
        print(f"Connected to {symbol} bookTicker stream!")

        try:
            while True:
                msg = await ws.recv() # receive raw message
                data = orjson.loads(msg) # parse JSON quickly
                # extract the payload
                payload = data
                print(payload)
        except websockets.ConnectionClosed:
            print("Connection closed, exiting...")
    # connectoin closed


if __name__ == "__main__":
    import asyncio
    asyncio.run(book_ticker_stream(SYMBOL))