import asyncio
import json
from pathlib import Path
import aiohttp
from crypto_connection import book_ticker_stream
from flatbuffer_encoder import encode_bookticker
from zmq_publisher import ZMQPublisher
import logging

SYMBOL_FILE = Path.cwd() / "binance_symbols.json"

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

async def fetch_binance_symbols():
    """
    Fetch all active Binance trading symbols (cached to disk).
    """
    if SYMBOL_FILE.exists():
        logger.info(f"[INFO] Loading cached symbols from {SYMBOL_FILE}")
        with open(SYMBOL_FILE, "r") as f:
            return json.load(f)

    logger.info("[INFO] Fetching active symbols from Binance REST API...")
    url = "https://api.binance.us/api/v3/exchangeInfo"

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

    logger.info(f"[INFO] Saved {len(symbols)} trading pairs to {SYMBOL_FILE}")
    return symbols

async def stream_and_publish(symbols):
    """
    Stream Binance bookTicker data and publish over ZMQ PUB socket.
    Includes throttling + clean shutdown handling.
    """
    publisher = ZMQPublisher("tcp://127.0.0.1:5555")

    async def consume(sym):
        """Consume a single symbol's websocket and publish FlatBuffers data."""
        async for payload in book_ticker_stream(sym):
            try:
                fb_bytes = encode_bookticker(payload)
                await publisher.publish(f"bookticker.{sym}", fb_bytes)

                # Optional: throttle sends to reduce CPU usage
                await asyncio.sleep(0.05)

            except Exception as e:
                logger.error(f"[ERROR] {sym}: {e}")
                await asyncio.sleep(1)  # backoff before retrying

    # Limit symbols for testing
    tasks = [asyncio.create_task(consume(sym)) for sym in symbols[:5]]

    try:
        done, pending = await asyncio.wait(tasks, return_when=asyncio.FIRST_EXCEPTION)
        for t in pending:
            t.cancel()
    except asyncio.CancelledError:
        logger.info("[INFO] Streaming cancelled. Cleaning up...")
    finally:
        await publisher.close()

async def main():
    symbols = await fetch_binance_symbols()
    await stream_and_publish(symbols)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("\n[INFO] Graceful shutdown requested by user.")