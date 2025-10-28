import asyncio
import json
from pathlib import Path
import aiohttp

# --- Local imports ---
from crypto_connection import book_ticker_stream
from flatbuffer_encoder import encode_bookticker
from zmq_publisher import ZMQPublisher

# File where Binance trading symbols are cached
SYMBOL_FILE = Path.cwd() / "binance_symbols.json"


async def fetch_binance_symbols():
    """
    Fetch all active Binance trading symbols (cached to disk).
    """
    if SYMBOL_FILE.exists():
        print(f"[INFO] Loading cached symbols from {SYMBOL_FILE}")
        with open(SYMBOL_FILE, "r") as f:
            return json.load(f)

    print("[INFO] Fetching active symbols from Binance REST API...")
    url = "https://api.binance.com/api/v3/exchangeInfo"

    async with aiohttp.ClientSession() as session:
        async with session.get(url) as resp:
            resp.raise_for_status()
            data = await resp.json()

    symbols = [
        s["symbol"].lower()
        for s in data["symbols"]
        if s["status"] == "TRADING"
    ]

    with open(SYMBOL_FILE, "w") as f:
        json.dump(symbols, f, indent=2)

    print(f"[INFO] Saved {len(symbols)} trading pairs to {SYMBOL_FILE}")
    return symbols


async def stream_and_publish(symbols):
    """
    Stream multiple Binance bookTicker feeds concurrently and publish via ZMQ.
    """
    publisher = ZMQPublisher("tcp://127.0.0.1:5555")

    async def consume(sym):
        async for payload in book_ticker_stream(sym):
            try:
                fb_bytes = encode_bookticker(payload)
                await publisher.publish(f"bookticker.{sym}", fb_bytes)
            except Exception as e:
                print(f"[ERROR] {sym}: {e}")

    # Limit to first 5 for testing
    tasks = [asyncio.create_task(consume(sym)) for sym in symbols[:5]]

    try:
        await asyncio.gather(*tasks)
    except asyncio.CancelledError:
        print("[INFO] Streaming cancelled. Cleaning up...")
    finally:
        await publisher.close()


async def main():
    symbols = await fetch_binance_symbols()
    await stream_and_publish(symbols)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n[INFO] Graceful shutdown requested by user.")
